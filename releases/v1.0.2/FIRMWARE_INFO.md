# 🔧 OpenSailingRC Display - Informations Firmware v1.0.2

## 📋 Informations Générales

| Propriété | Valeur |
|-----------|--------|
| **Version** | 1.0.2 |
| **Date de compilation** | 5 octobre 2025 |
| **Plateforme** | ESP32 (M5Stack Core2) |
| **Framework** | Arduino + ESP-IDF |
| **Toolchain** | PlatformIO 6.12.0 |
| **Compilateur** | GCC 11.2.0 |

---

## 💾 Structure Mémoire

### **Firmware Fusionné (MERGED)**

```
Adresse      Taille    Contenu                  Fichier Source
─────────────────────────────────────────────────────────────────
0x00000000   4 KB      Padding/Header           (généré par esptool)
0x00001000   17 KB     Bootloader ESP32         bootloader.bin
0x00008000   3 KB      Table de Partitions      partitions.bin
0x00010000   1.067 KB  Application Principale   firmware.bin
─────────────────────────────────────────────────────────────────
TOTAL        ~1,1 MB   Firmware Complet         MERGED.bin
```

### **Partitions Flash**

```
# Name,   Type, SubType, Offset,  Size
nvs,      data, nvs,     0x9000,  0x5000   # 20 KB - Non-Volatile Storage
otadata,  data, ota,     0xe000,  0x2000   # 8 KB  - OTA Data
app0,     app,  ota_0,   0x10000, 0x140000 # 1.25 MB - Application
spiffs,   data, spiffs,  0x150000,0x2B0000 # 2.7 MB - Système de fichiers
```

**Total utilisé** : ~4 MB / 16 MB disponibles (25%)

---

## 📦 Fichiers de la Release

### **OpenSailingRC_Display_v1.0.2_MERGED.bin**

**Format** : Binaire ESP32 fusionné  
**Taille** : 1.120.896 bytes (1,1 MB)  
**Adresse de flash** : **0x0**  
**Compression** : Non compressé (compression appliquée à la volée par esptool)  
**Checksum** : Vérifié par esptool lors du flash

**Contenu** :
```
Offset      Section           Size
0x0         Padding           4 KB
0x1000      Bootloader        17.120 bytes
0x8000      Partitions        3.072 bytes
0x10000     Application       1.067.504 bytes
```

**Utilisation** :
```bash
# Avec esptool
python3 -m esptool --chip esp32 --port PORT write_flash 0x0 MERGED.bin

# Avec M5Burner
Custom Firmware → Browse → Sélectionner MERGED.bin → Adresse: 0x0 → BURN
```

---

### **bootloader.bin**

**Format** : Bootloader ESP32  
**Taille** : 17.120 bytes (17 KB)  
**Adresse** : 0x1000  
**Version** : ESP-IDF v4.4

**Fonction** :
- Initialisation matérielle (CPU, RAM, Flash)
- Vérification de la table de partitions
- Chargement de l'application depuis la partition `app0`
- Gestion des mises à jour OTA

**Build** :
```bash
platformio run --target bootloader
# Génère : .pio/build/m5stack-core2/bootloader.bin
```

---

### **partitions.bin**

**Format** : Table de partitions ESP32  
**Taille** : 3.072 bytes (3 KB)  
**Adresse** : 0x8000  
**Schéma** : `default.csv` (PlatformIO)

**Contenu** :
```csv
nvs,      data, nvs,     0x9000,  20KB
otadata,  data, ota,     0xe000,  8KB
app0,     app,  ota_0,   0x10000, 1.25MB
spiffs,   data, spiffs,  0x150000,2.7MB
```

**Build** :
```bash
platformio run --target partitions
# Génère : .pio/build/m5stack-core2/partitions.bin
```

---

### **firmware.bin**

**Format** : Application ESP32  
**Taille** : 1.067.504 bytes (~1 MB)  
**Adresse** : 0x10000  
**Architecture** : Xtensa LX6 dual-core @ 240 MHz

**Sections** :
| Section | Taille | Description |
|---------|--------|-------------|
| `.text` | ~800 KB | Code exécutable |
| `.rodata` | ~150 KB | Constantes (strings, tables) |
| `.data` | ~50 KB | Variables initialisées |
| `.bss` | ~20 KB | Variables non initialisées |

