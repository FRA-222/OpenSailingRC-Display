#!/bin/bash

# Script de préparation Release V1.0.1
# OpenSailingRC Display Firmware

echo "🚀 Préparation Release V1.0.1 - OpenSailingRC Display"
echo "=================================================="

# Variables
RELEASE_VERSION="v1.0.1"
PROJECT_DIR="/Users/philippe/Documents/PlatformIO/Projects/OpenSailingRC-Display"
BUILD_DIR="$PROJECT_DIR/.pio/build/m5stack-core2"
RELEASE_DIR="$PROJECT_DIR/releases/$RELEASE_VERSION"

# Créer le dossier de release
echo "📁 Création du dossier de release..."
mkdir -p "$RELEASE_DIR"

# Copier les fichiers binaires
echo "📦 Copie des fichiers firmware..."
cp "$BUILD_DIR/firmware.bin" "$RELEASE_DIR/OpenSailingRC_Display_$RELEASE_VERSION.bin"
cp "$BUILD_DIR/bootloader.bin" "$RELEASE_DIR/"
cp "$BUILD_DIR/partitions.bin" "$RELEASE_DIR/"

# Copier les notes de release
echo "📝 Copie des notes de release..."
cp "$PROJECT_DIR/RELEASE_NOTES_V1.0.1.md" "$RELEASE_DIR/"

# Créer un fichier d'information sans hash SHA256
echo "ℹ️  Création du fichier d'information..."
cat > "$RELEASE_DIR/FIRMWARE_INFO.md" << EOF
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
\`\`\`bash
# Depuis le dossier du projet
~/.platformio/penv/bin/platformio run --target upload --upload-port /dev/cu.usbserial-XXXXXXXX
\`\`\`

### Méthode 2 : esptool.py
\`\`\`bash
# Installation firmware complet
esptool.py --chip esp32 --port /dev/cu.usbserial-XXXXXXXX --baud 921600 \\
  --before default_reset --after hard_reset write_flash -z \\
  --flash_mode dio --flash_freq 80m --flash_size 16MB \\
  0x1000 bootloader.bin \\
  0x8000 partitions.bin \\
  0x10000 OpenSailingRC_Display_v1.0.1.bin
\`\`\`

### Méthode 3 : Mise à jour firmware uniquement
\`\`\`bash
# Si bootloader et partitions déjà installés
esptool.py --chip esp32 --port /dev/cu.usbserial-XXXXXXXX --baud 921600 \\
  write_flash 0x10000 OpenSailingRC_Display_v1.0.1.bin
\`\`\`

## ⚙️ Configuration

### WiFi (optionnel)
Créer le fichier \`wifi_config.json\` sur la carte SD :
\`\`\`json
{
  "ssid": "VotreWiFi",
  "password": "VotreMotDePasse"
}
\`\`\`

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
**Build Date** : $(date)
**Platform** : ESP32 (M5Stack Core2)
**Framework** : Arduino
EOF

# Créer l'archive ZIP dans le répertoire releases
echo "📦 Création de l'archive ZIP..."
cd "$PROJECT_DIR"
zip -r "releases/OpenSailingRC-Display-$RELEASE_VERSION.zip" "releases/$RELEASE_VERSION/"

# Afficher la taille des fichiers
echo "📊 Taille des fichiers de release :"
ls -lh "$RELEASE_DIR"
echo ""
echo "📊 Archive ZIP créée :"
ls -lh "releases/OpenSailingRC-Display-$RELEASE_VERSION.zip"

echo ""
echo "✅ Release V1.0.1 préparée avec succès !"
echo "📁 Dossier : $RELEASE_DIR"
echo ""
echo "🔄 Prochaines étapes :"
echo "1. Vérifiez le contenu du dossier releases/$RELEASE_VERSION/"
echo "2. Créez une release sur GitHub avec ces fichiers"
echo "3. Uploadez le firmware .bin comme asset principal"
echo ""