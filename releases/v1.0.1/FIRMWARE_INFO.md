# Firmware Information - OpenSailingRC Display V1.0.1

## 📋 Fichiers inclus

### Fichiers principaux
- **OpenSailingRC_Display_v1.0.1.bin** : Firmware principal (1.048.285 bytes)
- **bootloader.bin** : Bootloader ESP32 (17.536 bytes)  
- **partitions.bin** : Table de partitions (3.072 bytes)

### Documentation
- **RELEASE_NOTES_V1.0.1.md** : Notes de release détaillées
- **FIRMWARE_INFO.md** : Ce fichier d'information

## 🔧 Installation

### Méthode 1 : PlatformIO (recommandée)
```bash
# Depuis le dossier du projet
~/.platformio/penv/bin/platformio run --target upload --upload-port /dev/cu.usbserial-XXXXXXXX
```

### Méthode 2 : esptool.py
```bash
# Installation firmware complet
esptool.py --chip esp32 --port /dev/cu.usbserial-XXXXXXXX --baud 921600 \
  --before default_reset --after hard_reset write_flash -z \
  --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 OpenSailingRC_Display_v1.0.1.bin
```

### Méthode 3 : Mise à jour firmware uniquement
```bash
# Si bootloader et partitions déjà installés
esptool.py --chip esp32 --port /dev/cu.usbserial-XXXXXXXX --baud 921600 \
  write_flash 0x10000 OpenSailingRC_Display_v1.0.1.bin
```

## ⚙️ Configuration

### WiFi (optionnel)
Créer le fichier `wifi_config.json` sur la carte SD :
```json
{
  "ssid": "VotreWiFi",
  "password": "VotreMotDePasse"
}
```

### Cartes SD supportées
- Format : FAT32
- Taille : 1GB à 32GB recommandé
- Insertion à chaud supportée avec réinitialisation

## 🔍 Vérification

Après installation, vérifiez via série (115200 baud) :
- Message "Setup complete"
- Pas d'erreur de compilation
- Affichage sur écran M5Stack

## 🚨 Dépannage

### Erreurs communes
- **Carte SD** : Message d'erreur rouge → Insérer carte ou toucher écran
- **WiFi** : Bouton reste rouge → Vérifier credentials dans le code
- **ESP-NOW** : Pas de données → Vérifier adresses MAC des devices

### Support
- Logs série pour diagnostic détaillé
- GitHub Issues pour problèmes persistants

---
**Build Date** : Dim 28 sep 2025 11:44:09 CEST
**Platform** : ESP32 (M5Stack Core2)
**Framework** : Arduino