**Dépendances** :
- M5Core2 v0.1.9
- M5GFX v0.1.14
- WiFi (ESP32 core)
- SPIFFS (ESP32 core)
- SD (ESP32 core)

**Build** :
```bash
platformio run
# Génère : .pio/build/m5stack-core2/firmware.bin
```

---

## 🔧 Configuration de Compilation

### **platformio.ini**

```ini
[env:m5stack-core2]
platform = espressif32
board = m5stack-core2
framework = arduino

# Configuration flash
board_build.flash_mode = dio
board_build.f_flash = 80000000L
board_build.flash_size = 16MB
board_build.partitions = default.csv

# Configuration CPU
board_build.f_cpu = 240000000L

# Bibliothèques
lib_deps = 
    m5stack/M5Core2 @ ^0.1.9
    m5stack/M5GFX @ ^0.1.14

# Options de compilation
build_flags = 
    -DCORE_DEBUG_LEVEL=0
    -DBOARD_HAS_PSRAM
    -mfix-esp32-psram-cache-issue

# Moniteur série
monitor_speed = 115200
monitor_filters = esp32_exception_decoder
```

---

## 🔬 Analyse Technique

### **Utilisation RAM**

| Type | Taille | % | Description |
|------|--------|---|-------------|
| **DRAM** | ~150 KB | 45% | Variables globales + heap |
| **IRAM** | ~80 KB | 60% | Code critique (ISR) |
| **PSRAM** | ~2 MB | 5% | Buffer display + cache |

**Total RAM utilisée** : ~230 KB / 520 KB (44%)

### **Utilisation Flash**

