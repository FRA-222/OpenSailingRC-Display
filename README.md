# OpenSailingRC-Display

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Hardware: M5Stack Core2](https://img.shields.io/badge/Hardware-M5Stack%20Core2-orange.svg)](https://shop.m5stack.com/products/m5stack-core2-esp32-iot-development-kit)

Un système d'affichage GPS pour télécommande de voilier RC basé sur un M5Stack Core2. Ce projet fait partie de l'écosystème [OpenSailingRC](https://github.com/FRA-222) et affiche en temps réel les données GPS d'un voilier RC.

## 🎯 Caractéristiques

### 📊 Affichage des données GPS
- **Vitesse** : Affichage en nœuds avec barre colorée progressive
  - Vert : < 2 nœuds (navigation lente)
  - Orange : 2-4 nœuds (navigation normale)
  - Rouge : > 4 nœuds (navigation rapide)
- **Cap** : Rose des vents avec aiguille mobile indiquant la direction
- **Satellites** : Nombre de satellites GPS visibles
- **Position** : Latitude et longitude (optionnel)

### 🌐 Connectivité
- **ESP-NOW** : Réception des données GPS en temps réel depuis le voilier
- **WiFi** : Mode serveur de fichiers pour accès aux données stockées
- **Commutation automatique** : Basculement intelligent entre ESP-NOW et WiFi

### 💾 Stockage des données
- **Carte SD** : Enregistrement automatique des données de navigation
- **Format JSON** : Données structurées et facilement exploitables
- **Serveur web** : Accès distant aux fichiers via interface web

## 🛠️ Matériel requis

### Composants principaux
- [M5Stack Core2 v1.1](https://shop.m5stack.com/products/m5stack-core2-esp32-iot-development-kit) - Écran tactile ESP32
- Carte microSD (recommandée : 8GB+)
- Câble USB-C pour programmation et alimentation

### Spécifications techniques
- **Processeur** : ESP32 dual-core 240MHz
- **Mémoire** : 16MB Flash, 8MB PSRAM
- **Écran** : 320x240 pixels, tactile capacitif
- **Connectivité** : WiFi 802.11 b/g/n, Bluetooth 4.2, ESP-NOW
- **Alimentation** : Batterie intégrée 390mAh + USB-C

## 🚀 Installation

### Prérequis
1. [PlatformIO](https://platformio.org/) installé
2. [VS Code](https://code.visualstudio.com/) avec extension PlatformIO
3. Pilotes USB-C pour M5Stack Core2

### Étapes d'installation

1. **Cloner le projet**
   ```bash
   git clone https://github.com/FRA-222/OpenSailingRC-Display.git
   cd OpenSailingRC-Display
   ```

2. **Compiler le projet**
   ```bash
   platformio run
   ```

3. **Téléverser sur le M5Stack**
   ```bash
   platformio upload
   ```

4. **Moniteur série** (optionnel)
   ```bash
   platformio device monitor
   ```

### Configuration initiale

1. **Carte SD** : Insérer une carte microSD formatée en FAT32
2. **WiFi** : Créer le fichier `wifi_config.json` sur la carte SD :
   ```json
   {
     "ssid": "VotreSSID",
     "password": "VotreMotDePasse"
   }
   ```

## 📖 Utilisation

### Démarrage
1. Allumer le M5Stack Core2
2. L'écran affiche l'interface de navigation
3. Les données GPS s'affichent automatiquement lors de la réception

### Interface utilisateur
- **Écran principal** : Affichage des données de navigation
- **Rose des vents** : Orientation visuelle du cap
- **Barre de vitesse** : Indicateur coloré sur le côté droit
- **Informations textuelles** : Vitesse, cap et satellites

### Modes de fonctionnement

#### Mode ESP-NOW (par défaut)
- Réception des données GPS depuis le voilier RC
- Affichage temps réel des informations de navigation
- Enregistrement automatique sur carte SD

#### Mode WiFi (serveur de fichiers)
- Activation via interface tactile
- Serveur web accessible à l'adresse IP affichée
- Téléchargement des fichiers de données enregistrés

## 🏗️ Architecture du projet

### Structure des fichiers
```
OpenSailingRC-Display/
├── src/
│   ├── main.cpp              # Programme principal
│   ├── Display.cpp           # Gestion de l'affichage
│   ├── Storage.cpp           # Stockage sur carte SD
│   ├── FileServerManager.cpp # Serveur de fichiers WiFi
│   └── Logger.cpp           # Système de logging
├── include/
│   ├── Display.h
│   ├── Storage.h
│   ├── FileServerManager.h
│   ├── Logger.h
│   └── DisplayTypes.h
├── docs/                    # Documentation technique
├── platformio.ini          # Configuration PlatformIO
└── README.md
```

### Classes principales

#### `Display`
Gestion de l'interface utilisateur et de l'affichage des données GPS.
- Rose des vents interactive
- Barres de progression colorées
- Interface tactile

#### `Storage`
Système de stockage des données sur carte SD.
- Écriture en format JSON
- Gestion des erreurs SD
- Organisation des fichiers par date

#### `FileServerManager`
Serveur web pour l'accès distant aux données.
- Interface HTTP simple
- Liste et téléchargement de fichiers
- Gestion de la connexion WiFi

#### `Logger`
Système de logging unifié pour le debugging.
- Messages horodatés
- Niveaux de priorité
- Sortie série et fichier

## 📊 Format des données

### Structure ESP-NOW
```cpp
struct GPSData {
    double latitude;    // Latitude GPS (degrés)
    double longitude;   // Longitude GPS (degrés)
    float speed;        // Vitesse (m/s)
    float heading;      // Cap (degrés, 0=Nord)
    int satellites;     // Nombre de satellites
};
```

### Format JSON (carte SD)
```json
{
  "timestamp": "2025-09-21T14:30:00Z",
  "latitude": 43.123456,
  "longitude": 2.654321,
  "speed_ms": 2.5,
  "speed_knots": 4.86,
  "heading": 285.0,
  "satellites": 8
}
```

## 🔧 Configuration avancée

### Paramètres d'affichage
- **Vitesse max** : Configurable dans `DisplayTypes.h`
- **Couleurs** : Personnalisables par vitesse
- **Unités** : Support nœuds/km/h

### Communication ESP-NOW
- **Canal WiFi** : Auto-détection
- **Adresses MAC** : Configuration automatique
- **Retry logic** : Gestion des pertes de connexion

### Stockage
- **Intervalle** : Fréquence d'enregistrement configurable
- **Compression** : Optimisation de l'espace disque
- **Rotation** : Gestion automatique des anciens fichiers

## 🧪 Tests et validation

### Tests unitaires
```bash
platformio test
```

### Tests d'intégration
1. Test de réception ESP-NOW
2. Validation stockage SD
3. Vérification serveur web

### Debugging
- **Logs série** : `monitor_speed = 115200`
- **Logs SD** : Fichiers de debug sur carte
- **Interface web** : Diagnostic système

## 🤝 Contribution

Les contributions sont les bienvenues ! Ce projet fait partie de l'écosystème OpenSailingRC.

### Comment contribuer
1. Fork du projet
2. Créer une branche feature (`git checkout -b feature/AmazingFeature`)
3. Commit des changements (`git commit -m 'Add AmazingFeature'`)
4. Push vers la branche (`git push origin feature/AmazingFeature`)
5. Ouvrir une Pull Request

### Guidelines
- Code documenté en anglais
- Tests unitaires pour les nouvelles fonctionnalités
- Respect des conventions de nommage
- Documentation mise à jour

## 📝 License

Ce projet est sous licence GNU General Public License v3.0. Voir le fichier [LICENSE](LICENSE) pour plus de détails.

## 🙏 Remerciements

- **M5Stack** pour le hardware Core2
- **Espressif** pour l'ESP32 et ESP-NOW
- **Communauté PlatformIO** pour l'écosystème de développement
- **OpenSailingRC** pour l'inspiration et les spécifications

## 📞 Support

- **Issues GitHub** : [Signaler un bug](https://github.com/FRA-222/OpenSailingRC-Display/issues)
- **Discussions** : [Forum de discussion](https://github.com/FRA-222/OpenSailingRC-Display/discussions)
- **Documentation** : [Wiki du projet](https://github.com/FRA-222/OpenSailingRC-Display/wiki)

## 🔄 Projets liés

- [OpenSailingRC-Anemometer-v2](https://github.com/FRA-222/OpenSailingRC-Anemometer-v2) - Anémomètre pour voilier RC
- [OpenSailingRC-GPS](https://github.com/FRA-222/OpenSailingRC-GPS) - Module GPS pour voilier RC
- [OpenSailingRC-Base](https://github.com/FRA-222/OpenSailingRC-Base) - Système de base pour télécommande

---

**Développé avec ❤️ pour la communauté RC Sailing**