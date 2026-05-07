# OpenSailingRC-Display

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Hardware: M5Stack Core2](https://img.shields.io/badge/Hardware-M5Stack%20Core2-orange.svg)](https://shop.m5stack.com/products/m5stack-core2-esp32-iot-development-kit)
[![Version](https://img.shields.io/badge/Version-1.0.7-brightgreen.svg)](https://github.com/FRA-222/Boat-GPS-Display/releases)

Un système d'affichage GPS et anémométrique pour télécommande de voilier RC basé sur un M5Stack Core2. Ce projet fait partie de l'écosystème [OpenSailingRC](https://github.com/FRA-222) et affiche en temps réel les données de navigation (bateau et vent) via ESP-NOW.

> 🎉 **Version 1.0.7** - Nouvelle interface 4 lignes avec système de timeout intelligent

## 🎯 Caractéristiques

### 📊 Affichage 4 lignes (v1.0.3)
Interface compacte et lisible affichant toutes les données essentielles :
- **Ligne 1 - BOAT Speed** : Vitesse du bateau en nœuds
- **Ligne 2 - BOAT Heading** : Cap du bateau en degrés (0-360°)
- **Ligne 3 - WIND Speed** : Vitesse du vent en nœuds
- **Ligne 4 - WIND Direction** : Direction du vent en degrés (prévu pour bouées GPS)

### ⏱️ Système de timeout intelligent
- **Détection automatique** : Affiche `--` après 5 secondes sans données
- **Indication visuelle** : Identifie immédiatement les pertes de connexion
- **Indépendant** : Timeout séparé pour bateau et vent
- **Pas de synchronisation** : Utilise l'horloge locale du Display uniquement

### 🌐 Connectivité
- **ESP-NOW** : Réception des données en temps réel (100-200m de portée)
  - Données bateau (GPS) : vitesse, cap, timestamp
  - Données vent (anémomètre) : vitesse, timestamp
- **Multi-sources** : Supporte plusieurs bateaux et anémomètres simultanément
- **Broadcast** : Communication point-à-multipoint sans appairage

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

### Option 1 : M5Burner (Recommandé) 🌟

**La méthode la plus simple pour flasher votre M5Stack !**

1. **Télécharger M5Burner**
   - Windows/Mac/Linux : [M5Burner Download](https://docs.m5stack.com/en/download)

2. **Télécharger le firmware**
   - Aller dans [Releases](https://github.com/FRA-222/Boat-GPS-Display/releases)
   - Télécharger `OpenSailingRC_Display_v1.0.7_MERGED.bin`

3. **Flasher le M5Stack Core2**
   - Ouvrir M5Burner
   - Connecter le M5Stack Core2 en USB-C
   - Sélectionner le port COM
   - Choisir le fichier `.bin` téléchargé
   - Adresse : `0x0` (important !)
   - Cliquer sur "Burn"

4. **Vérifier**
   - Redémarrer le M5Stack
   - L'interface 4 lignes devrait s'afficher
   - Vérifier que `--` apparaît (normal sans signal)

✅ **Aucune compilation nécessaire !**

### Option 2 : PlatformIO (Pour développeurs)

#### Prérequis
1. [PlatformIO](https://platformio.org/) installé
2. [VS Code](https://code.visualstudio.com/) avec extension PlatformIO
3. Pilotes USB-C pour M5Stack Core2

#### Étapes d'installation

1. **Cloner le projet**
   ```bash
   git clone https://github.com/FRA-222/Boat-GPS-Display.git
   cd Boat-GPS-Display/OpenSailingRC-Display
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
2. L'interface 4 lignes s'affiche automatiquement
3. Par défaut, toutes les valeurs affichent `--` (pas de données)
4. Les données apparaissent dès réception des signaux ESP-NOW

### Interface utilisateur (v1.0.3)

#### Écran principal 4 lignes
```
┌─────────────────────────────┐
│ BOAT   12.5 KTS            │  ← Vitesse bateau
│ BOAT   285 DEG             │  ← Cap bateau
│ WIND   8.3 KTS             │  ← Vitesse vent
│ WIND   -- DEG              │  ← Direction vent*
└─────────────────────────────┘
```
*Direction vent préparée pour future implémentation avec bouées GPS

#### Indicateurs de timeout
- **Valeur numérique** : Donnée valide reçue < 5 secondes
- **`--`** : Aucune donnée depuis > 5 secondes
- **Indépendant** : Bateau et vent ont des timeouts séparés

### Modes de fonctionnement

#### Mode ESP-NOW (par défaut)
- **Réception bateau** : Vitesse, cap depuis module GPS embarqué
- **Réception vent** : Vitesse depuis anémomètre AtomS3
- **Temps réel** : Rafraîchissement automatique à chaque réception
- **Portée** : 100-200 mètres en ligne de vue

#### Compatibilité
- ⚠️ **Version 1.0.7 requise sur tous les appareils**
- Incompatible avec versions antérieures (structure modifiée)
- Nécessite :
  - OpenSailingRC-Display v1.0.7
  - OpenSailingRC-Anemometer v1.0.7
  - OpenSailingRC-BoatGPS v1.0.6+

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

### Structures ESP-NOW (v1.0.7)

#### struct_message_Boat
```cpp
struct struct_message_Boat {
    float speedKnots;      // Vitesse en nœuds
    float heading;         // Cap en degrés (0-360)
    unsigned long timestamp; // Timestamp (rempli par Display)
};
```

#### struct_message_Anemometer
```cpp
struct struct_message_Anemometer {
    float windSpeed;       // Vitesse du vent en nœuds
    unsigned long timestamp; // Timestamp (rempli par Display)
};
```

> 💡 **Note** : Le timestamp est initialisé à 0 par l'émetteur et rempli par le Display lors de la réception avec `millis()`. Cela permet le système de timeout sans synchronisation entre appareils.

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

## 📦 Releases

### Version 1.0.7 (Actuelle)
- ✅ Interface 4 lignes (BOAT + WIND)
- ✅ Système de timeout 5 secondes
- ✅ Terminologie WIND au lieu de BUOY
- ✅ Structures avec timestamp
- ✅ Merged bin pour M5Burner

📥 [Télécharger v1.0.7](https://github.com/FRA-222/Boat-GPS-Display/releases/tag/v1.0.7)

### Version 1.0.3
- ✅ Interface 4 lignes initiale
- ✅ Système de timeout 5 secondes

📥 [Télécharger v1.0.3](https://github.com/FRA-222/Boat-GPS-Display/releases/tag/v1.0.3)

### Changelog complet
Voir [RELEASE_NOTES_V1.0.3.md](releases/v1.0.3/RELEASE_NOTES_V1.0.3.md) pour tous les détails.

## 🔄 Projets liés

### Écosystème OpenSailingRC
- [OpenSailingRC-Anemometer-v2](https://github.com/FRA-222/OpenSailingRC-Anemometer-v2) - Anémomètre M5Stack AtomS3
- [OpenSailingRC-BoatGPS](https://github.com/FRA-222/OpenSailingRC-BoatGPS) - Module GPS embarqué M5Stack AtomS3
- [OpenSailingRC-Display](https://github.com/FRA-222/Boat-GPS-Display) - Display M5Stack Core2 (ce projet)

### Installation complète
Pour un système complet, flasher les 3 firmwares v1.0.3 :
1. **Display** (M5Stack Core2) - Écran 4 lignes
2. **Anemometer** (M5Stack AtomS3) - Mesure vitesse vent
3. **BoatGPS** (M5Stack AtomS3) - Mesure vitesse/cap bateau

---

**Développé avec ❤️ pour la communauté RC Sailing**