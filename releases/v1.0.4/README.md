# Installation OpenSailingRC-Display v1.0.4

## 📦 Contenu du package

- `OpenSailingRC_Display_v1.0.4_MERGED.bin` - Firmware combiné (1.1 MB)
- `SHA256SUMS.txt` - Checksums de vérification
- `RELEASE_NOTES_V1.0.4.md` - Notes de version complètes

## 🚀 Installation rapide avec M5Burner

### Prérequis
- M5Burner installé ([Télécharger ici](https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/software/M5Burner.zip))
- M5Stack Core2
- Câble USB-C

### Étapes d'installation

#### 1. Connecter le Core2
- Brancher le Core2 via USB-C
- Vérifier que le port USB apparaît

#### 2. Lancer M5Burner
- Ouvrir M5Burner
- Sélectionner le port USB dans la liste déroulante

#### 3. Charger le firmware
- Cliquer sur **"Custom"** ou **"Load firmware"**
- Sélectionner `OpenSailingRC_Display_v1.0.4_MERGED.bin`
- **Adresse de flash : `0x0000`** (important !)

#### 4. Flasher
- Cliquer sur **"Burn"** ou **"Start"**
- Attendre la fin du processus (environ 45 secondes)
- Message de confirmation : "Burn Successfully"

#### 5. Vérifier
- Le Core2 redémarre automatiquement
- Écran de démarrage : "OpenSailingRC Display V1.0.4"
- Indicateur batterie visible en haut au centre
- Pictogramme satellite en haut à droite

## 🔧 Installation avec PlatformIO

### Prérequis
- Visual Studio Code + PlatformIO
- Projet OpenSailingRC-Display cloné

### Étapes

```bash
# 1. Naviguer dans le projet
cd OpenSailingRC-Display

# 2. Compiler
platformio run --environment m5stack-core2

# 3. Uploader
platformio run --target upload --environment m5stack-core2

# 4. Moniteur série (optionnel)
platformio device monitor --baud 115200
```

## 🧪 Test et validation

### 1. Vérifier l'écran principal

**En haut de l'écran :**
- 🔋 **Centre** : Indicateur batterie avec pictogramme et pourcentage (couleur selon niveau)
- 🛰️ **Droite** : Pictogramme satellite + nombre

**Zone principale :**
- **BOAT** : Vitesse (KTS) et cap (DEG)
- **WIND** : Vitesse (KTS) et direction (DEG)
- Affiche "---" en l'absence de données

**En bas :**
- 🔵 Bouton gauche : **RECORD** (bleu marine si inactif)
- 🔴 Bouton gauche : **STOP** (rouge si enregistrement actif)
- 🔵 Bouton droit : **WIFI** (bleu marine si serveur inactif)
- 🔴 Bouton droit : **STOP** (rouge si serveur actif)

### 2. Tester la batterie

**Sans USB-C branché :**
- Vérifier l'affichage du pourcentage
- Vérifier la couleur selon le niveau :
  - 🟢 Vert si >50%
  - 🟠 Orange si 20-50%
  - 🔴 Rouge si <20%

**Avec USB-C branché :**
- Un **éclair jaune** doit apparaître dans le pictogramme batterie
- Indique que la charge est en cours

### 3. Tester la réception GPS

- Allumer un BoatGPS (v1.0.3 ou v1.0.4)
- Attendre le fix GPS (LED verte sur AtomS3)
- Vérifier l'affichage sur le Display :
  - Vitesse bateau (BOAT)
  - Cap bateau (DEG)
  - Nombre de satellites

**Avec BoatGPS v1.0.4 :**
- Le nom personnalisé du bateau s'affiche (ex: "BOAT1")

**Avec BoatGPS v1.0.3 :**
- L'adresse MAC s'affiche (ex: "AA:BB:CC:DD:EE:FF")

### 4. Tester l'enregistrement

- Toucher le bouton **RECORD** (gauche)
- Le bouton devient rouge avec label **STOP**
- Vérifier la création de fichiers sur la carte SD :
  - `/GPS_YYYY-MM-DD_HH-MM-SS_NNN.json` (données GPS)

- Toucher à nouveau pour arrêter (bouton **STOP**)
- Le bouton redevient bleu marine avec label **RECORD**

### 5. Tester le serveur de fichiers

- Toucher le bouton **WIFI** (droit)
- Le bouton devient rouge avec label **STOP**
- Message affiché au centre : **SERVEUR ACTIF** (fond rouge)
- URL affichée : `http://192.168.4.1` (en noir)

- Se connecter au WiFi :
  - SSID : `M5Stack-Display`
  - Pas de mot de passe

- Accéder à l'URL dans un navigateur :
  - Liste des fichiers GPS disponibles
  - Téléchargement possible

