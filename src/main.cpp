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
#include <esp_wifi.h>
#include <math.h>
#include <vector>
#include <map>
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
struct_message_Boat incomingBoatData = {};
struct_message_Anemometer incomingAnemometerData = {};
struct_message_Buoy incomingBuoyData = {};
struct_message_HubStatus incomingHubStatus = {};

// Timestamps de réception des données (géré localement)
unsigned long boatDataTimestamp = 0;
unsigned long anemometerDataTimestamp = 0;
unsigned long buoyDataTimestamp = 0;
unsigned long hubStatusTimestamp = 0;

// Structure pour gérer plusieurs bateaux
typedef struct BoatInfo {
    struct_message_Boat data;
    uint8_t macAddress[6];
    unsigned long lastUpdate; // Timestamp de la dernière réception
    String boatId; // "BOAT1", "BOAT2", etc.
    uint32_t lastSequenceNumber; // Dernier numéro de séquence reçu
    uint32_t receivedPackets; // Compteur de paquets reçus
    uint32_t lostPackets; // Compteur de paquets perdus
    uint32_t lastStoredSequence; // Dernier numéro de séquence stocké sur SD (pour éviter les doublons)
} BoatInfo;

// Map pour stocker les données de plusieurs bateaux (clé = adresse MAC convertie en String)
std::map<String, BoatInfo> detectedBoats;
std::vector<String> boatMacList; // Liste ordonnée des adresses MAC pour navigation
int selectedBoatIndex = 0; // Index du bateau actuellement sélectionné
const unsigned long BOAT_TIMEOUT_MS = 30000; // 30 secondes - timeout pour retirer un bateau

// Structure pour gérer les données de plusieurs bouées GPS
typedef struct BuoyInfo {
    struct_message_Buoy data;
    unsigned long lastUpdate; // Timestamp de la dernière réception
} BuoyInfo;

// Map pour stocker les données de plusieurs bouées (clé = buoyId)
std::map<uint8_t, BuoyInfo> detectedBuoys;
const unsigned long BUOY_TIMEOUT_MS = 10000; // 10 secondes - timeout données bouée
const unsigned long HUB_TIMEOUT_MS = 10000; // 10 secondes - timeout statut Hub
unsigned long lastBuoyUpdateTimestamp = 0; // Timestamp de la dernière mise à jour bouée
volatile float lastComputedWindDirection = -1; // Dernière direction du vent calculée (pour stockage anémomètre)
volatile bool sdWriteError = false; // Flag d'erreur d'écriture SD (mis par storageTask)

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
    logger.log("Offsets struct_message_Boat:");
    logger.log(String("  messageType: ") + String(offsetof(struct_message_Boat, messageType)));
    logger.log(String("  sequenceNumber: ") + String(offsetof(struct_message_Boat, sequenceNumber)));
    logger.log(String("  gpsTimestamp: ") + String(offsetof(struct_message_Boat, gpsTimestamp)));
    logger.log(String("  latitude: ") + String(offsetof(struct_message_Boat, latitude)));
    logger.log(String("  longitude: ") + String(offsetof(struct_message_Boat, longitude)));
    logger.log(String("  speed: ") + String(offsetof(struct_message_Boat, speed)));
    logger.log(String("  heading: ") + String(offsetof(struct_message_Boat, heading)));
    logger.log(String("  satellites: ") + String(offsetof(struct_message_Boat, satellites)));
    logger.log("--- ANÉMOMÈTRE ---");
    logger.log(String("Taille struct_message_Anemometer: ") + String(sizeof(struct_message_Anemometer)) + " bytes");
    logger.log("Offsets struct_message_Anemometer:");
    logger.log(String("  messageType: ") + String(offsetof(struct_message_Anemometer, messageType)));
    logger.log(String("  anemometerId: ") + String(offsetof(struct_message_Anemometer, anemometerId)));
    logger.log(String("  macAddress: ") + String(offsetof(struct_message_Anemometer, macAddress)));
    logger.log(String("  sequenceNumber: ") + String(offsetof(struct_message_Anemometer, sequenceNumber)));
    logger.log(String("  windSpeed: ") + String(offsetof(struct_message_Anemometer, windSpeed)));
    logger.log(String("  timestamp: ") + String(offsetof(struct_message_Anemometer, timestamp)));
    logger.log("--- BOUÉE GPS ---");
    logger.log(String("Taille struct_message_Buoy: ") + String(sizeof(struct_message_Buoy)) + " bytes");
    logger.log("Offsets struct_message_Buoy:");
    logger.log(String("  buoyId: ") + String(offsetof(struct_message_Buoy, buoyId)));
    logger.log(String("  timestamp: ") + String(offsetof(struct_message_Buoy, timestamp)));
    logger.log(String("  generalMode: ") + String(offsetof(struct_message_Buoy, generalMode)));
    logger.log(String("  navigationMode: ") + String(offsetof(struct_message_Buoy, navigationMode)));
    logger.log(String("  gpsOk: ") + String(offsetof(struct_message_Buoy, gpsOk)));
    logger.log(String("  temperature: ") + String(offsetof(struct_message_Buoy, temperature)));
    logger.log(String("  remainingCapacity: ") + String(offsetof(struct_message_Buoy, remainingCapacity)));
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
 * @brief Convertit une adresse MAC en String
 */
