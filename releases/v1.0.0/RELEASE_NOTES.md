# Release Notes - OpenSailingRC-Display v1.0.0

**Date de release :** 21 septembre 2025  
**Auteur :** Philippe Hubert  
**Licence :** GPL v3.0

## 🎉 Première release officielle !

Cette première version stable d'OpenSailingRC-Display offre un système complet d'affichage GPS pour télécommande de voilier RC basé sur M5Stack Core2.

## ✨ Nouvelles fonctionnalités

### 📊 Interface d'affichage GPS
- **Rose des vents interactive** avec aiguille mobile indiquant le cap
- **Barre de vitesse colorée** (vert/orange/rouge selon la vitesse)
- **Affichage temps réel** : vitesse en nœuds, cap en degrés, nombre de satellites
- **Interface tactile** responsive et intuitive

### 🌐 Connectivité sans fil
- **ESP-NOW** : Réception des données GPS depuis le voilier RC
- **WiFi** : Mode serveur de fichiers pour accès aux données
- **Commutation automatique** entre modes ESP-NOW et WiFi
- **Gestion intelligente** des connexions et reconnexions

### 💾 Stockage des données
- **Enregistrement automatique** sur carte microSD
- **Format JSON structuré** pour exploitation facile des données
- **Serveur web intégré** pour téléchargement des fichiers
- **Gestion robuste** des erreurs de carte SD

### 🛠️ Architecture modulaire
- **Classes bien définies** : Display, Storage, FileServerManager, Logger
- **Documentation complète** avec headers Doxygen
- **Code maintenable** et extensible
- **Logging unifié** pour debugging

## 🔧 Améliorations techniques

### Performance
- **Affichage fluide** à 60 FPS
- **Gestion mémoire optimisée** pour ESP32
- **Communication ESP-NOW** stable et rapide
- **Interface web** responsive

### Robustesse
- **Gestion d'erreurs complète** pour tous les modules
- **Recovery automatique** en cas de perte de connexion
- **Validation des données** GPS reçues
- **Protection contre les corruptions** de carte SD

### Compatibilité
- **M5Stack Core2 v1.1+** support complet
- **ESP32** dual-core optimisé
- **Cartes SD** FAT32 jusqu'à 32GB
- **Protocole ESP-NOW** compatible avec l'écosystème OpenSailingRC

## 📋 Contenu de la release

### Binaires
- `firmware.bin` (1.2 MB) - Firmware principal
- `bootloader.bin` (24 KB) - Bootloader ESP32
- `partitions.bin` (3 KB) - Table des partitions

### Documentation
- `FLASH_INSTRUCTIONS.md` - Guide de flash détaillé
- `RELEASE_NOTES.md` - Ces notes de release
- Lien vers documentation complète sur GitHub

## 🎯 Configuration requise

### Hardware
- **M5Stack Core2 v1.1** ou supérieur
- **Carte microSD** (recommandée, FAT32)
- **Câble USB-C** pour programmation/alimentation

### Logiciels
- **esptool** v4.0+ (pour flash manuel)
- **PlatformIO** v6.0+ (pour développement)
- **Navigateur web** moderne (pour interface serveur)

## 🚀 Utilisation rapide

1. **Flash** le firmware selon `FLASH_INSTRUCTIONS.md`
2. **Insérer** une carte microSD formatée (optionnel)
3. **Configurer** le WiFi via fichier `wifi_config.json`
4. **Allumer** le M5Stack Core2
5. **Profiter** de l'affichage GPS en temps réel !

## 🔄 Écosystème OpenSailingRC

Cette release s'intègre parfaitement avec :
- **OpenSailingRC-GPS** - Module GPS embarqué sur le voilier
- **OpenSailingRC-Anemometer-v2** - Capteur de vent
- **OpenSailingRC-Base** - Station de base télécommande

## 🐛 Problèmes connus

### Limitations mineures
- **Animation de l'aiguille** : Déplacement instantané (sera amélioré en v1.1)
- **Historique limité** : Pas de graphiques temporels (prévu en v1.2)
- **Configuration WiFi** : Nécessite fichier sur SD (interface web prévue)

### Workarounds
- **Rose des vents** : Fonction correctement malgré animation instantanée
- **Données historiques** : Accessibles via serveur web
- **WiFi** : Configuration simple via fichier JSON

## 🔜 Roadmap v1.1

- Animation fluide de l'aiguille de cap
- Interface de configuration WiFi tactile
- Graphiques de vitesse temporels
- Export données en CSV
- Mode démonstration

## 🙏 Remerciements

- **Communauté M5Stack** pour le support hardware
- **Espressif** pour ESP32 et ESP-NOW
- **Testeurs bêta** pour leurs retours précieux
- **Communauté OpenSailingRC** pour l'inspiration

## 📞 Support

- **Documentation** : [GitHub Wiki](https://github.com/FRA-222/OpenSailingRC-Display/wiki)
- **Issues** : [GitHub Issues](https://github.com/FRA-222/OpenSailingRC-Display/issues)
- **Discussions** : [GitHub Discussions](https://github.com/FRA-222/OpenSailingRC-Display/discussions)

---

**Téléchargement :** [Release v1.0.0](https://github.com/FRA-222/OpenSailingRC-Display/releases/tag/v1.0.0)  
**Code source :** [GitHub Repository](https://github.com/FRA-222/OpenSailingRC-Display)  
**Licence :** [GPL v3.0](https://github.com/FRA-222/OpenSailingRC-Display/blob/main/LICENSE.md)