- Toucher à nouveau pour arrêter (bouton **STOP**)
- Message : **SERVEUR ARRETE** (fond bleu marine)
- L'écran se rafraîchit automatiquement après 3 secondes

## 🔍 Dépannage

### Indicateur batterie affiche toujours 100%

**Cause :** USB-C branché en permanence

**Solution :**
- Débrancher l'USB-C
- Le pourcentage devrait se mettre à jour
- La batterie doit être calibrée (2-3 cycles charge/décharge)

### Pictogramme satellite illisible

**Cause :** Contraste insuffisant

**Solution :**
- Vérifier l'éclairage ambiant
- Le picto a un corps blanc + panneaux bleus
- Si toujours illisible, signaler un bug (problème d'affichage)

### Boutons ne répondent pas

**Cause :** Zone tactile non détectée

**Solution :**
- Toucher au **centre** du bouton
- Vérifier que l'écran tactile est calibré
- Redémarrer le Core2 si nécessaire

### Message serveur ne s'efface pas

**Cause :** Bug corrigé en v1.0.4

**Solution :**
- Vérifier que vous utilisez bien v1.0.4
- Le message devrait s'effacer après 3 secondes
- L'écran devrait se rafraîchir complètement

### Pas de données GPS affichées

**Cause :** BoatGPS non allumé ou hors de portée

**Solution :**
1. Vérifier que le BoatGPS est allumé (LED visible)
2. Vérifier le fix GPS (LED verte sur AtomS3)
3. Réduire la distance (<50m pour les tests)
4. Vérifier compatibilité versions (BoatGPS v1.0.3+ requis)

### Checksum incorrect

**Cause :** Fichier .bin corrompu lors du téléchargement

**Solution :**
1. Vérifier le SHA256 :
   ```bash
   shasum -a 256 OpenSailingRC_Display_v1.0.4_MERGED.bin
   ```
2. Comparer avec `SHA256SUMS.txt`
3. Re-télécharger si différent

## 📊 Spécifications techniques

### Matériel supporté
- M5Stack Core2
- Écran : 320x240 LCD tactile
- Batterie : 390mAh LiPo 3.7V
- Connectivité : WiFi 2.4GHz, ESP-NOW

### Communication
- **Protocole :** ESP-NOW broadcast
- **Fréquence réception :** 1 Hz
- **Portée :** 100-200m en ligne de vue
- **Taille paquet :** 52 bytes (optimisé avec BoatGPS v1.0.4)

### Batterie
- **Capacité :** 390mAh
- **Autonomie normale :** 2-3 heures
- **Autonomie économie :** 4-5 heures
- **Charge :** USB-C 5V/1A
- **Seuils alerte :** 50% (vert), 20% (orange), <20% (rouge)

### Stockage SD
- **Format :** FAT32
- **Capacité :** 4-32 GB recommandé
- **Format JSON :** Une ligne par échantillon
- **Rotation :** Nouveau fichier toutes les 1000 entrées ou 1 MB

### Performances
- **Temps de démarrage :** <5 secondes
- **Latence affichage :** <100ms
- **Timeout données :** 5 secondes (affiche "---")

## 🆕 Nouveautés v1.0.4

### Interface utilisateur
- ✅ Indicateur batterie centré avec pictogramme
- ✅ Pictogramme satellite (panneaux solaires bleus)
- ✅ Boutons bleu marine/rouge (plus clairs)
- ✅ Labels boutons améliorés (RECORD/STOP)
- ✅ Message serveur avec fond rouge/bleu marine

### Performance
- ✅ Réception paquets optimisés (-8 bytes, +13% efficacité)
- ✅ Timestamps gérés localement
- ✅ Support noms personnalisés bateau
- ✅ Rafraîchissement écran corrigé

### Compatibilité
- ✅ BoatGPS v1.0.3 : Compatible (MAC address)
- ✅ BoatGPS v1.0.4 : Optimisé (noms personnalisés)
- ✅ Anemometer v1.0.3 : Compatible

## 📚 Documentation complète

- [RELEASE_NOTES_V1.0.4.md](RELEASE_NOTES_V1.0.4.md) - Notes de version
- [Display.h](../../include/Display.h) - API Display
- [DisplayTypes.h](../../include/DisplayTypes.h) - Structures de données

## 🆘 Support

- **Issues GitHub :** [github.com/FRA-222/Boat-GPS-Display/issues](https://github.com/FRA-222/Boat-GPS-Display/issues)
- **Documentation :** [github.com/FRA-222/Boat-GPS-Display/wiki](https://github.com/FRA-222/Boat-GPS-Display/wiki)

---

**Version du guide :** 1.0.4  
**Dernière mise à jour :** 25 novembre 2025  
**Recommandation :** Utiliser avec BoatGPS v1.0.4 pour bénéficier de toutes les optimisations
