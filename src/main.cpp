/**
 * @file main.cpp
 * @brief Affichage des données GPS pour une télécommande de voilier RC
 * 
 * Ce programme implémente une interface d'affichage sur un M5Stack Core2 pour montrer les données GPS 
 * reçues d'un voilier via le protocole sans fil ESP-NOW. Il affiche :
 * - Vitesse actuelle en nœuds avec barre colorée (vert/orange/rouge)
 * - Cap en degrés
 * - Nombre de satellites GPS visibles
 * - Une rose des vents avec une aiguille mobile indiquant le cap actuel
 *
 * Structure des données reçues :
 * - latitude : latitude GPS
 * - longitude : longitude GPS
 * - speed : Vitesse en m/s (convertie en nœuds pour l'affichage)
 * - heading : Cap en degrés (0=Nord, 90=Est, etc)
 * - satellites : Nombre de satellites GPS visibles
 *
 * Caractéristiques de l'affichage :
 * - Barre de vitesse sur le côté droit (max 6 nœuds)
 * - Indicateur de vitesse en couleur (vert <2nds, orange 2-4nds, rouge >4nds)
 * - Rose des vents avec points cardinaux et aiguille mobile
 * - Vitesse et cap actuels en format texte
 * - Nombre de satellites
 *
 * Communication :
 * - Utilise le protocole ESP-NOW pour recevoir les données
 * - L'appareil fonctionne en mode WiFi Station
 * 
 * @note L'aiguille de la boussole se déplace instantanément vers la nouvelle position (animation fluide désactivée)
 */
#include <M5Unified.h>
#include <WiFi.h>
#include <esp_now.h>
#include <math.h>

// Structure pour recevoir les données du bateau
typedef struct struct_message_Boat {
    int8_t messageType;  // 1 = Boat, 2 = Anemometer
    float latitude;
    float longitude;
    float speed;     // en m/s
    float heading;   // en degrés (0=N, 90=E, 180=S, 270=W)
    uint8_t satellites; // nombre de satellites visibles
    bool isGPSRecording; // Indique si l'enregistrement GPS est activé
} struct_message_Boat;

struct_message_Boat incomingBoatData;

/**
 * @brief Structure containing anemometer data for broadcast
 */
typedef struct struct_message_Anemometer {
    int8_t messageType;  // 1 = Boat, 2 = Anemometer
    uint32_t anemometerId;   // Unique ID of the anemometer
    uint8_t macAddress[6];   // MAC address of the device
    float windSpeed;         // Wind speed value
} struct_message_Anemometer;

struct_message_Anemometer incomingAnemometerData;


// Structure pour recevoir les données
typedef struct struct_message_display_to_boat {
    bool recordGPS; // Indique si l'enregistrement GPS est activé;
} struct_message_display_to_boat;

struct_message_display_to_boat outgoingData;

// Variables d'affichage
//Boat data
float currentHeading = 0.0;
float targetHeading = 0.0;

//Anemometer data
float windSpeed = 0.0;
uint32_t anemometerID = 0;
bool newData = false;


// Constantes pour Core2
const int screenWidth = 320;
const int screenHeight = 240;
const int centerX = screenWidth / 2;
const int centerY = (screenHeight / 2) + 30;  // on descend un peu pour laisser du texte
const int arrowLength = 55; // plus grand que sur l'AtomS3
static float previousAngle = -1;  // Mémoriser le dernier angle pour effacer

// Structure for sending commands to the boat
typedef struct command_message {
  uint8_t command;  // 1 = start logging, 2 = stop logging
} command_message;

// MAC address of the boat (needs to be set to match your boat's ESP32)
uint8_t boatAddress[] = {0x24, 0xA1, 0x60, 0x45, 0xE7, 0xF8};  //Boat2

// MAC address of the anemometer (needs to be set to match your anemometer's ESP32)
uint8_t anemometerAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  //Anemometer1