String macToString(const uint8_t* mac) {
    String macStr = "";
    for (int i = 0; i < 6; i++) {
        if (i > 0) macStr += ":";
        if (mac[i] < 16) macStr += "0";
        macStr += String(mac[i], HEX);
    }
    macStr.toUpperCase();
    return macStr;
}

/**
 * @brief Calcule la direction moyenne du vent à partir des bouées actives
 * 
 * Utilise la moyenne circulaire (sin/cos) pour éviter les erreurs
 * autour de 0°/360° (ex: moyenne de 350° et 10° = 0°, pas 180°)
 * 
 * @return Direction moyenne du vent en degrés (0-360), ou -1 si aucune bouée active
 */
float computeAverageWindDirection() {
    unsigned long currentTime = millis();
    float sumSin = 0;
    float sumCos = 0;
    int count = 0;
    
    for (auto& pair : detectedBuoys) {
        // Utiliser uniquement les données récentes (dans le timeout)
        if (currentTime - pair.second.lastUpdate < BUOY_TIMEOUT_MS) {
            float headingRad = pair.second.data.autoPilotTrueHeadingCmde * DEG_TO_RAD;
            sumSin += sin(headingRad);
            sumCos += cos(headingRad);
            count++;
        }
    }
    
    if (count == 0) return -1; // Aucune bouée active
    
    float avgRad = atan2(sumSin / count, sumCos / count);
    float avgDeg = avgRad / DEG_TO_RAD;
    if (avgDeg < 0) avgDeg += 360.0;
    return avgDeg;
}

/**
 * @brief Nettoie les bateaux qui n'ont pas envoyé de données depuis BOAT_TIMEOUT_MS
 */
void cleanupTimedOutBoats() {
    unsigned long currentTime = millis();
    std::vector<String> toRemove;
    
    for (auto& pair : detectedBoats) {
        if (currentTime - pair.second.lastUpdate > BOAT_TIMEOUT_MS) {
            toRemove.push_back(pair.first);
            logger.log("Bateau timeout: " + pair.second.boatId + " (" + pair.first + ")");
        }
    }
    
    for (const String& mac : toRemove) {
        detectedBoats.erase(mac);
        // Retirer de la liste ordonnée
        boatMacList.erase(std::remove(boatMacList.begin(), boatMacList.end(), mac), boatMacList.end());
    }
    
    // Ajuster l'index sélectionné si nécessaire
    if (boatMacList.empty()) {
        selectedBoatIndex = 0;
    } else if (selectedBoatIndex >= boatMacList.size()) {
        selectedBoatIndex = boatMacList.size() - 1;
    }
}

