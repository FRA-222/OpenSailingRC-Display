#!/bin/bash
# Script de création d'un firmware fusionné pour M5Burner
# OpenSailingRC Display - Compatible M5Stack Core2
# 
# Ce script fusionne bootloader, partitions et firmware en un seul fichier
# flashable à l'adresse 0x0 avec M5Burner

echo "🔧 CRÉATION FIRMWARE FUSIONNÉ POUR M5BURNER"
echo "============================================"
echo ""

# Variables
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/.pio/build/m5stack-core2"
RELEASE_DIR="$PROJECT_DIR/releases/v1.0.2"

# Fichiers source
BOOTLOADER="$BUILD_DIR/bootloader.bin"
PARTITIONS="$BUILD_DIR/partitions.bin"
FIRMWARE="$BUILD_DIR/firmware.bin"
BOOT_APP0="$BUILD_DIR/boot_app0.bin"

# Fichier de sortie
MERGED_FIRMWARE="$RELEASE_DIR/OpenSailingRC_Display_v1.0.2_MERGED.bin"

# Vérifier que les fichiers existent
echo "📁 Vérification des fichiers source..."
if [ ! -f "$BOOTLOADER" ]; then
    echo "❌ Erreur: bootloader.bin non trouvé"
    echo "   Compilez d'abord le projet avec: platformio run"
    exit 1
fi

if [ ! -f "$PARTITIONS" ]; then
    echo "❌ Erreur: partitions.bin non trouvé"
    exit 1
fi

if [ ! -f "$FIRMWARE" ]; then
    echo "❌ Erreur: firmware.bin non trouvé"
    exit 1
fi

echo "✅ Tous les fichiers source sont présents"
echo ""

# Créer le répertoire de release si nécessaire
mkdir -p "$RELEASE_DIR"

# Afficher les tailles des fichiers
echo "📊 Tailles des fichiers:"
echo "   Bootloader: $(ls -lh "$BOOTLOADER" | awk '{print $5}')"
echo "   Partitions: $(ls -lh "$PARTITIONS" | awk '{print $5}')"
if [ -f "$BOOT_APP0" ]; then
    echo "   Boot App0:  $(ls -lh "$BOOT_APP0" | awk '{print $5}')"
fi
echo "   Firmware:   $(ls -lh "$FIRMWARE" | awk '{print $5}')"
echo ""

# Vérifier si esptool est disponible
if ! command -v python3 &> /dev/null || ! python3 -c "import esptool" &> /dev/null; then
    echo "⚠️  esptool non trouvé. Installation..."
    pip3 install esptool
    if [ $? -ne 0 ]; then
        echo "❌ Échec installation esptool"
        exit 1
    fi
fi

# Créer le firmware fusionné
echo "🔨 Fusion des fichiers en cours..."
echo ""

if [ -f "$BOOT_APP0" ]; then
    # Avec boot_app0.bin (pour OTA)
    python3 -m esptool --chip esp32 merge_bin \
        -o "$MERGED_FIRMWARE" \
        --flash_mode dio \
        --flash_freq 80m \
        --flash_size 16MB \
        0x1000 "$BOOTLOADER" \
        0x8000 "$PARTITIONS" \
        0xe000 "$BOOT_APP0" \
        0x10000 "$FIRMWARE"
else
    # Sans boot_app0.bin (firmware simple)
    python3 -m esptool --chip esp32 merge_bin \
        -o "$MERGED_FIRMWARE" \
        --flash_mode dio \
        --flash_freq 80m \
        --flash_size 16MB \
        0x1000 "$BOOTLOADER" \
        0x8000 "$PARTITIONS" \
        0x10000 "$FIRMWARE"
fi

if [ $? -ne 0 ]; then
    echo "❌ Erreur lors de la fusion"
    exit 1
fi

