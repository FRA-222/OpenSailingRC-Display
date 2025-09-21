# Releases - OpenSailingRC-Display

Ce répertoire contient les binaires compilés et prêts à flasher pour différentes versions d'OpenSailingRC-Display.

## 📦 Structure des releases

Chaque version suit la structure suivante :
```
releases/
├── v1.0.0/
│   ├── firmware.bin          # Firmware principal
│   ├── bootloader.bin        # Bootloader ESP32
│   ├── partitions.bin        # Table des partitions
│   ├── firmware.elf          # Symboles de debug (optionnel)
│   ├── checksums.sha256      # Checksums de vérification
│   ├── release_info.txt      # Informations de build
│   ├── FLASH_INSTRUCTIONS.md # Instructions de flash
│   └── RELEASE_NOTES.md      # Notes de release
├── OpenSailingRC-Display-v1.0.0.zip # Archive complète
└── README.md                 # Ce fichier
```

## 🚀 Utilisation rapide

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

| Version | Date | Taille | Description |
|---------|------|--------|-------------|
| [v1.0.0](v1.0.0/) | 2025-09-21 | ~1.2MB | Première release stable |

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