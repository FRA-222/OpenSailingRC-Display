# Releases - OpenSailingRC-Display

Ce répertoire## 🚀 Utilisation rapide

### 🔥 **M5Burner (Recommandé - Interface Graphique)**
1. **Guide complet** : [`M5BURNER_INSTALLATION_GUIDE.md`](M5BURNER_INSTALLATION_GUIDE.md) 
2. **Guide express** : [`M5BURNER_QUICK_GUIDE.md`](M5BURNER_QUICK_GUIDE.md)
3. **En 2 minutes** : M5Burner → Custom Firmware → `.bin` → BURN ✅

⚠️ **En cas d'erreur "invalid header: 0x2068746f"** après M5Burner :
```bash
# Solution automatique
./fix_bootloader_error.sh
```

### ⚡ **Ligne de commande (Avancé)**
1. **Télécharger** les binaires de la dernière version
2. **Suivre** les instructions dans `FLASH_INSTRUCTIONS.md`
3. **Utiliser esptool** ou PlatformIO pour flasher

### Commande flash rapide :
```bash
# Flash complet avec esptool
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 \
  --before default_reset --after hard_reset write_flash -z \
  --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin
```aires compilés et prêts à flasher pour différentes versions d'OpenSailingRC-Display.

## 📦 Structure des releases

Chaque version suit la structure suivante :
```
releases/
├── v1.0.0/                   # Release initiale
│   ├── firmware.bin          # Firmware principal
│   ├── bootloader.bin        # Bootloader ESP32
│   ├── partitions.bin        # Table des partitions
│   ├── checksums.sha256      # Checksums de vérification
│   ├── FLASH_INSTRUCTIONS.md # Instructions de flash
│   └── RELEASE_NOTES.md      # Notes de release
├── v1.0.1/                   # Release stable améliorée
│   ├── OpenSailingRC_Display_v1.0.1.bin # Firmware principal
│   ├── bootloader.bin        # Bootloader ESP32
│   ├── partitions.bin        # Table des partitions
│   ├── FIRMWARE_INFO.md      # Instructions complètes
│   └── RELEASE_NOTES_V1.0.1.md # Notes détaillées
├── OpenSailingRC-Display-v1.0.0.zip # Archive v1.0.0
├── OpenSailingRC-Display-v1.0.1.zip # Archive v1.0.1
└── README.md                 # Ce fichier
```

## � Versions disponibles

### 🔥 **v1.0.1** (Recommandée - Stable)
- **Date** : 28 septembre 2025
- **Taille** : 1,0 MB (669 KB compressé)
- **Améliorations** :
  - ✅ Gestion robuste des erreurs SD (pas de plantage)
  - ✅ Système de boutons tactiles amélioré (debouncing)
  - ✅ Interface non-bloquante pour serveur WiFi
  - ✅ Compatibilité étendue (legacy + actuel)
  - ✅ WiFi fallback intégré
- **Installation** : Voir `v1.0.1/FIRMWARE_INFO.md`

### 📦 **v1.0.0** (Legacy)
- **Date** : 21 septembre 2025  
- **Taille** : 1,0 MB (659 KB compressé)
- **Caractéristiques** :
  - Version initiale stable
  - Fonctionnalités de base complètes
  - Checksums SHA256 inclus
- **Installation** : Voir `v1.0.0/FLASH_INSTRUCTIONS.md`

## �🚀 Utilisation rapide

### Pour flasher la dernière version :
1. **Télécharger** les binaires de la dernière version
2. **Suivre** les instructions dans `FLASH_INSTRUCTIONS.md`
3. **Utiliser esptool** ou PlatformIO pour flasher

### Commande flash rapide :
```bash
# Flash complet avec esptool
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 \
  --before default_reset --after hard_reset write_flash -z \
  --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin
```

## 🛠️ Création d'une nouvelle release

### Méthode automatique (recommandée) :
```bash
./build_release.sh v1.1.0 "Description de la release"
```

### Méthode manuelle :
```bash
# 1. Compiler le projet
platformio run

# 2. Créer le répertoire
mkdir -p releases/v1.1.0

# 3. Copier les binaires
cp .pio/build/m5stack-core2/*.bin releases/v1.1.0/

# 4. Générer les checksums
cd releases/v1.1.0
sha256sum *.bin > checksums.sha256
```

## 📋 Vérification d'intégrité

Avant de flasher, vérifiez toujours l'intégrité des fichiers :
```bash
# Vérifier les checksums
sha256sum -c checksums.sha256
```

## 🏷️ Versions disponibles

| Version | Date | Taille | Description | Installation |
|---------|------|--------|-------------|--------------|
| [**v1.0.1**](v1.0.1/) | 2025-09-28 | ~1.0MB | **Release stable améliorée** | [**M5Burner Guide**](M5BURNER_INSTALLATION_GUIDE.md) |
| [v1.0.0](v1.0.0/) | 2025-09-21 | ~1.0MB | Première release stable | [Guide ESPTool](v1.0.0/FLASH_INSTRUCTIONS.md) |

## 🔗 Liens utiles

- **Documentation** : [README principal](../README.md)
- **Code source** : [Repository GitHub](https://github.com/FRA-222/OpenSailingRC-Display)
- **Issues** : [GitHub Issues](https://github.com/FRA-222/OpenSailingRC-Display/issues)
- **Releases GitHub** : [GitHub Releases](https://github.com/FRA-222/OpenSailingRC-Display/releases)

## ⚠️ Important

- **Ne jamais committer** les binaires dans le repository principal
- **Toujours tester** les binaires avant de créer une release GitHub
- **Vérifier les checksums** avant distribution
- **Maintenir** les notes de release à jour

---

**Maintenu par :** Philippe Hubert  
**Licence :** GPL v3.0