/**
 * @brief Fonction de rappel déclenchée après l'envoi d'un message ESP-NOW.
 * 
 * Cette fonction est appelée pour indiquer le statut d'un message envoyé via ESP-NOW.
 * Elle affiche le résultat de la transmission sur le moniteur série, indiquant si
 * le message a été envoyé avec succès ou s'il a échoué.
 * 
 * @param mac_addr Pointeur vers l'adresse MAC de l'appareil destinataire.
 * @param status Statut de la transmission du message. Il peut être l'un des suivants :
 *               - ESP_NOW_SEND_SUCCESS : Le message a été envoyé avec succès.
 *               - ESP_NOW_SEND_FAIL : L'envoi du message a échoué.
 */
void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Envoi: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Succès" : "Échec");
}

/**
 * @brief Fonction de rappel pour la réception des messages ESP-NOW
 * @param mac Adresse MAC de l'émetteur
 * @param incomingDataPtr Pointeur vers les données reçues
 * @param len Longueur des données reçues
 * 
 * Cette fonction est appelée lorsque des données ESP-NOW sont reçues. Elle copie les données 
 * dans la structure incomingData et met à jour le cap cible pour la boussole.
 * Affiche également la latitude et la longitude sur le port série pour le débogage.
 */
void onReceive(const uint8_t *mac, const uint8_t *incomingDataPtr, int len) {
  uint8_t messageType = incomingDataPtr[0];  // Premier byte = type

  switch (messageType)
  {
  case 1: // GPS Boat

    memcpy(&incomingBoatData, incomingDataPtr, sizeof(incomingBoatData));

    Serial.print("Latitude: ");
    Serial.println(incomingBoatData.latitude, 6);
    Serial.print("Longitude: ");
    Serial.println(incomingBoatData.longitude, 6);
    targetHeading = incomingBoatData.heading;
    newData = true;

    break;

  case 2: // Anemometer

    memcpy(&incomingAnemometerData, incomingDataPtr, sizeof(incomingAnemometerData));

    Serial.print("Anemometer ID: ");
    Serial.println(incomingAnemometerData.anemometerId);
    Serial.print("Wind Speed: ");
    Serial.println(incomingAnemometerData.windSpeed, 2);
    windSpeed = incomingAnemometerData.windSpeed;
    anemometerID = incomingAnemometerData.anemometerId;
    newData = true;

    break;
  }
}


/**
 * @brief Affiche l'écran de démarrage initial
 * 
 * Affiche un écran de démarrage avec le texte "GPS" et "FRA222" centré sur l'écran
 * pendant 2 secondes avant d'effacer l'écran en noir.
 */
void showSplashScreen() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextDatum(MC_DATUM); // Centre
  M5.Lcd.setTextSize(4);
  M5.Lcd.drawString("GPS", centerX, centerY - 60);
  M5.Lcd.drawString("FRA222", centerX, centerY + 0);
  delay(2000);
  M5.Lcd.fillScreen(BLACK);
}

/**
 * @brief Dessine une barre de vitesse verticale sur le côté droit de l'écran
 * @param speedKnots Valeur de la vitesse en noeuds à afficher
 * 
 * Crée une barre verticale montrant la vitesse actuelle avec:
 * - Valeur maximum de 6 noeuds
 * - Code couleur: vert (<2nds), orange (2-4nds), rouge (>4nds)  
 * - Bordure blanche autour de la barre
 */
void drawSpeedBar(float speedKnots) {
  // Nettoyer la zone de la barre
  M5.Lcd.fillRect(screenWidth - 20, 100, 10, 120, BLACK);

  float maxSpeed = 6.0;
  int barHeight = (int)((speedKnots / maxSpeed) * 120);
  if (barHeight > 120) barHeight = 120;

  uint32_t barColor = GREEN;
  if (speedKnots > 2.0 && speedKnots <= 4.0) {
    barColor = ORANGE;
  } else if (speedKnots > 4.0) {
    barColor = RED;
  }

  int barY = 220 - barHeight;
  M5.Lcd.fillRect(screenWidth - 20, barY, 10, barHeight, barColor);
  M5.Lcd.drawRect(screenWidth - 20, 100, 10, 120, WHITE);
}

/**
 * @brief Met à jour les informations principales de l'affichage
 * 
 * Affiche :
 * - Vitesse en nœuds avec code couleur
 * - Cap en degrés
 * - Nombre de satellites visibles
 * Appelle également drawSpeedBar pour mettre à jour l'indicateur de vitesse
 */
