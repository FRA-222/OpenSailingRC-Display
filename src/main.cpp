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
bool isRecording = false;

uint8_t boatAddress[] = {0x24, 0xA1, 0x60, 0x45, 0xE7, 0xF8};  //Boat2
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
  uint8_t messageType = incomingDataPtr[0]; // Premier byte = type

  switch (messageType)
  {
  case 1: // GPS Boat

    memcpy(&incomingBoatData, incomingDataPtr, sizeof(incomingBoatData));

    logger.log(String("Latitude: ") + String(incomingBoatData.latitude, 6));
    logger.log(String("Longitude: ") + String(incomingBoatData.longitude, 6));
    newData = true;

    // Ajouter les données à la queue pour stockage
    StorageData storageData;
    storageData.timestamp = millis();
    storageData.boatData = incomingBoatData;
    storageData.anemometerData = incomingAnemometerData; // Données actuelles de l'anémomètre

    if (isRecording) {
      if (xSemaphoreTake(storageDataMutex, portMAX_DELAY) == pdTRUE) {
        pendingStorageData.push_back(storageData);
        logger.log("Données ajoutées à la file d'attente de stockage: " + String(storageData.boatData.latitude) + ", " + String(storageData.boatData.longitude) + ", " + String(storageData.anemometerData.windSpeed));
        logger.log("Données ajoutées à la file d'attente de stockage: " + String(pendingStorageData.size()));

        xSemaphoreGive(storageDataMutex);
      }
    }
    break;

  case 2: // Anemometer

    memcpy(&incomingAnemometerData, incomingDataPtr, sizeof(incomingAnemometerData));

    logger.log(String("Anemometer ID: ") + String(incomingAnemometerData.anemometerId));
    logger.log(String("Wind Speed: ") + String(incomingAnemometerData.windSpeed, 2));
    newData = true;
    
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
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

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

  display.showSplashScreen();
  logger.log("Setup started");

  if (!storage.initSD()) {
        logger.log("Erreur d'initialisation du stockage SD");
        vTaskDelete(NULL);
        return;
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
  
  // Gestion des boutons tactiles en bas de l'écran
  if (M5.Touch.getCount()) {
    auto t = M5.Touch.getDetail();
    if (t.y > 200) {  // Zone des boutons en bas de l'écran
      
      // Bouton 1 (gauche) - Gestion enregistrement GPS
      if (t.x < 107) {  // Premier tiers de l'écran (0-106px)
        // Toggle de l'enregistrement GPS
        isRecording = !isRecording;
        logger.log(String("Enregistrement GPS ") + (isRecording ? "activé" : "désactivé"));
        display.drawDisplay(incomingBoatData, incomingAnemometerData, isRecording, fileServer.isServerActive());
      }
      
      // Bouton 3 (droite) - Gestion serveur de fichiers
      else if (t.x > 213) {  // Dernier tiers de l'écran (214-320px)
        if (!fileServer.isServerActive()) {
          logger.log("Démarrage du serveur de fichiers HTTP...");
          if (fileServer.startFileServer()) {
            display.showFileServerStatus(true, fileServer.getServerIP());
            logger.log("Serveur de fichiers actif sur: http://" + fileServer.getServerIP());
            // Rafraîchir l'affichage pour montrer le nouveau statut
            display.drawDisplay(incomingBoatData, incomingAnemometerData, isRecording, fileServer.isServerActive());
          } else {
            logger.log("Erreur: Impossible de démarrer le serveur de fichiers");
            display.showFileServerStatus(false, "Erreur config WiFi");
          }
        } else {
          logger.log("Arrêt du serveur de fichiers HTTP...");
          if (fileServer.stopFileServer()) {
            display.showFileServerStatus(false, "");
            logger.log("Serveur de fichiers désactivé, retour en mode ESPNow");
            
            // Réinitialiser ESPNow après la déconnexion WiFi
            reinitializeESPNow();
            
            // Rafraîchir l'affichage pour montrer le nouveau statut
            display.drawDisplay(incomingBoatData, incomingAnemometerData, isRecording, fileServer.isServerActive());
          }
        }
      }
      
      // Attendre que l'utilisateur relâche le doigt
      delay(200);
    }
  }
  
  // Gérer les requêtes du serveur de fichiers
  fileServer.handleClient();
 
  if (newData) {
    display.drawDisplay(incomingBoatData, incomingAnemometerData, isRecording,fileServer.isServerActive());
    newData = false;
  }

  delay(50);
}

