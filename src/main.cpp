// Copyright (C) 2025 Philippe Hubert
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
#include <vector>
#include <stddef.h>  // Pour offsetof
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "Logger.h"
#include "Display.h"
#include "DisplayTypes.h"
#include "Storage.h"
#include "FileServerManager.h"


// Instances globales
struct_message_Boat incomingBoatData;
struct_message_Anemometer incomingAnemometerData;

// Structure de compatibilité pour bateau sans gpsTimestamp (pour debug)
typedef struct struct_message_Boat_Legacy {
    int8_t messageType;  // 1 = Boat, 2 = Anemometer
    float latitude;
    float longitude;
    float speed;     // en m/s
    float heading;   // en degrés (0=N, 90=E, 180=S, 270=W)
    uint8_t satellites; // nombre de satellites visibles
    bool isGPSRecording; // Indique si l'enregistrement GPS est activé
} struct_message_Boat_Legacy;

struct_message_display_to_boat outgoingData;

// Instances des gestionnaires
Logger logger;
Display display;
Storage storage;
FileServerManager fileServer;

// Queue pour les données à stocker
QueueHandle_t storageQueue;
std::vector<StorageData> pendingStorageData;
SemaphoreHandle_t storageDataMutex;

bool newData = false;
bool sdInitialized = false; // État de la carte SD
bool isRecording = false;

// Variables pour le debouncing des boutons tactiles (un timer par bouton)
unsigned long lastTouchTimeButton1 = 0; // Bouton GPS (gauche)
unsigned long lastTouchTimeButton2 = 0; // Bouton central
unsigned long lastTouchTimeButton3 = 0; // Bouton WiFi (droite)
const unsigned long TOUCH_DEBOUNCE_MS = 500; // 500ms entre les appuis

uint8_t boatAddress[] = {0x24, 0xA1, 0x60, 0x45, 0xE7, 0xF8};  //Boat2
uint8_t anemometerAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  //Anemometer1

/**
 * @brief Affiche des informations de diagnostic sur la structure des données
 */
void printStructureInfo() {
    logger.log("=== DIAGNOSTIC STRUCTURE ===");
    logger.log("--- BATEAU ---");
    logger.log(String("Taille struct_message_Boat: ") + String(sizeof(struct_message_Boat)) + " bytes");
    logger.log(String("Taille struct_message_Boat_Legacy: ") + String(sizeof(struct_message_Boat_Legacy)) + " bytes");
    logger.log("--- ANÉMOMÈTRE ---");
    logger.log(String("Taille struct_message_Anemometer: ") + String(sizeof(struct_message_Anemometer)) + " bytes");
    logger.log(String("Taille struct_message_Anemometer_Legacy: ") + String(sizeof(struct_message_Anemometer_Legacy)) + " bytes");
    logger.log("--- OFFSETS ANÉMOMÈTRE ACTUEL ---");
    logger.log(String("Offset messageType: ") + String(offsetof(struct_message_Anemometer, messageType)));
    logger.log(String("Offset anemometerId: ") + String(offsetof(struct_message_Anemometer, anemometerId)));
    logger.log(String("Offset macAddress: ") + String(offsetof(struct_message_Anemometer, macAddress)));
    logger.log(String("Offset windSpeed: ") + String(offsetof(struct_message_Anemometer, windSpeed)));
    logger.log("--- OFFSETS ANÉMOMÈTRE LEGACY ---");
    logger.log(String("Legacy offset messageType: ") + String(offsetof(struct_message_Anemometer_Legacy, messageType)));
    logger.log(String("Legacy offset anemometerId: ") + String(offsetof(struct_message_Anemometer_Legacy, anemometerId)));
    logger.log(String("Legacy offset macAddress: ") + String(offsetof(struct_message_Anemometer_Legacy, macAddress)));
    logger.log(String("Legacy offset windSpeed: ") + String(offsetof(struct_message_Anemometer_Legacy, windSpeed)));
    logger.log("--- OFFSETS BATEAU ACTUEL ---");
    logger.log(String("Offset messageType: ") + String(offsetof(struct_message_Boat, messageType)));
    logger.log(String("Offset gpsTimestamp: ") + String(offsetof(struct_message_Boat, gpsTimestamp)));
    logger.log(String("Offset latitude: ") + String(offsetof(struct_message_Boat, latitude)));
    logger.log(String("Offset longitude: ") + String(offsetof(struct_message_Boat, longitude)));
    logger.log(String("Offset speed: ") + String(offsetof(struct_message_Boat, speed)));
    logger.log(String("Offset heading: ") + String(offsetof(struct_message_Boat, heading)));
    logger.log(String("Offset satellites: ") + String(offsetof(struct_message_Boat, satellites)));
    logger.log(String("Offset isGPSRecording: ") + String(offsetof(struct_message_Boat, isGPSRecording)));
    logger.log("--- OFFSETS BATEAU LEGACY ---");
    logger.log(String("Legacy offset messageType: ") + String(offsetof(struct_message_Boat_Legacy, messageType)));
    logger.log(String("Legacy offset latitude: ") + String(offsetof(struct_message_Boat_Legacy, latitude)));
    logger.log(String("Legacy offset longitude: ") + String(offsetof(struct_message_Boat_Legacy, longitude)));
    logger.log(String("Legacy offset speed: ") + String(offsetof(struct_message_Boat_Legacy, speed)));
    logger.log(String("Legacy offset heading: ") + String(offsetof(struct_message_Boat_Legacy, heading)));
    logger.log(String("Legacy offset satellites: ") + String(offsetof(struct_message_Boat_Legacy, satellites)));
    logger.log(String("Legacy offset isGPSRecording: ") + String(offsetof(struct_message_Boat_Legacy, isGPSRecording)));
    logger.log("===============================");
}