echo ""
echo "✅ Firmware fusionné créé avec succès!"
echo ""
echo "📦 Fichier généré:"
echo "   $MERGED_FIRMWARE"
echo "   Taille: $(ls -lh "$MERGED_FIRMWARE" | awk '{print $5}')"
echo ""
echo "📋 INSTRUCTIONS POUR M5BURNER:"
echo "================================"
echo ""
echo "1. Ouvrir M5Burner"
echo "2. Aller dans 'Custom Firmware'"
echo "3. Sélectionner: OpenSailingRC_Display_v1.0.2_MERGED.bin"
echo "4. ⚠️  IMPORTANT: Configurer l'adresse à 0x0 (et non 0x10000)"
echo "5. Cocher 'Erase Flash'"
echo "6. Cliquer 'Burn'"
echo ""
echo "💡 Différence avec le firmware classique:"
echo "   • Firmware classique:    Flash à 0x10000 + bootloader + partitions"
echo "   • Firmware fusionné:     Flash à 0x0 (tout inclus)"
echo ""
echo "🎯 Le Core2 devrait maintenant démarrer correctement!"
echo ""

# Créer aussi un README pour M5Burner
cat > "$RELEASE_DIR/M5BURNER_INSTRUCTIONS.txt" << 'EOF'
╔══════════════════════════════════════════════════════════════╗
║  INSTALLATION AVEC M5BURNER - OpenSailingRC Display V1.0.2  ║
╚══════════════════════════════════════════════════════════════╝

📋 FICHIER À UTILISER:
   OpenSailingRC_Display_v1.0.2_MERGED.bin

⚠️  CONFIGURATION IMPORTANTE:
   • Adresse de flash: 0x0 (ZÉRO, pas 0x10000 !)
   • Erase Flash: COCHÉ
   • Baud Rate: 921600 ou 115200

📝 ÉTAPES D'INSTALLATION:

1. Télécharger M5Burner depuis:
   https://docs.m5stack.com/en/download

2. Connecter le M5Stack Core2 via USB-C

3. Ouvrir M5Burner

4. Cliquer sur "Custom Firmware"

5. Sélectionner le fichier:
   OpenSailingRC_Display_v1.0.2_MERGED.bin

6. ⚠️  IMPORTANT: Changer l'adresse à 0x0
   (Par défaut M5Burner met 0x10000, il faut changer !)

7. Cocher "Erase Flash"

8. Sélectionner le port série (COM ou /dev/cu.usbserial-xxx)

9. Cliquer "BURN"

10. Attendre la fin du flash (~1 minute)

✅ VÉRIFICATION:

Après le flash, le M5Stack Core2 devrait:
• Redémarrer automatiquement
• Afficher le splash "WIND / BOAT" pendant 2 secondes
• Afficher l'interface principale avec:
  - Indicateur BOAT (vitesse, cap, satellites)
  - Indicateur BUOY (vitesse)
  - 3 boutons en bas: STOP, (vide), WIFI

🆘 DÉPANNAGE:

❌ Core2 reboot en boucle:
   → Vérifier que l'adresse est bien 0x0 (pas 0x10000)
   → Effacer complètement la flash et réessayer

❌ "invalid header" dans les logs:
   → Flasher avec esptool au lieu de M5Burner
   → Utiliser le script fix_bootloader_error.sh

❌ Écran noir:
   → Maintenir boutons A+C pendant 10 secondes (reset)
   → Flasher à nouveau

📚 DOCUMENTATION COMPLÈTE:

Voir les fichiers dans le répertoire releases/:
• M5BURNER_INSTALLATION_GUIDE.md (guide détaillé)
• M5BURNER_QUICK_GUIDE.md (référence rapide)
• FIRMWARE_INFO.md (informations techniques)

🔗 SUPPORT:

GitHub: https://github.com/FRA-222/OpenSailingRC-Display/issues

════════════════════════════════════════════════════════════════
EOF

echo "📄 Instructions M5Burner créées: M5BURNER_INSTRUCTIONS.txt"
echo ""
echo "🎉 Prêt pour distribution!"
