# Instructions de Flash - OpenSailingRC-Display v1.0.0

## 📦 Contenu de la release

- **`firmware.bin`** - Firmware principal de l'application
- **`bootloader.bin`** - Bootloader ESP32
- **`partitions.bin`** - Table des partitions Flash

## 🛠️ Méthodes de flash

### Méthode 1 : Avec esptool (recommandée)

#### Installation d'esptool
```bash
pip install esptool
```

#### Flash complet (tous les binaires)
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 \
  --before default_reset --after hard_reset write_flash -z \
  --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin
```

#### Flash firmware seulement (mise à jour)
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 \
  --before default_reset --after hard_reset write_flash -z \
  --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x10000 firmware.bin
```

### Méthode 2 : Avec PlatformIO

1. **Cloner le projet source**
   ```bash
   git clone https://github.com/FRA-222/OpenSailingRC-Display.git
   cd OpenSailingRC-Display
   ```

2. **Flash avec PlatformIO**
   ```bash
   platformio run --target upload
   ```

### Méthode 3 : Avec ESP32 Flash Download Tool

1. **Télécharger** [ESP32 Flash Download Tool](https://www.espressif.com/en/support/download/other-tools)
2. **Configurer les adresses :**
   - `bootloader.bin` → `0x1000`
   - `partitions.bin` → `0x8000`
   - `firmware.bin` → `0x10000`
3. **Paramètres :**
   - SPI Speed: `80MHz`
   - SPI Mode: `DIO`
   - Flash Size: `16MB`

## 🔧 Identification du port série

### Windows
- Généralement `COM3`, `COM4`, etc.
- Vérifier dans le Gestionnaire de périphériques

### macOS
```bash
ls /dev/tty.usbserial-*
# ou
ls /dev/tty.SLAB_USBtoUART
```

### Linux
```bash
ls /dev/ttyUSB*
# ou
ls /dev/ttyACM*
```

## ⚠️ Prérequis et précautions

### Hardware requis
- **M5Stack Core2 v1.1** ou supérieur
- **Câble USB-C** de qualité
- **Carte microSD** formatée en FAT32 (optionnel)

### Avant le flash
1. **Installer les pilotes** USB-Serial si nécessaire
2. **Fermer** tous les moniteurs série ouverts
3. **Connecter** le M5Stack Core2 via USB-C
4. **Maintenir** le bouton de reset si nécessaire

### Vérification post-flash
1. **Redémarrer** le M5Stack Core2
2. **Vérifier** l'affichage de l'interface GPS
3. **Tester** la réception ESP-NOW (si voilier configuré)
4. **Valider** le mode serveur de fichiers WiFi

## 🐛 Résolution de problèmes

### Erreurs communes

#### `Failed to connect to ESP32`
- Vérifier le câble USB-C
- Maintenir le bouton reset pendant le flash
- Essayer un autre port USB
- Réduire la vitesse de baud à `115200`

#### `Invalid head of packet`
- Flash corrompu, recommencer le flash complet
- Vérifier l'intégrité des fichiers binaires

#### `No such file or directory`
- Vérifier les chemins des fichiers binaires
- S'assurer que tous les fichiers sont présents

### Support
- **Issues GitHub** : [Signaler un problème](https://github.com/FRA-222/OpenSailingRC-Display/issues)
- **Documentation** : [Wiki du projet](https://github.com/FRA-222/OpenSailingRC-Display/wiki)

## 📋 Checksums (vérification d'intégrité)

Vérifiez l'intégrité des fichiers avec ces checksums SHA256 :

```bash
# Générer les checksums
sha256sum *.bin
```

| Fichier | SHA256 |
|---------|--------|
| firmware.bin | *À générer après build* |
| bootloader.bin | *À générer après build* |
| partitions.bin | *À générer après build* |

---

**Version :** v1.0.0  
**Date :** 21 septembre 2025  
**Compatibilité :** M5Stack Core2 v1.1+  
**Auteur :** Philippe Hubert