/**
 * @brief Synchronize RTC with NTP when WiFi becomes available
 * 
 * This function attempts to synchronize the M5Stack Core2 RTC with an NTP server
 * to ensure accurate timestamps for file naming and data logging.
 */
void syncRTCIfWiFiConnected() {
    if (WiFi.status() == WL_CONNECTED) {
        logger.log("WiFi connected - attempting RTC synchronization");
        if (storage.syncRTCFromNTP("pool.ntp.org", 3600, 3600)) { // GMT+1 + DST
            logger.log("RTC synchronized successfully with NTP");
        } else {
            logger.log("RTC synchronization failed");
        }
    }
}



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
  logger.log(String("Envoi: ") + (status == ESP_NOW_SEND_SUCCESS ? "Succès" : "Échec"));
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
void onReceive(const uint8_t *mac, const uint8_t *incomingDataPtr, int len)
{
  // Debug: Afficher les données brutes reçues
  logger.log(String("=== DONNÉES BRUTES REÇUES ==="));
  logger.log(String("Longueur: ") + String(len) + " bytes");
  String hexData = "";
  for (int i = 0; i < len && i < 32; i++) {  // Limiter à 32 bytes pour éviter le spam
    if (i > 0) hexData += " ";
    if (incomingDataPtr[i] < 16) hexData += "0";
    hexData += String(incomingDataPtr[i], HEX);
  }
  logger.log(String("Hex: ") + hexData);
  logger.log(String("============================="));

  uint8_t messageType = incomingDataPtr[0]; // Premier byte = type
  logger.log(String("Message Type détecté: ") + String(messageType));

  switch (messageType)
  {
  case 1: // GPS Boat

    // Tester d'abord avec la structure legacy si la taille correspond
    if (len == sizeof(struct_message_Boat_Legacy)) {
        logger.log("*** UTILISATION STRUCTURE LEGACY ***");
        struct_message_Boat_Legacy legacyData;
        memcpy(&legacyData, incomingDataPtr, sizeof(legacyData));
        
        // Copier dans la structure actuelle avec des valeurs par défaut
        incomingBoatData.messageType = legacyData.messageType;
        incomingBoatData.gpsTimestamp = 0; // Pas de timestamp GPS disponible
        incomingBoatData.latitude = legacyData.latitude;
        incomingBoatData.longitude = legacyData.longitude;
        incomingBoatData.speed = legacyData.speed;
        incomingBoatData.heading = legacyData.heading;
        incomingBoatData.satellites = legacyData.satellites;
        incomingBoatData.isGPSRecording = legacyData.isGPSRecording;
        
        logger.log("=== DONNÉES BATEAU (LEGACY) ===");
        logger.log(String("Speed (legacy): ") + String(legacyData.speed, 2) + " m/s");
    } else if (len == sizeof(struct_message_Boat)) {
        logger.log("*** UTILISATION STRUCTURE ACTUELLE ***");
        memcpy(&incomingBoatData, incomingDataPtr, sizeof(incomingBoatData));
    } else {
        logger.log("*** TAILLE INATTENDUE ***");
        logger.log(String("Reçu: ") + String(len) + " bytes");
        logger.log(String("Attendu actuel: ") + String(sizeof(struct_message_Boat)) + " bytes");
        logger.log(String("Attendu legacy: ") + String(sizeof(struct_message_Boat_Legacy)) + " bytes");
        // Essayer quand même avec la structure actuelle
        memcpy(&incomingBoatData, incomingDataPtr, min(len, (int)sizeof(incomingBoatData)));
    }

    logger.log(String("=== DONNÉES BATEAU REÇUES ==="));
    logger.log(String("Taille reçue: ") + String(len) + " bytes");
    logger.log(String("Taille structure: ") + String(sizeof(incomingBoatData)) + " bytes");
    logger.log(String("Message Type: ") + String(incomingBoatData.messageType));
    logger.log(String("GPS Timestamp: ") + String(incomingBoatData.gpsTimestamp));
    logger.log(String("Latitude: ") + String(incomingBoatData.latitude, 6));
    logger.log(String("Longitude: ") + String(incomingBoatData.longitude, 6));
    logger.log(String("Speed: ") + String(incomingBoatData.speed, 2) + " m/s");
    logger.log(String("Heading: ") + String(incomingBoatData.heading, 1) + "°");
    logger.log(String("Satellites: ") + String(incomingBoatData.satellites));
    logger.log(String("GPS Recording: ") + String(incomingBoatData.isGPSRecording ? "true" : "false"));
    logger.log(String("=============================="));
    newData = true;

    // Ajouter les données du bateau à la queue pour stockage
    if (isRecording) {
      StorageData storageData;
      // Utiliser le timestamp GPS du bateau
      storageData.timestamp = incomingBoatData.gpsTimestamp;
      storageData.dataType = DATA_TYPE_BOAT;
      storageData.boatData = incomingBoatData;

      // Vérifier si la SD est initialisée avant de stocker
      if (sdInitialized && xSemaphoreTake(storageDataMutex, portMAX_DELAY) == pdTRUE) {
        pendingStorageData.push_back(storageData);
        logger.log("Données bateau ajoutées: " + String(storageData.boatData.latitude, 6) + ", " + String(storageData.boatData.longitude, 6));
        logger.log("Timestamp GPS: " + String(storageData.timestamp));
        logger.log("File d'attente: " + String(pendingStorageData.size()) + " entrées");

        xSemaphoreGive(storageDataMutex);
      } else if (!sdInitialized) {
        logger.log("Données bateau ignorées - SD non initialisée");
      }
    }
    break;

  case 2: // Anemometer

    // Diagnostic similaire au bateau
    logger.log("=== ANÉMOMÈTRE - DONNÉES BRUTES ===");
    logger.log(String("Taille reçue: ") + String(len) + " bytes");
    logger.log(String("Attendu actuel: ") + String(sizeof(struct_message_Anemometer)) + " bytes");
    logger.log(String("Attendu legacy: ") + String(sizeof(struct_message_Anemometer_Legacy)) + " bytes");

    // Tester d'abord avec la structure legacy si la taille correspond
    if (len == sizeof(struct_message_Anemometer_Legacy)) {
        logger.log("*** UTILISATION STRUCTURE ANÉMOMÈTRE LEGACY ***");
        struct_message_Anemometer_Legacy legacyData;
        memcpy(&legacyData, incomingDataPtr, sizeof(legacyData));
        
        // Copier dans la structure actuelle avec conversion
        incomingAnemometerData.messageType = legacyData.messageType;
        
        // Convertir uint32_t ID en string MAC
        String macString = "";
        for (int i = 0; i < 6; i++) {
            if (i > 0) macString += ":";
            if (legacyData.macAddress[i] < 16) macString += "0";
            macString += String(legacyData.macAddress[i], HEX);
        }
        macString.toUpperCase();
        strcpy(incomingAnemometerData.anemometerId, macString.c_str());
        
        // Copier MAC et vitesse
        memcpy(incomingAnemometerData.macAddress, legacyData.macAddress, 6);
        incomingAnemometerData.windSpeed = legacyData.windSpeed;
        
        logger.log("=== DONNÉES ANÉMOMÈTRE (LEGACY) ===");
        logger.log(String("Legacy ID: ") + String(legacyData.anemometerId));
        logger.log(String("Wind Speed (legacy): ") + String(legacyData.windSpeed, 2));
        
    } else if (len == sizeof(struct_message_Anemometer)) {
        logger.log("*** UTILISATION STRUCTURE ANÉMOMÈTRE ACTUELLE ***");
        memcpy(&incomingAnemometerData, incomingDataPtr, sizeof(incomingAnemometerData));
        
        // Formater l'ID de l'anémomètre à partir de l'adresse MAC (si pas déjà formaté)
        if (strlen(incomingAnemometerData.anemometerId) == 0) {
            String macString = "";
            for (int i = 0; i < 6; i++) {
                if (i > 0) macString += ":";
                if (incomingAnemometerData.macAddress[i] < 16) macString += "0";
                macString += String(incomingAnemometerData.macAddress[i], HEX);
            }
            macString.toUpperCase();
            strcpy(incomingAnemometerData.anemometerId, macString.c_str());
        }
        
    } else {
        logger.log("*** TAILLE ANÉMOMÈTRE INATTENDUE ***");
        logger.log(String("Reçu: ") + String(len) + " bytes");
        // Essayer quand même avec la structure actuelle
        memcpy(&incomingAnemometerData, incomingDataPtr, min(len, (int)sizeof(incomingAnemometerData)));
        
        // Formater l'ID comme fallback
        String macString = "";
        for (int i = 0; i < 6; i++) {
            if (i > 0) macString += ":";
            if (incomingAnemometerData.macAddress[i] < 16) macString += "0";
            macString += String(incomingAnemometerData.macAddress[i], HEX);
        }
        macString.toUpperCase();
        strcpy(incomingAnemometerData.anemometerId, macString.c_str());
    }

    logger.log(String("=== DONNÉES ANÉMOMÈTRE FINALES ==="));
    logger.log(String("Message Type: ") + String(incomingAnemometerData.messageType));
    logger.log(String("Anemometer ID: ") + String(incomingAnemometerData.anemometerId));
    logger.log(String("Wind Speed: ") + String(incomingAnemometerData.windSpeed, 2));
    logger.log(String("=================================="));
    newData = true;
    
    // Ajouter les données de l'anémomètre à la queue pour stockage
    if (isRecording) {
      StorageData storageData;
      // Utiliser le timestamp RTC du Core2
      storageData.timestamp = storage.getCurrentTimestamp();
      // Si RTC non disponible, utiliser millis() comme fallback
      if (storageData.timestamp == 0) {
        storageData.timestamp = millis() / 1000; // Convertir en secondes
      }
      storageData.dataType = DATA_TYPE_ANEMOMETER;
      storageData.anemometerData = incomingAnemometerData;

      // Vérifier si la SD est initialisée avant de stocker
      if (sdInitialized && xSemaphoreTake(storageDataMutex, portMAX_DELAY) == pdTRUE) {
        pendingStorageData.push_back(storageData);
        logger.log("Données anémomètre ajoutées: ID=" + String(incomingAnemometerData.anemometerId) + ", Vitesse=" + String(incomingAnemometerData.windSpeed, 2));
        logger.log("Timestamp RTC: " + String(storageData.timestamp));
        logger.log("File d'attente: " + String(pendingStorageData.size()) + " entrées");

        xSemaphoreGive(storageDataMutex);
      } else if (!sdInitialized) {
        logger.log("Données anémomètre ignorées - SD non initialisée");
      }
    }
    
    break;
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
/**
 * @brief Tâche FreeRTOS pour l'écriture des données sur la carte SD
 * 
 * Cette tâche s'exécute en arrière-plan et écrit périodiquement les données
 * reçues sur la carte SD au format JSON. Elle utilise un mutex pour accéder
 * en sécurité à la liste des données en attente.
 */
void storageTask(void* parameter) {
    storage.setLogger(logger);
    
    
    
    while (true) {
        std::vector<StorageData> dataToWrite;
        
        // Récupérer les données en attente de manière thread-safe
        if (xSemaphoreTake(storageDataMutex, portMAX_DELAY) == pdTRUE) {
            if (!pendingStorageData.empty()) {
                dataToWrite = pendingStorageData;
                pendingStorageData.clear();
            }
            xSemaphoreGive(storageDataMutex);
        }
        
        // Écrire les données sur la carte SD
        if (!dataToWrite.empty()) {
            if (!storage.writeDataBatch(dataToWrite)) {
                logger.log("Erreur d'écriture sur SD");
            }
        }
        
        // Attendre 5 secondes avant la prochaine écriture
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/**
 * @brief Réinitialise ESPNow après une déconnexion WiFi
 * 
 * Cette fonction réinitialise la communication ESPNow pour revenir au mode normal
 * après avoir utilisé le WiFi pour le serveur de fichiers.
 */
void reinitializeESPNow() {
  logger.log("Réinitialisation d'ESPNow...");
  
  // Configurer le WiFi en mode station et se déconnecter
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // Augmente la puissance TX WiFi pour améliorer la portée ESP-NOW
  // 19.5 dBm offre un bon compromis portée/consommation (+50-100m de portée)
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  logger.log("Puissance TX réglée à 19.5 dBm");
  
  // Réinitialiser ESPNow
  if (esp_now_init() != ESP_OK) {
    logger.log("Erreur de réinitialisation ESPNow");
    return;
  }
  
  // Réenregistrer les callbacks
  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(onReceive);
  
  // Réajouter les peers
  esp_now_peer_info_t peerInfoBoat = {};
  memcpy(peerInfoBoat.peer_addr, boatAddress, 6);
  peerInfoBoat.channel = 0;  
  peerInfoBoat.encrypt = false;
  
  if (esp_now_add_peer(&peerInfoBoat) != ESP_OK) {
    logger.log("Erreur réajout peer Boat");
    return;
  }
  
  esp_now_peer_info_t peerInfoAnemometer = {};
  memcpy(peerInfoAnemometer.peer_addr, anemometerAddress, 6);
  peerInfoAnemometer.channel = 1;  
  peerInfoAnemometer.encrypt = false;
  
  if (esp_now_add_peer(&peerInfoAnemometer) != ESP_OK) {
    logger.log("Erreur réajout peer Anemometer");
    return;
  }
  
  logger.log("ESPNow réinitialisé avec succès");
}

/**
 * @brief Initializes the M5 device and sets up ESP-NOW communication with boat and anemometer peers.
 * 
 * This function performs the following initialization steps:
 * - Configures and initializes the M5 device with display clearing and power output enabled
 * - Sets up WiFi in station mode and disconnects from any networks
 * - Creates a mutex for protecting storage data access
 * - Initializes ESP-NOW protocol for wireless communication
 * - Registers send and receive callbacks for ESP-NOW
 * - Adds two peers: boat (channel 0) and anemometer (channel 1)
 * - Displays splash screen on the device
 * - Creates a storage task for SD card operations
 * 
 * @note If any critical initialization step fails (mutex creation, ESP-NOW init, peer addition),
 *       the device will restart automatically.
 * @note The function logs the device's MAC address and setup progress.
 */
void setup() {

  auto cfg = M5.config();
  cfg.clear_display = true;
  cfg.output_power = true;

  M5.begin(cfg);
  
  // Initialize RTC if not already set
  // Check if RTC has a valid date (after 2023)
  auto dt = M5.Rtc.getDateTime();
  if (dt.date.year < 2023) {
    // Set a default date/time if RTC is not configured
    // Format: Year, Month, Day, Hour, Minute, Second
    M5.Rtc.setDateTime({{2025, 9, 21}, {12, 0, 0}});
    logger.log("RTC initialized with default date: 2025-09-21 12:00:00");
  } else {
    logger.log("RTC already configured: " + String(dt.date.year) + "-" + 
               String(dt.date.month) + "-" + String(dt.date.date));
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // Augmente la puissance TX WiFi pour améliorer la portée ESP-NOW
  // 19.5 dBm offre un bon compromis portée/consommation (+50-100m de portée)
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  logger.log("Puissance TX réglée à 19.5 dBm");

  logger.enableScreenLogging(false);

  // Créer le mutex pour protéger l'accès aux données de stockage
  storageDataMutex = xSemaphoreCreateMutex();
  if (storageDataMutex == NULL) {
    logger.log("Erreur de création du mutex");
    ESP.restart();
  }

  String macAddress = WiFi.macAddress();
  logger.log("Adresse MAC :");
  logger.log(macAddress);

  if (esp_now_init() != ESP_OK) {
    logger.log("Erreur d'initialisation ESPNow");
    ESP.restart();
  }

  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfoBoat = {};
  memcpy(peerInfoBoat.peer_addr, boatAddress, 6);
  peerInfoBoat.channel = 0;  
  peerInfoBoat.encrypt = false;
  
  if (esp_now_add_peer(&peerInfoBoat) != ESP_OK) {
    logger.log("Erreur ajout peer Boat");
    ESP.restart();
  }

  esp_now_peer_info_t peerInfoAnemometer = {};
  memcpy(peerInfoAnemometer.peer_addr, anemometerAddress, 6);
  peerInfoAnemometer.channel = 1;  
  peerInfoAnemometer.encrypt = false;

  if (esp_now_add_peer(&peerInfoAnemometer) != ESP_OK) {
    logger.log("Erreur ajout peer Anemometer");
    ESP.restart();
  }
  
  esp_now_register_recv_cb(onReceive);

  // Afficher les informations de diagnostic des structures
  printStructureInfo();

  display.showSplashScreen();
  logger.log("Setup started");

  if (!storage.initSD()) {
        logger.log("Erreur d'initialisation du stockage SD");
        display.showSDError("Carte SD non détectée");
        sdInitialized = false;
        // Continuer le setup au lieu de faire planter le système
    } else {
        sdInitialized = true;
        
        // Initialize filename now that RTC is configured
        if (!storage.initializeFileName()) {
            logger.log("Erreur d'initialisation du nom de fichier");
        } else {
            logger.log("Nom de fichier initialisé avec horodatage RTC");
        }
    }
    
  logger.log("Tâche de stockage SD démarrée");

  // Créer la tâche de stockage SD
  xTaskCreate(
    storageTask,        // Fonction de la tâche
    "StorageTask",      // Nom de la tâche
    4096,               // Taille de la pile (en mots)
    NULL,               // Paramètre de la tâche
    1,                  // Priorité de la tâche
    NULL                // Handle de la tâche
  );
  
  // Initialiser le gestionnaire de serveur de fichiers
  fileServer.setLogger(logger);
  if (fileServer.initFileServer()) {
    logger.log("Serveur de fichiers initialisé - Prêt pour connexion WiFi");
  } else {
    logger.log("Échec initialisation serveur de fichiers");
  }
  
  logger.log("Setup complete");
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
  
  // Si la SD n'est pas initialisée, vérifier si l'utilisateur touche l'écran pour réessayer
  if (!sdInitialized) {
    if (M5.Touch.getCount()) {
      logger.log("Tentative de réinitialisation SD...");
      if (storage.initSD()) {
        sdInitialized = true;
        logger.log("SD réinitialisée avec succès");
        if (!storage.initializeFileName()) {
          logger.log("Erreur d'initialisation du nom de fichier");
        }
      } else {
        logger.log("Échec de la réinitialisation SD");
        display.showSDError("Réinitialisation échouée");
        delay(2000); // Attendre 2 secondes avant de permettre un nouveau test
      }
    }
    // Afficher les données malgré l'erreur SD si on a des données
    if (millis() % 5000 < 100) { // Toutes les 5 secondes pendant 100ms
      display.drawDisplay(incomingBoatData, incomingAnemometerData, isRecording, fileServer.isServerActive());
      delay(100);
      display.showSDError("Toucher écran pour réessayer");
    }
    delay(50);
    return;
  }
  
  // Gestion des boutons tactiles en bas de l'écran
  // Utiliser wasPressed() pour détecter uniquement le DÉBUT de l'appui
  // et éviter les détections multiples pendant que le doigt reste posé
  if (M5.Touch.getCount()) {
    auto t = M5.Touch.getDetail();
    
    // Vérifier si c'est un nouvel appui (wasPressed) et non un maintien
    if (t.wasPressed() && t.y > 200) {  // Zone des boutons en bas de l'écran
      
      unsigned long currentTime = millis();
      logger.log(String("Touch PRESSED à x=") + String(t.x) + ", y=" + String(t.y));
      
      // Bouton 1 (gauche) - Gestion enregistrement GPS
      if (t.x < 107) {  // Premier tiers de l'écran (0-106px)
        // Vérifier le debouncing pour ce bouton spécifique
        if (currentTime - lastTouchTimeButton1 < TOUCH_DEBOUNCE_MS) {
          logger.log("Appui ignoré sur bouton GPS - debouncing actif");
          return;
        }
        lastTouchTimeButton1 = currentTime;
        
        // Toggle de l'enregistrement GPS
        isRecording = !isRecording;
        logger.log(String("Enregistrement GPS ") + (isRecording ? "activé" : "désactivé"));
        // L'affichage sera rafraîchi automatiquement dans la boucle principale
      }
      
      // Bouton 2 (centre) - Réservé pour usage futur
      else if (t.x >= 107 && t.x <= 213) {  // Deuxième tiers (107-213px)
        // Vérifier le debouncing pour ce bouton spécifique
        if (currentTime - lastTouchTimeButton2 < TOUCH_DEBOUNCE_MS) {
          logger.log("Appui ignoré sur bouton central - debouncing actif");
          return;
        }
        lastTouchTimeButton2 = currentTime;
        
        logger.log("Bouton central pressé (non assigné)");
        // Fonctionnalité future
      }
      
      // Bouton 3 (droite) - Gestion serveur de fichiers
      else if (t.x > 213) {  // Dernier tiers de l'écran (214-320px)
        // Vérifier le debouncing pour ce bouton spécifique
        if (currentTime - lastTouchTimeButton3 < TOUCH_DEBOUNCE_MS) {
          logger.log("Appui ignoré sur bouton WiFi - debouncing actif");
          return;
        }
        lastTouchTimeButton3 = currentTime;
        
        logger.log("Bouton serveur de fichiers détecté");
        logger.log(String("État serveur AVANT: ") + (fileServer.isServerActive() ? "ACTIF" : "INACTIF"));
        
        if (!fileServer.isServerActive()) {
          logger.log("Démarrage du serveur de fichiers HTTP...");
          bool startResult = fileServer.startFileServer();
          logger.log(String("Résultat startFileServer(): ") + (startResult ? "SUCCÈS" : "ÉCHEC"));
          logger.log(String("État serveur APRÈS start: ") + (fileServer.isServerActive() ? "ACTIF" : "INACTIF"));
          
          if (startResult) {
            display.showFileServerStatus(true, fileServer.getServerIP());
            logger.log("Serveur de fichiers actif sur: http://" + fileServer.getServerIP());
            
            // Synchronize RTC with NTP now that WiFi is connected
            syncRTCIfWiFiConnected();
            
            // Redessiner les boutons pour afficher le bouton WiFi en VERT
            display.drawButtonLabels(isRecording, true);
            
            // L'affichage sera rafraîchi automatiquement après le message
          } else {
            logger.log("Erreur: Impossible de démarrer le serveur de fichiers");
            display.showFileServerStatus(false, "Erreur config WiFi");
          }
        } else {
          logger.log("Arrêt du serveur de fichiers HTTP...");
          bool stopResult = fileServer.stopFileServer();
          logger.log(String("Résultat stopFileServer(): ") + (stopResult ? "SUCCÈS" : "ÉCHEC"));
          logger.log(String("État serveur APRÈS stop: ") + (fileServer.isServerActive() ? "ACTIF" : "INACTIF"));
          
          if (stopResult) {
            display.showFileServerStatus(false, "");
            logger.log("Serveur de fichiers désactivé, retour en mode ESPNow");
            
            // Réinitialiser ESPNow après la déconnexion WiFi
            reinitializeESPNow();
            
            // Redessiner les boutons pour afficher le bouton WiFi en ROUGE
            display.drawButtonLabels(isRecording, false);
            
            // L'affichage sera rafraîchi automatiquement après le message
          }
        }
        
        logger.log("Fin traitement bouton serveur de fichiers");
      }
    }
  }
  
  // Gérer les requêtes du serveur de fichiers
  fileServer.handleClient();
  
  // Mettre à jour l'affichage temporaire du serveur (non-bloquant)
  display.updateServerMessageDisplay();
  
  // Vérifier si un refresh est nécessaire après un message serveur
  // MAIS ne pas rafraîchir si le serveur est actif (on veut garder l'URL affichée)
  if (display.needsRefresh() && !fileServer.isServerActive()) {
    logger.log("Refresh automatique après message serveur");
    display.drawDisplay(incomingBoatData, incomingAnemometerData, isRecording, fileServer.isServerActive());
  }
 
  // Ne pas rafraîchir l'affichage si le serveur de fichiers est actif
  // pour garder l'URL visible à l'écran
  if (!fileServer.isServerActive()) {
    if (newData) {
      display.drawDisplay(incomingBoatData, incomingAnemometerData, isRecording, fileServer.isServerActive());
      newData = false;
    }
    
    // Refresh périodique pour s'assurer que l'état des boutons est correct
    static unsigned long lastPeriodicRefresh = 0;
    if (millis() - lastPeriodicRefresh > 1000) { // Toutes les secondes
      lastPeriodicRefresh = millis();
      
      // Debug: Afficher l'état du serveur périodiquement
      static bool lastServerState = false;
      bool currentServerState = fileServer.isServerActive();
      if (currentServerState != lastServerState) {
        logger.log(String("CHANGEMENT État serveur: ") + (currentServerState ? "ACTIF" : "INACTIF"));
        lastServerState = currentServerState;
      }
      
      display.drawDisplay(incomingBoatData, incomingAnemometerData, isRecording, fileServer.isServerActive());
    }
  } else {
    // Serveur actif : consommer le flag newData mais ne pas rafraîchir l'écran
    if (newData) {
      newData = false;
      logger.log("Données reçues mais affichage suspendu (serveur actif)");
    }
  }

  delay(50);
}

