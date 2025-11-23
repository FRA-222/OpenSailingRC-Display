# OpenSailingRC v1.0.3 - Firmware Information

## 📦 Fichiers disponibles

### 1. OpenSailingRC_Display_v1.0.3_MERGED.bin
- **Appareil** : M5Stack Core2
- **Processeur** : ESP32 (dual-core @ 240MHz)
- **Taille** : 1,106,176 octets (1.05 MB)
- **Flash** : 15.9% utilisé sur 16MB
- **RAM** : 1.1% utilisée (48,036 octets sur 4.3MB)
- **Adresse flash** : 0x0

#### Caractéristiques
- Affichage 4 lignes (BOAT vitesse, BOAT cap, WIND vitesse, WIND direction)
- Timeout 5 secondes avec affichage "--"
- Support multi-bateaux
- Serveur de fichiers WiFi
- Enregistrement SD card
- Écran tactile 320x240

### 2. OpenSailingRC_Anemometer_v1.0.3_MERGED.bin
- **Appareil** : M5Stack AtomS3
- **Processeur** : ESP32-S3 (dual-core @ 240MHz)
- **Taille** : 917,856 octets (896 KB)
- **Flash** : 25.4% utilisé sur 8MB
- **RAM** : 14.0% utilisée (45,936 octets sur 320KB)
- **Adresse flash** : 0x0

#### Caractéristiques
- Mesure vitesse du vent
- Broadcast ESP-NOW
- Affichage LCD 128x128
- Puissance TX maximale (19.5 dBm)
- Mise à jour toutes les 2 secondes

### 3. OpenSailingRC_BoatGPS_v1.0.3_MERGED.bin
- **Appareil** : M5Stack AtomS3
- **Processeur** : ESP32-S3 (dual-core @ 240MHz)
- **Taille** : 951,216 octets (929 KB)
- **Flash** : 26.5% utilisé sur 8MB
- **RAM** : 14.4% utilisée (47,144 octets sur 320KB)
- **Adresse flash** : 0x0

#### Caractéristiques
- GPS haute précision
- Broadcast ESP-NOW
- Affichage LCD 128x128
- Enregistrement SD card
- Numéro de séquence pour détection de perte de paquets

## 🔒 Checksums SHA256

```
37747c0c8de64d965b0049bf26a07e7facb3260c956d23fd37a279d2ce094dde  OpenSailingRC_Anemometer_v1.0.3_MERGED.bin
47824faba6a42447122673b4a7028520d3977a7b9c21592f4fe0deb4ceb03ae3  OpenSailingRC_BoatGPS_v1.0.3_MERGED.bin
3afb2f6bfc82f4be241cd3a2b57dfd1ec97780393b2f73a6600154a0cc82d21f  OpenSailingRC_Display_v1.0.3_MERGED.bin
```

## 🔧 Spécifications techniques

### ESP32 (Display)
- Architecture : Xtensa LX6
- Fréquence : 240 MHz
- SRAM : 520 KB
- Flash : 16 MB
- WiFi : 802.11 b/g/n
- Bluetooth : Classic + BLE
- GPIO : 38

### ESP32-S3 (Anémomètre & BoatGPS)
- Architecture : Xtensa LX7
- Fréquence : 240 MHz
- SRAM : 512 KB
- PSRAM : 8 MB
- Flash : 8 MB
- WiFi : 802.11 b/g/n
- Bluetooth : BLE 5.0
- GPIO : 45

## 📡 Communication ESP-NOW

### Protocole
- Fréquence : 2.4 GHz
- Portée : 100-200m (environnement ouvert)
- Latence : 5-10ms
- Mode : Broadcast (FF:FF:FF:FF:FF:FF)
- Puissance TX : 19.5 dBm (maximum)

### Structures de données

#### BoatGPS → Display
```cpp
struct GPSBroadcastPacket {
    int8_t messageType;      // 1 = Boat
    char name[18];           // MAC address
    int boatId;              // Boat ID
    uint32_t sequenceNumber; // Packet counter
    uint32_t gpsTimestamp;   // GPS timestamp
    float latitude;          // Latitude (degrees)
    float longitude;         // Longitude (degrees)
    float speed;             // Speed (knots)
    float heading;           // Heading (degrees)
    uint8_t satellites;      // Satellite count
    unsigned long timestamp; // Reception timestamp
};
// Taille : 64 octets
```