void drawDisplay() {
  // Efface le haut pour infos
  M5.Lcd.fillRect(0, 0, screenWidth, 180, BLACK);

  // Speed / Course
  float speedKnots = incomingBoatData.speed * 1.94384; //conversion m/s en noeuds 
  float windSpeedKnots = incomingAnemometerData.windSpeed * 1.94384; //conversion m/s en noeuds

  uint32_t speedColor = GREEN;
  if (speedKnots > 2.0 && speedKnots <= 4.0) {
    speedColor = ORANGE;
  } else if (speedKnots > 4.0) {
    speedColor = RED;
  }
  
  M5.Lcd.setTextDatum(TL_DATUM);
  M5.Lcd.setTextColor(WHITE);
  
  // Speed in large digital format
  M5.Lcd.setTextColor(RED);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(10, 50);
  M5.Lcd.println("F222");
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(140, 50);
  M5.Lcd.printf("%.1f", speedKnots);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(240, 50);
  M5.Lcd.println("KTS");

  // Course in large digital format
  M5.Lcd.setTextSize(4);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(140, 90);
  M5.Lcd.printf("%.0f", incomingBoatData.heading);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(240, 90);
  M5.Lcd.println("DEG");

  // Satellites
  M5.Lcd.setCursor(240, 10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("%d SAT", incomingBoatData.satellites);

  // WindSpeed in large digital format
  M5.Lcd.setTextColor(RED);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(10, 150);
  M5.Lcd.println("BUOY1");
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(140, 150);
  M5.Lcd.printf("%.1f", windSpeedKnots);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(240, 150);
  M5.Lcd.println("KTS");

  // Barre de vitesse
  //drawSpeedBar(speedKnots);

  //Indique si l'enregistrement GPS est actif sur le bateau
  if (incomingBoatData.isGPSRecording) 
  {  
        M5.Lcd.fillRect(0, 200, 80, 40, GREEN);
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("RECORD", 40, 220);
  } 
  else 
  { 
        M5.Lcd.fillRect(0, 200, 80, 40, BLACK);      
  }
}

/**
 * @brief Dessine la rose des vents et l'indicateur de cap
 * @param heading Cap actuel en degrés
 * 
 * Affiche :
 * - Cercle avec points cardinaux (N,E,S,O)
 * - Aiguille rouge pointant dans la direction actuelle
 * - Point blanc au centre
 * L'ancienne position de l'aiguille est effacée avant de dessiner la nouvelle
 */
void drawCompass(float heading) {
  
  int x0 = centerX;
  int y0 = centerY;

  if (previousAngle >= 0) {
    float oldRad = (previousAngle - 90) * DEG_TO_RAD;
    int oldX = x0 + cos(oldRad) * arrowLength;
    int oldY = y0 + sin(oldRad) * arrowLength;
    M5.Lcd.drawLine(x0, y0, oldX, oldY, BLACK);
  }

  // Cercle et cardinal points (ne pas recréer tout le temps)
  static bool circleDrawn = false;
  if (!circleDrawn) {
    M5.Lcd.drawCircle(x0, y0, arrowLength, TFT_BLUE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("N", x0, y0 - arrowLength - 15);
    M5.Lcd.drawString("E", x0 + arrowLength + 15, y0);
    M5.Lcd.drawString("S", x0, y0 + arrowLength + 15);
    M5.Lcd.drawString("W", x0 - arrowLength - 15, y0);
    circleDrawn = true;
  }

  // ➔ Ici l'angle corrigé
  float angleRad = (heading - 90) * DEG_TO_RAD;
  int x1 = x0 + cos(angleRad) * arrowLength;
  int y1 = y0 + sin(angleRad) * arrowLength;

  M5.Lcd.drawLine(x0, y0, x1, y1, RED);
  M5.Lcd.fillCircle(x0, y0, 5, WHITE);

  previousAngle = heading;
}





/**
 * @brief Vérifie et gère les interactions tactiles sur l'écran M5Stack
 * 
 * Cette fonction surveille les entrées tactiles de l'utilisateur dans la zone inférieure de l'écran.
 * Elle divise la zone en deux parties :
 * - La moitié gauche déclenche le démarrage de l'enregistrement (commande 1)
 * - La moitié droite arrête l'enregistrement (commande 2)
 * 
 * Pour chaque action, la fonction :
 * 1. Envoie la commande appropriée
 * 2. Met à jour l'affichage avec un rectangle coloré (vert pour démarrer, rouge pour arrêter)
 * 3. Affiche le texte correspondant à l'action
 * 
 * @note La zone tactile active se trouve en bas de l'écran (y > 200)
 */
void checkButtons() {
  if (M5.Touch.getCount()) {
    auto t = M5.Touch.getDetail();
    if (t.y > 200) {  // Bottom area of screen
      if (t.x < 160) 
      {  // Left half - Start logging
        // Envoi des données GPS à l'afficheur distant
        outgoingData.recordGPS = true; // Indique que l'enregistrement GPS est activé
      } 
      else 
      {  // Right half - Stop logging
        outgoingData.recordGPS = false; // Indique que l'enregistrement GPS est désactivé
      }
   
      // Envoi des données GPS à l'afficheur distant
      esp_err_t result = esp_now_send(boatAddress, (uint8_t *)&outgoingData, sizeof(outgoingData));
      if (result == ESP_OK) {
        Serial.println("Données isGPSRecording envoyées au bateau");
      } else {
        Serial.println("Erreur d'envoi de donnée isGPSRecording envoyées au bateau");
      }
   }
  }
}


/**
 * @brief Fonction d'initialisation exécutée une seule fois au démarrage
 *
 * Initialise :
 * - L'écran M5Stack et l'alimentation
 * - La communication série 
 * - Le WiFi en mode station
 * - Le protocole ESP-NOW
 * Affiche l'adresse MAC sur le port série et montre l'écran de démarrage
 */
void setup() {

  auto cfg = M5.config();
  cfg.clear_display = true; // Effacer l'affichage au démarrage
  cfg.output_power = true;  // Activer l'alimentation des périphériques

  M5.begin(cfg);
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Récupère l'adresse MAC
  String macAddress = WiFi.macAddress();

  // Affiche sur le port série
  Serial.println("Adresse MAC :");
  Serial.println(macAddress);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erreur d'initialisation ESPNow");
    ESP.restart();
  }

  //Boat Management
  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfoBoat = {};
  memcpy(peerInfoBoat.peer_addr, boatAddress, 6);
  peerInfoBoat.channel = 0;  
  peerInfoBoat.encrypt = false;
  
  if (esp_now_add_peer(&peerInfoBoat) != ESP_OK) 
  {
    Serial.println("Erreur ajout peer Boat");
    ESP.restart();
  }

  //Anemometer Management
  esp_now_peer_info_t peerInfoAnemometer = {};
  memcpy(peerInfoAnemometer.peer_addr, anemometerAddress, 6);
  peerInfoAnemometer.channel = 1;  
  peerInfoAnemometer.encrypt = false;

  if (esp_now_add_peer(&peerInfoAnemometer) != ESP_OK) 
  {
    Serial.println("Erreur ajout peer Anemometer");
    ESP.restart();
  }
  
  
  esp_now_register_recv_cb(onReceive);

  showSplashScreen();

}

/**
 * @brief Boucle principale du programme
 * 
 * En continu :
 * - Met à jour l'état du M5Stack
 * - Rafraîchit l'affichage quand de nouvelles données sont reçues
 * - Met à jour le cap de la boussole
 * - Gère le débordement du cap (0-360 degrés) 
 * S'exécute avec un délai de 50ms entre les itérations
 */
void loop() {

  M5.update(); // Met à jour l'état des boutons et autres périphériques M5
 
  if (newData) {
    drawDisplay();
    newData = false;
  }

  checkButtons(); // Vérifie les interactions tactiles

  
  currentHeading = incomingBoatData.heading;
  if (currentHeading < 0) {
    currentHeading += 360;
  } else if (currentHeading >= 360) {
    currentHeading -= 360;
  }
 
  //drawCompass(currentHeading);

  delay(50);

}

