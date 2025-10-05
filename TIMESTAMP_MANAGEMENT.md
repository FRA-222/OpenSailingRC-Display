# Gestion des Timestamps - OpenSailingRC

## Vue d'ensemble

Le système OpenSailingRC utilise maintenant deux sources de timestamps distinctes selon le type de données :

### 🚢 **Données de Bateau**
- **Source** : Timestamp GPS du bateau
- **Champ** : `gpsTimestamp` dans la structure `struct_message_Boat`
- **Avantage** : Synchronisation précise avec le temps GPS (UTC)
- **Fiabilité** : Horloge satellite, très précise

### 🌬️ **Données d'Anémomètre**
- **Source** : RTC du M5Stack Core2
- **Champ** : Timestamp généré par `storage.getCurrentTimestamp()`
- **Avantage** : Horloge locale stable et synchronisée via NTP
- **Fallback** : `millis() / 1000` si RTC non disponible

## Architecture Technique

### Structure des Données

#### Bateau (DisplayTypes.h)
```cpp
typedef struct struct_message_Boat {
    int8_t messageType;        // Type de message
    uint32_t gpsTimestamp;     // ⭐ Timestamp GPS (secondes depuis epoch Unix)
    float latitude;            // Position GPS
    float longitude;
    float speed;               // Vitesse en m/s
    float heading;             // Cap en degrés
    uint8_t satellites;        // Nombre de satellites
    bool isGPSRecording;       // État d'enregistrement
} struct_message_Boat;
```

#### Anémomètre (DisplayTypes.h)
```cpp
typedef struct struct_message_Anemometer {
    int8_t messageType;        // Type de message
    char anemometerId[18];     // ID au format MAC "AA:BB:CC:DD:EE:FF"
    uint8_t macAddress[6];     // Adresse MAC
    float windSpeed;           // Vitesse du vent
} struct_message_Anemometer;
```

### Fonction RTC (Storage.cpp)
```cpp
time_t Storage::getCurrentTimestamp() {
    auto datetime = M5.Rtc.getDateTime();
    
    // Vérification de validité (année >= 2023)
    if (datetime.date.year < 2023) {
        return 0;  // RTC non configuré
    }
    
    // Conversion en timestamp Unix
    struct tm timeinfo;
    timeinfo.tm_year = datetime.date.year - 1900;
    timeinfo.tm_mon = datetime.date.month - 1;
    timeinfo.tm_mday = datetime.date.date;
    timeinfo.tm_hour = datetime.time.hours;
    timeinfo.tm_min = datetime.time.minutes;
    timeinfo.tm_sec = datetime.time.seconds;
    
    return mktime(&timeinfo);
}
```

### Logique d'Attribution (main.cpp)
```cpp
void onReceive(const uint8_t *mac, const uint8_t *incomingDataPtr, int len) {
    switch (messageType) {
    case 1: // GPS Boat
        StorageData storageData;
        storageData.timestamp = incomingBoatData.gpsTimestamp;  // ⭐ GPS
        storageData.dataType = DATA_TYPE_BOAT;
        break;
        
    case 2: // Anemometer
        StorageData storageData;
        storageData.timestamp = storage.getCurrentTimestamp();  // ⭐ RTC
        if (storageData.timestamp == 0) {
            storageData.timestamp = millis() / 1000;  // Fallback
        }
        storageData.dataType = DATA_TYPE_ANEMOMETER;
        break;
    }
}
```

## Format JSON Résultant

### Données de Bateau
```json
{
  "timestamp": 1704096000,    // ⭐ Timestamp GPS du bateau
  "type": 1,
  "boat": {
    "messageType": 1,
    "gpsTimestamp": 1704096000,  // ⭐ Même valeur, redondance pour clarté
    "latitude": 45.123456,
    "longitude": 2.654321,
    "speed": 12.5,
    "heading": 270.0,
    "satellites": 8,
    "isGPSRecording": true
  }
}
```

### Données d'Anémomètre
```json
{
  "timestamp": 1704096001,    // ⭐ Timestamp RTC du Core2
  "type": 2,
  "anemometer": {
    "messageType": 2,
    "anemometerId": "AA:BB:CC:DD:EE:FF",
    "windSpeed": 15.8,
    "macAddress": [170, 187, 204, 221, 238, 255]
  }
}
```

## Avantages de cette Approche

### ⏰ **Précision Temporelle**
- **Bateau** : Utilise l'horloge GPS satellite (précision nanoseconde)
- **Anémomètre** : Utilise l'horloge RTC synchronisée NTP (précision milliseconde)

### 🔄 **Cohérence des Données**
- Chaque source de données utilise sa propre référence temporelle
- Évite les décalages dus à la latence de transmission ESP-NOW
- Permet une analyse temporelle précise des événements

### 🛡️ **Robustesse**
- **GPS indisponible** : Le bateau peut transmettre un timestamp à 0
- **RTC non configuré** : Fallback automatique sur `millis()`
- **Synchronisation NTP** : Mise à jour automatique du RTC au démarrage

### 📊 **Analyse des Données**
- **Corrélation temporelle** : Possible entre GPS et RTC
- **Analyse de performance** : Latence réseau calculable
- **Debugging** : Source du timestamp identifiable

## Configuration Requise

### Côté Bateau
```cpp
// Le bateau doit inclure le timestamp GPS dans ses données
boatData.gpsTimestamp = gps.getUnixTime();  // Exemple
```

### Côté Core2
```cpp
// Synchronisation RTC via NTP au démarrage
storage.syncRTCFromNTP("pool.ntp.org", 3600, 3600);  // GMT+1 + DST
```

## Cas d'Usage

### Navigation Précise
- **Bateau** : Timestamp GPS pour trajectoire exacte
- **Vent** : Timestamp RTC pour conditions météo locales

### Analyse Performance
- Comparaison des timestamps pour calculer la latence
- Analyse de la dérive temporelle entre sources

### Debugging
- Identification de la source de problèmes temporels
- Validation de la synchronisation des horloges

Cette architecture garantit une gestion optimale des timestamps selon la nature des données, tout en maintenant la robustesse et la précision du système.