#### Anémomètre → Display
```cpp
struct AnemometerData {
    int8_t messageType;      // 2 = Anemometer
    char anemometerId[18];   // MAC address
    uint8_t macAddress[6];   // MAC bytes
    uint32_t sequenceNumber; // Packet counter
    float windSpeed;         // Wind speed (m/s)
    unsigned long timestamp; // Reception timestamp
};
// Taille : 36 octets
```

## ⚙️ Configuration WiFi

### Display (M5Stack Core2)
```cpp
WiFi.mode(WIFI_STA);
esp_wifi_set_max_tx_power(84);  // 21 dBm
esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
```

### Anémomètre (M5Stack AtomS3)
```cpp
WiFi.mode(WIFI_STA);
WiFi.setTxPower(WIFI_POWER_19_5dBm);  // 19.5 dBm
```

### BoatGPS (M5Stack AtomS3)
```cpp
WiFi.mode(WIFI_STA);
esp_wifi_set_max_tx_power(84);  // 21 dBm
esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
```

## 📊 Performances

### Consommation mémoire

| Composant | Flash utilisé | RAM utilisée |
|-----------|---------------|--------------|
| Display | 1,040,221 bytes (15.9%) | 48,036 bytes (1.1%) |
| Anémomètre | 850,417 bytes (25.4%) | 45,936 bytes (14.0%) |
| BoatGPS | 885,321 bytes (26.5%) | 47,144 bytes (14.4%) |

### Fréquences de mise à jour
- **Anémomètre** : 2000ms (0.5 Hz)
- **BoatGPS** : Variable selon GPS fix
- **Display** : 50ms refresh loop (20 Hz)
- **Timeout** : 5000ms (5 secondes)

### Latences typiques
- ESP-NOW transmission : < 10ms
- Display refresh : 50ms
- Total end-to-end : < 100ms

## 🛠️ Outils de développement

### PlatformIO
- Version : 6.5.0
- Framework : Arduino ESP32 v2.0.14
- Toolchain : xtensa-esp32 / riscv32-esp32s3 v8.4.0

### Bibliothèques principales
- **M5Unified** : v0.1.17 / v0.2.5
- **TinyGPSPlus** : v1.1.0
- **ArduinoJson** : v7.4.2
- **FastLED** : v3.10.2/v3.10.3

## 📋 Compatibilité

### Versions supportées
- ✅ v1.0.3 (actuelle)
- ❌ v1.0.2 (incompatible - taille structure différente)
- ❌ v1.0.1 (incompatible)
- ❌ v1.0.0 (incompatible)

### Migration
**IMPORTANT** : Tous les appareils doivent être en v1.0.3
- Le Display v1.0.3 rejette les paquets des versions antérieures
- Les versions antérieures du Display ne peuvent pas recevoir v1.0.3
- Mise à jour complète du système requise

## 🔍 Débogage

### Logs série
- **Vitesse** : 115200 baud
- **Format** : 8N1
- **Activer** : Connecter USB et ouvrir moniteur série

### Logs typiques

**Display**
```
=== DIAGNOSTIC STRUCTURE ===
Taille struct_message_Boat: 64 bytes
Taille struct_message_Anemometer: 36 bytes
Setup complete
📊 Bateau 1: Seq #123, Reçus=100, Perdus=2 (2.0%)
```

**Anémomètre**
```
✓ ESP-NOW: Initialized in broadcast mode
→ Broadcast #1: Wind Speed: 5.2 m/s
```

**BoatGPS**
```
✓ ESP-NOW: MAC Address: D0:CF:13:0F:D9:DC
→ Broadcast #1: 48.123456,-4.654321 (5.2kts, 45°, 12 sats)
```

## 📞 Support technique

### Vérifications de base
1. Vérifier la version du firmware
2. Vérifier les checksums SHA256
3. Vérifier la puissance WiFi
4. Consulter les logs série
5. Tester la portée ESP-NOW

### Issues connues
- Aucun problème connu en v1.0.3

### Rapporter un bug
1. GitHub Issues : https://github.com/FRA-222/Boat-GPS-Display/issues
2. Inclure : version, logs série, contexte
3. Vérifier les checksums avant de rapporter

---

**Dernière mise à jour** : 23 novembre 2025