/**
 * @brief Obtient les données du bateau actuellement sélectionné
 */
BoatInfo* getSelectedBoat() {
    cleanupTimedOutBoats();
    
    if (boatMacList.empty()) {
        return nullptr;
    }
    
    String selectedMac = boatMacList[selectedBoatIndex];
    auto it = detectedBoats.find(selectedMac);
    if (it != detectedBoats.end()) {
        return &(it->second);
    }
    return nullptr;
}

/**
 * @brief Passe au bateau suivant dans la liste
 */
void selectNextBoat() {
    cleanupTimedOutBoats();
    
    if (boatMacList.empty()) {
        logger.log("Aucun bateau détecté");
        return;
    }
    
    selectedBoatIndex = (selectedBoatIndex + 1) % boatMacList.size();
    BoatInfo* boat = getSelectedBoat();
    if (boat) {
        logger.log("Bateau sélectionné: " + boat->boatId + " (" + macToString(boat->macAddress) + ")");
        display.forceFullRefresh(); // Force un rafraîchissement complet pour le nouveau bateau
        newData = true; // Force le rafraîchissement de l'affichage
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
  // CALLBACK CRITIQUE : Doit être ULTRA-RAPIDE (< 1ms)
  // Logs de debug désactivés pour éviter de bloquer le callback
  // et manquer des paquets suivants
  
  uint8_t messageType = incomingDataPtr[0]; // Premier byte = type

  switch (messageType)
  {
  case MSG_TYPE_HUB_STATUS: { // Hub Status (messageType=10)
    if (len == sizeof(struct_message_HubStatus)) {
        memcpy(&incomingHubStatus, incomingDataPtr, sizeof(incomingHubStatus));
        hubStatusTimestamp = millis();
        newData = true;
    }
    break;
  }

  case 1: { // GPS Boat

    // Copie rapide des données (vérification de taille exacte)
    if (len != sizeof(struct_message_Boat)) {
        return; // Paquet invalide, ignorer
    }
    memcpy(&incomingBoatData, incomingDataPtr, sizeof(incomingBoatData));
    boatDataTimestamp = millis(); // Timestamp de réception
    
    // Identification du bateau par son nom embarqué (et non la MAC ESP-NOW)
    // Car le Hub retransmet avec sa propre MAC, pas celle du bateau original
    String boatKey(incomingBoatData.name);
    if (boatKey.length() == 0) {
        // Fallback sur MAC si le nom est vide
        static char macBuf[18];
        snprintf(macBuf, sizeof(macBuf), "%02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        boatKey = String(macBuf);
    }
    
    // Vérifier si c'est un nouveau bateau
    bool isNewBoat = (detectedBoats.find(boatKey) == detectedBoats.end());
    
    if (isNewBoat) {
        boatMacList.push_back(boatKey);
    }
    
    // Mettre à jour ou créer l'entrée du bateau
    BoatInfo& boat = detectedBoats[boatKey];
    
    // Détection rapide de paquets perdus et déduplication
    if (!isNewBoat && boat.receivedPackets > 0) {
        uint32_t receivedSeq = incomingBoatData.sequenceNumber;
        
        // Déduplication : ignorer les paquets déjà traités (direct + relayé par Hub)
        if (receivedSeq == boat.lastSequenceNumber) {
            break; // Paquet dupliqué, ignorer
        }
        
        uint32_t expectedSeq = boat.lastSequenceNumber + 1;
        if (receivedSeq > expectedSeq) {
            boat.lostPackets += (receivedSeq - expectedSeq);
        }
        boat.lastSequenceNumber = receivedSeq;
        boat.receivedPackets++;
    } else {
        // Premier paquet de ce bateau
        boat.lastSequenceNumber = incomingBoatData.sequenceNumber;
        boat.receivedPackets = 1;
        boat.lostPackets = 0;
        boat.lastStoredSequence = 0;
    }
    
    boat.data = incomingBoatData;
    memcpy(boat.macAddress, mac, 6);
    boat.lastUpdate = millis();
    boat.boatId = (boatMacList.size() > 1 ? 
                   std::find(boatMacList.begin(), boatMacList.end(), boatKey) - boatMacList.begin() + 1 : 
                   1);
    
    newData = true;

    // Stockage ultra-rapide (sans logs verbeux)
    if (isRecording && sdInitialized && 
        incomingBoatData.sequenceNumber != boat.lastStoredSequence) {
      
      if (xSemaphoreTake(storageDataMutex, 0) == pdTRUE) { // Non-bloquant !
        StorageData storageData;
        storageData.timestamp = millis(); // Display clock for Kepler consistency
        storageData.dataType = DATA_TYPE_BOAT;
        storageData.boatData = incomingBoatData;
        pendingStorageData.push_back(storageData);
        xSemaphoreGive(storageDataMutex);
        
        boat.lastStoredSequence = incomingBoatData.sequenceNumber;
      }
      // Si mutex occupé, on abandonne ce paquet (mieux que bloquer le callback)
    }
    break;
  } // Fin case 1

  case 2: { // Anemometer (callback rapide, pas de logs)

    // Vérification de la taille exacte et copie des données
    if (len != sizeof(struct_message_Anemometer)) {
        return; // Paquet invalide, ignorer
    }
    
    memcpy(&incomingAnemometerData, incomingDataPtr, sizeof(incomingAnemometerData));
    
    // Déduplication : ignorer si même sequenceNumber que le dernier reçu
    // (paquet reçu en direct + relayé par Hub)
    static uint32_t lastAnemometerSequence = 0;
    if (incomingAnemometerData.sequenceNumber == lastAnemometerSequence && lastAnemometerSequence != 0) {
        break; // Paquet dupliqué, ignorer
    }
    lastAnemometerSequence = incomingAnemometerData.sequenceNumber;
    
    // Si l'anemometerId est vide, générer depuis l'adresse MAC
    if (strlen(incomingAnemometerData.anemometerId) == 0) {
        snprintf(incomingAnemometerData.anemometerId, sizeof(incomingAnemometerData.anemometerId),
                 "%02X:%02X:%02X:%02X:%02X:%02X",
                 incomingAnemometerData.macAddress[0], incomingAnemometerData.macAddress[1],
                 incomingAnemometerData.macAddress[2], incomingAnemometerData.macAddress[3],
                 incomingAnemometerData.macAddress[4], incomingAnemometerData.macAddress[5]);
    }
    
    anemometerDataTimestamp = millis(); // Timestamp de réception
    
    newData = true;
    
    // Stockage ultra-rapide (sans logs)
    if (isRecording && sdInitialized) {
      if (xSemaphoreTake(storageDataMutex, 0) == pdTRUE) { // Non-bloquant !
        StorageData storageData;
        storageData.timestamp = millis(); // Display clock for Kepler consistency
        storageData.windDirection = lastComputedWindDirection;
        storageData.dataType = DATA_TYPE_ANEMOMETER;
        storageData.anemometerData = incomingAnemometerData;
        pendingStorageData.push_back(storageData);
        xSemaphoreGive(storageDataMutex);
      }
    }
    
    break;
  } // Fin case 2

  default: {
    // Essayer de décoder comme message de bouée GPS autonome
    if (len == sizeof(struct_message_Buoy)) {
      memcpy(&incomingBuoyData, incomingDataPtr, sizeof(incomingBuoyData));
      
      // Déduplication par sequenceNumber et buoyId
      // Ignorer les paquets déjà traités (direct + relayé par Hub)
      BuoyInfo& buoyInfo = detectedBuoys[incomingBuoyData.buoyId];
      if (buoyInfo.lastUpdate > 0 && 
          incomingBuoyData.sequenceNumber == buoyInfo.data.sequenceNumber &&
          incomingBuoyData.sequenceNumber != 0) {
          break; // Paquet dupliqué, ignorer
      }
      
      buoyDataTimestamp = millis();
      
      // Stocker les données par bouée pour le calcul de la direction du vent
      buoyInfo.data = incomingBuoyData;
      buoyInfo.lastUpdate = millis();
      lastBuoyUpdateTimestamp = millis();
      
      newData = true;
      
      // Stockage sur SD (non-bloquant)
      if (isRecording && sdInitialized) {
        if (xSemaphoreTake(storageDataMutex, 0) == pdTRUE) {
          StorageData storageData;
          storageData.timestamp = millis(); // Display clock for Kepler consistency
          storageData.dataType = DATA_TYPE_BUOY;
          storageData.buoyData = incomingBuoyData;
          pendingStorageData.push_back(storageData);
          xSemaphoreGive(storageDataMutex);
        }
      }
    } else {
      // Message inconnu
      logger.log("Message ESP-NOW inconnu (taille: " + String(len) + " bytes)");
    }
    break;
  }
  } // Fin switch
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
                sdWriteError = true;
            } else {
                sdWriteError = false;
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
  
  // Enable ESP-NOW Long Range mode only (all devices use LR)
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
  logger.log("ESP-NOW Long Range ONLY réactivé");
  
  // Set maximum TX power for best range (84 = 21 dBm = maximum power)
  esp_wifi_set_max_tx_power(84);
  logger.log("Puissance TX réglée à 21 dBm (max)");
  
  // Set WiFi to channel 1 for consistency with BoatGPS
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  
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
  
  logger.enableSerialLogging(true);

  // Enable ESP-NOW Long Range mode only (all devices use LR)
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
  
  // Verify Long Range mode is active
  uint8_t protocol = 0;
  esp_wifi_get_protocol(WIFI_IF_STA, &protocol);
  logger.log(String("WiFi protocol: ") + 
    String((protocol & WIFI_PROTOCOL_LR) ? "LR" : "NO-LR"));
  logger.log("ESP-NOW Long Range ONLY mode");
  
  // Set maximum TX power for best range (84 = 21 dBm = maximum power)
  esp_wifi_set_max_tx_power(84);
  logger.log("Puissance TX réglée à 21 dBm (max)");
  
  // Set WiFi to channel 1 for consistency with BoatGPS
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  logger.log("Canal WiFi: 1");

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
  
  // Initialiser les timestamps à 0 pour forcer l'affichage de "--" au démarrage
  boatDataTimestamp = 0;
  anemometerDataTimestamp = 0;

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
  
  // Calculer la direction moyenne du vent à partir des bouées actives
  float avgWindDir = computeAverageWindDirection();
  unsigned long windDirTs = (avgWindDir >= 0) ? lastBuoyUpdateTimestamp : 0;
  if (avgWindDir >= 0) lastComputedWindDirection = avgWindDir;
  
  // Logs périodiques (toutes les 10 secondes) pour ne pas ralentir le callback
  static unsigned long lastStatsLog = 0;
  if (millis() - lastStatsLog > 10000) {
    lastStatsLog = millis();
    for (auto& pair : detectedBoats) {
      BoatInfo& boat = pair.second;
      if (boat.receivedPackets > 0) {
        float lossRate = (boat.receivedPackets + boat.lostPackets > 0) ? 
                         100.0f * boat.lostPackets / (boat.receivedPackets + boat.lostPackets) : 0;
        Serial.printf("📊 Bateau %d: Seq #%lu, Reçus=%lu, Perdus=%lu (%.1f%%)\n",
                      boat.boatId, boat.lastSequenceNumber, boat.receivedPackets, 
                      boat.lostPackets, lossRate);
      }
    }
    if (pendingStorageData.size() > 0) {
      Serial.printf("💾 File d'attente stockage: %d entrées\n", pendingStorageData.size());
    }
  }
  
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
      bool hubActive = (hubStatusTimestamp > 0) && (millis() - hubStatusTimestamp < HUB_TIMEOUT_MS);
      uint32_t hubRelayed = incomingHubStatus.relayedCommands + incomingHubStatus.relayedStates + incomingHubStatus.relayedGPS + incomingHubStatus.relayedAnemometer;
      display.drawDisplay(incomingBoatData, incomingAnemometerData, isRecording, fileServer.isServerActive(), boatMacList.size(), avgWindDir, windDirTs, sdWriteError, selectedBoatIndex, hubActive, hubRelayed);
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
        if (isRecording) {
          storage.startNewRecording();
        }
        logger.log(String("Enregistrement GPS ") + (isRecording ? "activé" : "désactivé"));
        // L'affichage sera rafraîchi automatiquement dans la boucle principale
      }
      
      // Bouton 2 (centre) - Sélection du bateau
      else if (t.x >= 107 && t.x <= 213) {  // Deuxième tiers (107-213px)
        // Vérifier le debouncing pour ce bouton spécifique
        if (currentTime - lastTouchTimeButton2 < TOUCH_DEBOUNCE_MS) {
          logger.log("Appui ignoré sur bouton central - debouncing actif");
          return;
        }
        lastTouchTimeButton2 = currentTime;
        
        logger.log("Bouton sélection bateau pressé");
        selectNextBoat();
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
            display.drawButtonLabels(isRecording, true, boatMacList.size(), sdWriteError);
            
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
            display.drawButtonLabels(isRecording, false, boatMacList.size(), sdWriteError);
            
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
    bool hubActive = (hubStatusTimestamp > 0) && (millis() - hubStatusTimestamp < HUB_TIMEOUT_MS);
    uint32_t hubRelayed = incomingHubStatus.relayedCommands + incomingHubStatus.relayedStates + incomingHubStatus.relayedGPS + incomingHubStatus.relayedAnemometer;
    display.drawDisplay(incomingBoatData, incomingAnemometerData, isRecording, fileServer.isServerActive(), boatMacList.size(), avgWindDir, windDirTs, sdWriteError, selectedBoatIndex, hubActive, hubRelayed);
  }
 
  // Nettoyer les bateaux avec timeout
  static unsigned long lastCleanup = 0;
  if (millis() - lastCleanup > 5000) { // Vérifier toutes les 5 secondes
    cleanupTimedOutBoats();
    lastCleanup = millis();
  }
  
  // Obtenir le bateau actuellement sélectionné
  BoatInfo* selectedBoat = getSelectedBoat();
  
  // Copier les données du bateau sélectionné dans incomingBoatData pour compatibilité
  if (selectedBoat != nullptr) {
    incomingBoatData = selectedBoat->data;
  }
  
  // Ne pas rafraîchir l'affichage si le serveur de fichiers est actif
  // pour garder l'URL visible à l'écran
  if (!fileServer.isServerActive()) {
    bool hubActive = (hubStatusTimestamp > 0) && (millis() - hubStatusTimestamp < HUB_TIMEOUT_MS);
    uint32_t hubRelayed = incomingHubStatus.relayedCommands + incomingHubStatus.relayedStates + incomingHubStatus.relayedGPS + incomingHubStatus.relayedAnemometer;
    if (newData) {
      display.drawDisplay(incomingBoatData, incomingAnemometerData, isRecording, fileServer.isServerActive(), boatMacList.size(), avgWindDir, windDirTs, sdWriteError, selectedBoatIndex, hubActive, hubRelayed);
      
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
      
      display.drawDisplay(incomingBoatData, incomingAnemometerData, isRecording, fileServer.isServerActive(), boatMacList.size(), avgWindDir, windDirTs, sdWriteError, selectedBoatIndex, hubActive, hubRelayed);
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

