# Architecture Multi-Appareils OpenSailingRC

## Vue d'ensemble
Ce système supporte maintenant plusieurs bateaux et anémomètres avec stockage séparé des données pour chaque type d'appareil.

## Structure des Données

### Types de Données (DataType enum)
- `DATA_TYPE_BOAT = 1` : Données de bateau (GPS, navigation)
- `DATA_TYPE_ANEMOMETER = 2` : Données d'anémomètre (vitesse du vent)

### Format JSON de Sortie

#### Données de Bateau
```json
{
  "timestamp": 1704096000,
  "type": 1,
  "boat": {
    "messageType": 1,
    "latitude": 45.123456,
    "longitude": 2.654321,
    "speed": 12.5,
    "heading": 270.0,
    "satellites": 8,
    "isGPSRecording": true
  }
}
```

#### Données d'Anémomètre
```json
{
  "timestamp": 1704096001,
  "type": 2,
  "anemometer": {
    "messageType": 2,
    "anemometerId": "AA:BB:CC:DD:EE:FF",
    "windSpeed": 15.8,
    "macAddress": [170, 187, 204, 221, 238, 255]
  }
}
```

## Fonctionnalités Clés

### 1. Horloge Temps Réel (RTC)
- **Composant** : BM8563 intégré au M5Stack Core2
- **Synchronisation** : NTP via WiFi au démarrage
- **Fallback** : Numérotation basée sur la session si RTC indisponible
- **Format fichier** : `YYYYMMDD_HHMMSS.json` ou `session_X.json`

### 2. Stockage Multi-Appareils
- **Union Structure** : Optimisation mémoire avec union C++
- **Type Safety** : Enum DataType pour identification
- **Séparation** : Données bateau et anémomètre dans des objets JSON distincts
- **Identification** : Adresse MAC pour identification unique des anémomètres

### 3. Communication ESP-NOW
- **Protocole** : ESP-NOW pour communication low-latency
- **Gestion** : Callbacks séparés selon le type de données reçues
- **Threading** : Protection mutex FreeRTOS pour accès concurrent

## Avantages de l'Architecture

### Évolutivité
- Support de multiples bateaux indépendants
- Support de multiples anémomètres avec ID unique
- Structure extensible pour nouveaux types d'appareils

### Performance
- Union structure pour optimisation mémoire
- Stockage par batch pour réduire l'usure SD
- Communication ESP-NOW low-latency

### Fiabilité
- RTC avec synchronisation NTP pour timestamps précis
- Fallback de nommage si RTC indisponible
- Protection thread-safe avec mutex

## Utilisation

### Configuration WiFi
```cpp
// Synchronisation RTC via NTP
storage.syncRTCFromNTP("pool.ntp.org", 3600, 0); // GMT+1
```

### Réception Données
```cpp
void onReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
    // Détection automatique du type et stockage approprié
    // Voir main.cpp pour l'implémentation complète
}
```

### Stockage
```cpp
// Stockage immédiat
storage.writeData(storageData);

// Stockage par batch (recommandé)
std::vector<StorageData> dataList;
// ... ajouter données ...
storage.writeDataBatch(dataList);
```

## Fichiers Modifiés

### Storage.h
- Ajout enum `DataType`
- Ajout union `StorageData` pour optimisation mémoire
- Nouvelles méthodes RTC

### Storage.cpp
- Implémentation RTC avec BM8563
- Génération de noms de fichiers intelligente
- Sérialisation JSON adaptée au type de données

### main.cpp
- Callback `onReceive()` modifié pour gestion multi-appareils
- Création de structures `StorageData` séparées par type
- Identification automatique des appareils

## Exemple de Session

1. **Démarrage** : Synchronisation RTC via NTP
2. **Réception** : Données bateau et anémomètres via ESP-NOW
3. **Stockage** : Création d'entrées JSON séparées par type
4. **Fichier** : `20240101_120000.json` avec données mélangées mais typées

Cette architecture permet une gestion flexible et évolutive de multiples appareils tout en maintenant des performances optimales.