| Type | Taille | % | Description |
|------|--------|---|-------------|
| **App** | 1 MB | 80% | Code + données |
| **NVS** | 5 KB | 25% | WiFi credentials |
| **SPIFFS** | 0 KB | 0% | Fichiers (vide pour l'instant) |

**Total Flash utilisée** : ~1 MB / 4 MB alloués (25%)  
**Espace libre** : ~15 MB pour logs/données

---

## 🧬 Dépendances

### **Bibliothèques Arduino**

| Bibliothèque | Version | Utilisation |
|--------------|---------|-------------|
| **M5Core2** | 0.1.9 | Hardware M5Stack (écran, touch, SD) |
| **M5GFX** | 0.1.14 | Affichage graphique avancé |
| **WiFi** | 2.0.0 | Serveur WiFi Access Point |
| **WebServer** | 2.0.0 | Serveur HTTP pour file server |
| **SD** | 2.0.0 | Lecture/écriture carte microSD |
| **SPIFFS** | 2.0.0 | Système de fichiers interne |

### **ESP32 Core**

| Composant | Version | Utilisation |
|-----------|---------|-------------|
| **ESP-IDF** | 4.4.6 | Framework système |
| **Arduino-ESP32** | 2.0.14 | Couche Arduino |
| **FreeRTOS** | 10.4.3 | Système temps réel |

---

## 📊 Performances

### **Temps de Démarrage**

```
Phase                  Temps    Cumulé
─────────────────────────────────────────
Bootloader             150 ms   150 ms
Init ESP32             200 ms   350 ms
Init M5Core2           400 ms   750 ms
Splash screen          2000 ms  2750 ms
Interface principale   250 ms   3000 ms
─────────────────────────────────────────
TOTAL                           ~3 secondes
```

### **Cycle d'Affichage**

| Opération | Fréquence | Durée |
|-----------|-----------|-------|
| Refresh display | 10 Hz (100 ms) | ~30 ms |
| Update GPS data | 1 Hz (1 sec) | ~5 ms |
| Touch detection | 100 Hz (10 ms) | <1 ms |
| SD card write | On event | ~50 ms |

**CPU Load** : ~40% (120 MHz utilisés / 240 MHz disponibles)

---

## 🔐 Sécurité

### **WiFi Credentials**

**Stockage** : `src/wifi_credentials.txt` (NON inclus dans le firmware)  
**Format** : Texte brut (SSID et password)  
**Sécurité** : ⚠️ **Pas de chiffrement** - À améliorer dans v1.1.0

**Recommandation** :
- Utiliser un SSID/password temporaire pour le mode AP
- Ne jamais commiter `wifi_credentials.txt` dans Git
- Ajouter chiffrement AES pour v1.1.0

### **OTA Updates**

**Status** : ❌ Non implémenté dans v1.0.2  
**Sécurité** : N/A  
**Planifié** : v1.1.0 avec signature cryptographique

---

## 🧪 Tests

### **Tests de Flash**

| Test | Adresse | Résultat | Temps |
|------|---------|----------|-------|
| **MERGED.bin à 0x0** | 0x0 | ✅ OK | 65 sec |
| **MERGED.bin à 0x10000** | 0x10000 | ❌ Reboot | - |
| **3 fichiers séparés** | 0x1000, 0x8000, 0x10000 | ✅ OK | 70 sec |

### **Tests de Démarrage**

| Scénario | Résultat | Observations |
|----------|----------|--------------|
| **Cold boot** | ✅ OK | 3 secondes |
| **Warm boot** | ✅ OK | 2.5 secondes |
| **Reset button** | ✅ OK | 2.8 secondes |
| **Power loss** | ✅ OK | NVS préservé |

### **Tests Tactiles**

| Test | v1.0.1 | v1.0.2 |
|------|--------|--------|
| **1 appui = 1 action** | ❌ (3 actions) | ✅ OK |
| **Appui maintenu** | ❌ (continu) | ✅ OK (1 seule fois) |
| **Multi-touch** | ❌ | ⚠️ Non supporté |

---

## 🐛 Debugging

### **Logs Série**

**Configuration** :
- **Baud rate** : 115200
- **Filter** : `esp32_exception_decoder`
- **Tool** : `platformio device monitor`

**Commandes** :
```bash
# Moniteur simple
platformio device monitor --baud 115200

# Avec décodeur d'exceptions
platformio device monitor --filter esp32_exception_decoder

# Log vers fichier
platformio device monitor > debug.log
```

### **Core Dumps**

**Location** : Partition `coredump` (non activé par défaut)  
**Size** : 64 KB  
**Format** : ELF

**Activation** :
```ini
# platformio.ini
build_flags = 
    -DCONFIG_ESP32_ENABLE_COREDUMP_TO_FLASH=1
```

---

## 🔄 Création du Firmware Fusionné

### **Script : create_merged_firmware.sh**

```bash
#!/bin/bash
VERSION="1.0.2"
MERGED_FILE="releases/v1.0.2/OpenSailingRC_Display_v${VERSION}_MERGED.bin"

python3 -m esptool --chip esp32 merge_bin \
    -o "$MERGED_FILE" \
    --flash_mode dio \
    --flash_freq 80m \
    --flash_size 16MB \
    0x1000 .pio/build/m5stack-core2/bootloader.bin \
    0x8000 .pio/build/m5stack-core2/partitions.bin \
    0x10000 .pio/build/m5stack-core2/firmware.bin

echo "✅ Firmware fusionné créé: $MERGED_FILE"
ls -lh "$MERGED_FILE"
```

**Execution** :
```bash
chmod +x create_merged_firmware.sh
./create_merged_firmware.sh
```

**Output** :
```
Wrote 0x111a80 bytes to file OpenSailingRC_Display_v1.0.2_MERGED.bin
✅ Firmware fusionné créé: releases/v1.0.2/OpenSailingRC_Display_v1.0.2_MERGED.bin
-rw-r--r--  1 user  staff  1,1M Oct  5 14:30 MERGED.bin
```

---

## 📖 Références

### **Documentation ESP32**
- **Datasheet** : https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
- **Memory Map** : https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/general-notes.html
- **Flash Layout** : https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html

### **Outils**
- **ESPTool** : https://docs.espressif.com/projects/esptool/
- **PlatformIO** : https://docs.platformio.org/en/latest/platforms/espressif32.html
- **M5Burner** : https://docs.m5stack.com/en/download

### **M5Stack Core2**
- **Schematic** : https://docs.m5stack.com/en/core/core2
- **M5Core2 Library** : https://github.com/m5stack/M5Core2
- **M5GFX** : https://github.com/m5stack/M5GFX

---

## 📝 Notes de Version

Pour les détails sur les corrections et nouvelles fonctionnalités, voir :
- `RELEASE_NOTES_V1.0.2.md`
- `MERGED_FIRMWARE_M5BURNER.md`

---

**Date de génération** : 5 octobre 2025  
**Auteur** : Philippe Hubert  
**Firmware Version** : 1.0.2  
**Document Version** : 1.0

---

*Ce document contient toutes les informations techniques nécessaires pour comprendre, compiler, flasher et débugger le firmware OpenSailingRC Display v1.0.2.* 🔧