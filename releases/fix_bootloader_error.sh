#!/bin/bash
# Script de correction pour l'erreur "invalid header: 0x2068746f"
# OpenSailingRC Display V1.0.1 - Fix M5Burner Bootloader Issue

echo "🔧 CORRECTIF ERREUR BOOTLOADER M5BURNER"
echo "========================================"
echo ""
echo "Ce script corrige l'erreur 'invalid header: 0x2068746f'"
echo "qui survient quand M5Burner ne flash que le firmware principal"
echo "sans le bootloader et les partitions."
echo ""

# Variables
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FIRMWARE_DIR="$SCRIPT_DIR/v1.0.1"
BOOTLOADER="$FIRMWARE_DIR/bootloader.bin"
PARTITIONS="$FIRMWARE_DIR/partitions.bin"
FIRMWARE="$FIRMWARE_DIR/OpenSailingRC_Display_v1.0.1.bin"

# Vérification des prérequis
echo "🔍 Vérification des prérequis..."

# Vérifier esptool
if ! command -v python3 &> /dev/null || ! python3 -c "import esptool" &> /dev/null; then
    echo "❌ ESPTool non trouvé. Installation en cours..."
    pip3 install esptool
    if [ $? -ne 0 ]; then
        echo "❌ Échec installation ESPTool. Installez manuellement:"
        echo "   pip3 install esptool"
        exit 1
    fi
    echo "✅ ESPTool installé avec succès"
else
    echo "✅ ESPTool disponible"
fi

# Vérifier les fichiers firmware
echo ""
echo "📁 Vérification des fichiers firmware..."
if [ ! -f "$BOOTLOADER" ]; then
    echo "❌ Bootloader non trouvé: $BOOTLOADER"
    exit 1
fi
if [ ! -f "$PARTITIONS" ]; then
    echo "❌ Partitions non trouvées: $PARTITIONS"
    exit 1
fi
if [ ! -f "$FIRMWARE" ]; then
    echo "❌ Firmware non trouvé: $FIRMWARE"
    exit 1
fi
echo "✅ Tous les fichiers firmware sont présents"

# Détection automatique du port
echo ""
echo "🔌 Détection du port série..."
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    PORT=$(ls /dev/cu.usbserial-* 2>/dev/null | head -n 1)
    if [ -z "$PORT" ]; then
        PORT=$(ls /dev/cu.SLAB_USBtoUART 2>/dev/null | head -n 1)
    fi
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    PORT=$(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | head -n 1)
else
    # Windows (via Git Bash ou WSL)
    echo "⚠️  Détection automatique non supportée sur Windows"
    echo "   Spécifiez le port manuellement (ex: COM3)"
    read -p "Port série (ex: COM3): " PORT
fi

if [ -z "$PORT" ]; then
    echo "❌ Aucun port série détecté. Connectez votre M5Stack Core2."
    echo ""
    echo "Ports possibles:"
    echo "  macOS   : /dev/cu.usbserial-XXXXXXXX"
    echo "  Linux   : /dev/ttyUSB0 ou /dev/ttyACM0"
    echo "  Windows : COM3, COM4, etc."
    echo ""
    read -p "Spécifiez le port manuellement: " PORT
    if [ -z "$PORT" ]; then
        echo "❌ Port non spécifié. Arrêt."
        exit 1
    fi
fi
echo "✅ Port série détecté: $PORT"

# Confirmation utilisateur
echo ""
echo "⚠️  ATTENTION: Cette opération va EFFACER complètement la flash de votre M5Stack Core2"
echo "   et installer le firmware OpenSailingRC Display V1.0.1 avec bootloader."
echo ""
echo "📊 Fichiers à flasher:"
echo "   • 0x1000  : bootloader.bin ($(du -h "$BOOTLOADER" | cut -f1))"
echo "   • 0x8000  : partitions.bin ($(du -h "$PARTITIONS" | cut -f1))"
echo "   • 0x10000 : OpenSailingRC_Display_v1.0.1.bin ($(du -h "$FIRMWARE" | cut -f1))"
echo ""
read -p "Continuer? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "❌ Opération annulée par l'utilisateur"
    exit 0
fi

# Étape 1: Effacement complet
echo ""
echo "🧹 ÉTAPE 1: Effacement complet de la flash..."
echo "============================================="
python3 -m esptool --chip esp32 --port "$PORT" --baud 115200 erase_flash
if [ $? -ne 0 ]; then
    echo "❌ Échec de l'effacement. Vérifiez la connexion."
    exit 1
fi
echo "✅ Flash effacée avec succès"

# Attente stabilisation
echo ""
echo "⏳ Attente stabilisation (3 secondes)..."
sleep 3

# Étape 2: Flash système complet
echo ""
echo "🔥 ÉTAPE 2: Flash système complet..."
echo "===================================="
python3 -m esptool --chip esp32 --port "$PORT" --baud 115200 \
    --before default_reset --after hard_reset write_flash -z \
    --flash_mode dio --flash_freq 80m --flash_size 16MB \
    0x1000 "$BOOTLOADER" \
    0x8000 "$PARTITIONS" \
    0x10000 "$FIRMWARE"

if [ $? -ne 0 ]; then
    echo "❌ Échec du flash. Réessayez avec une vitesse plus lente:"
    echo "   Remplacez --baud 115200 par --baud 57600"
    exit 1
fi

# Succès
echo ""
echo "🎉 SUCCÈS! Firmware installé avec succès"
echo "========================================"
echo ""
echo "✅ Bootloader flashé à 0x1000"
echo "✅ Partitions flashées à 0x8000" 
echo "✅ Firmware flashé à 0x10000"
echo ""
echo "🚀 Votre M5Stack Core2 devrait maintenant démarrer avec:"
echo "   1. Splash screen 'WIND/BOAT' (2 secondes)"
echo "   2. Interface principale OpenSailingRC"
echo ""
echo "📝 Si vous voyez encore 'invalid header', reconnectez le M5Stack"
echo "   et vérifiez le port série à 115200 baud."
echo ""
echo "🔗 Support: https://github.com/FRA-222/OpenSailingRC-Display/issues"