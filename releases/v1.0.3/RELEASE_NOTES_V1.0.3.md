# OpenSailingRC v1.0.3 - Release Notes

**Date de sortie :** 23 novembre 2025  
**Version :** 1.0.3

## 🎯 Nouveautés principales

### 1. Affichage amélioré du Display (4 lignes)

#### Changement de terminologie
- **BUOY → WIND** : Meilleure représentation sémantique des données d'anémomètre

#### Nouvelle organisation de l'affichage
```
Ligne 1: BOAT - Vitesse (KTS)
Ligne 2: BOAT - Cap (DEG)
Ligne 3: WIND - Vitesse (KTS)
Ligne 4: WIND - Direction (DEG)  ← NOUVEAU
```

#### Police réduite pour lisibilité
- Taille de police réduite de 4 à 3
- Permet l'affichage de 4 lignes d'information
- Maintient une excellente lisibilité

### 2. Système de timeout intelligent (5 secondes)

#### Affichage "--" pour données obsolètes
Lorsqu'aucune donnée n'est reçue pendant 5 secondes :
- Vitesse bateau : affiche `--` au lieu de la dernière valeur
- Cap bateau : affiche `--` au lieu de la dernière valeur
- Vitesse vent : affiche `--` au lieu de la dernière valeur
- Direction vent : affiche `--` (en attente des futures bouées GPS)

#### Fonctionnement du timeout
- Le Display marque l'heure de réception de chaque paquet
- Compare son temps actuel avec le temps de réception
- Pas de synchronisation entre appareils nécessaire
- Tout calculé sur l'horloge locale du Display

### 3. Structures de données mises à jour

#### Ajout du champ `timestamp`
Toutes les structures ont été enrichies d'un timestamp :

**struct_message_Boat**
```cpp
typedef struct {
    // ... champs existants ...
    unsigned long timestamp; // Timestamp de mesure (NOUVEAU)
} struct_message_Boat;
```

**struct_message_Anemometer**
```cpp
typedef struct {
    // ... champs existants ...
    unsigned long timestamp; // Timestamp de mesure (NOUVEAU)
} struct_message_Anemometer;
```

**GPSBroadcastPacket** (BoatGPS)
```cpp
struct GPSBroadcastPacket {
    // ... champs existants ...
    unsigned long timestamp; // Timestamp de mesure (NOUVEAU)
};
```

#### Suppression du code legacy
- `struct_message_Anemometer_Legacy` supprimée
- Code de compatibilité legacy nettoyé
- Meilleure maintenabilité

### 4. Préparation pour direction du vent des bouées

#### Architecture extensible
La fonction `drawDisplay()` accepte maintenant :
```cpp
void drawDisplay(
    const struct_message_Boat& boatData,
    const struct_message_Anemometer& anemometerData,
    bool isRecording,
    bool isServerActive = false,
    int boatCount = 0,
    float windDirection = 0,           // Direction du vent (bouées)
    unsigned long windDirTimestamp = 0 // Timestamp direction
);
```

#### Séparation des sources de données
- **Anémomètre** : Vitesse du vent uniquement
- **Bouées GPS** : Direction du vent (à venir)
- Architecture propre et modulaire

## 🔧 Améliorations techniques

### Display (M5Stack Core2)
- Optimisation de l'affichage avec gestion des timeouts
- Réduction de la taille de police pour 4 lignes
- Meilleure gestion de la mémoire d'affichage
- Taille binaire : 1,106,176 octets (16.9%)

### Anémomètre (M5Stack AtomS3)
- Ajout du timestamp dans les paquets ESP-NOW
- Puissance WiFi maximale maintenue (19.5 dBm)
- Taille binaire : 917,856 octets (27.5%)

### BoatGPS (M5Stack AtomS3)
- Ajout du timestamp dans GPSBroadcastPacket
- Compatibilité totale avec le nouveau Display
- Taille binaire : 951,216 octets (28.5%)

## 📊 Comparaison des versions

| Fonctionnalité | v1.0.2 | v1.0.3 |
|----------------|--------|--------|
| Lignes d'affichage | 3 | 4 |
| Timeout des données | ❌ | ✅ 5s |
| Direction du vent | ❌ | ⏳ Préparé |
| Affichage "--" | ❌ | ✅ |
| Code legacy | ✅ | ❌ Nettoyé |
| Timestamp | ❌ | ✅ |

## 🚀 Installation

### Via M5Burner (Recommandé)
1. Télécharger les 3 fichiers `.bin`
2. Ouvrir M5Burner
3. Sélectionner le bon appareil :
   - **Display** : M5Stack Core2 → `OpenSailingRC_Display_v1.0.3_MERGED.bin`
   - **Anémomètre** : M5Stack AtomS3 → `OpenSailingRC_Anemometer_v1.0.3_MERGED.bin`
   - **BoatGPS** : M5Stack AtomS3 → `OpenSailingRC_BoatGPS_v1.0.3_MERGED.bin`
4. Flasher à l'adresse `0x0`

### Via esptool.py (Avancé)
```bash
# Display (ESP32)
esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash 0x0 OpenSailingRC_Display_v1.0.3_MERGED.bin

# Anémomètre (ESP32-S3)
esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x0 OpenSailingRC_Anemometer_v1.0.3_MERGED.bin

# BoatGPS (ESP32-S3)
esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x0 OpenSailingRC_BoatGPS_v1.0.3_MERGED.bin
```

## ⚠️ Notes importantes

### Mise à jour obligatoire pour compatibilité
**ATTENTION :** Les 3 composants doivent être mis à jour ensemble :
- Le Display v1.0.3 rejette les paquets des anciennes versions (taille incorrecte)
- Les anciennes versions du Display ne fonctionneront pas avec les nouveaux firmwares
- Mettre à jour **tous les appareils** avant utilisation

### Migration depuis v1.0.2
1. Flasher le Display en premier
2. Flasher l'Anémomètre
3. Flasher le(s) BoatGPS
4. Redémarrer tous les appareils
5. Vérifier l'affichage des 4 lignes

### Comportement attendu
- Au démarrage : toutes les valeurs affichent `--` (pas de données)
- Après réception : valeurs numériques apparaissent
- Si perte de signal : retour à `--` après 5 secondes
- Direction du vent : affiche toujours `--` (en attente des bouées)

## 🔍 Vérification de l'installation

### Tests recommandés
1. **Test au démarrage** : Vérifier `--` sur toutes les valeurs
2. **Test réception normale** : Vérifier l'affichage des valeurs
3. **Test timeout bateau** : Éteindre le bateau → `--` après 5s
4. **Test timeout anémomètre** : Éteindre l'anémomètre → `--` après 5s
5. **Test reconnexion** : Rallumer les appareils → retour aux valeurs

### Indicateurs de bon fonctionnement
- ✅ 4 lignes visibles sur le Display
- ✅ Labels "WIND" au lieu de "BUOY"
- ✅ Affichage "--" si aucun appareil connecté
- ✅ Valeurs numériques si appareils actifs
- ✅ Pas d'erreurs dans les logs série

## 📝 Changelog détaillé

### Display v1.0.3
- ✨ Ajout 4ème ligne d'affichage (direction du vent)
- ✨ Système de timeout 5 secondes avec affichage "--"
- ✨ Changement BUOY → WIND
- 🔧 Police réduite (taille 4 → 3)
- 🔧 Ajout timestamp dans struct_message_Boat
- 🔧 Ajout timestamp dans struct_message_Anemometer
- 🗑️ Suppression struct_message_Anemometer_Legacy
- 🐛 Correction rejection des paquets boat
- 📝 Mise à jour fonction printStructureInfo()

### Anémomètre v1.0.3
- ✨ Ajout timestamp dans AnemometerData
- 🔧 Initialisation timestamp = 0 (Display le remplit)
- 📝 Documentation STRUCTURE_UPDATES.md

### BoatGPS v1.0.3
- ✨ Ajout timestamp dans GPSBroadcastPacket
- 🔧 Initialisation timestamp = 0 (Display le remplit)
- 🐛 Correction compatibilité avec Display v1.0.3

## 🔗 Liens utiles

- **Documentation M5Burner** : `M5BURNER_INSTALLATION_GUIDE.md`
- **Guide rapide** : `M5BURNER_QUICK_GUIDE.md`
- **Checksums SHA256** : `SHA256SUMS.txt`
- **GitHub Repository** : https://github.com/FRA-222/Boat-GPS-Display

## 📞 Support

En cas de problème :
1. Vérifier que tous les appareils sont en v1.0.3
2. Vérifier les checksums SHA256
3. Consulter les logs série (115200 baud)
4. Ouvrir une issue sur GitHub

## 📅 Prochaines versions

### v1.0.4 (prévu)
- 🔜 Réception direction du vent depuis les bouées GPS
- 🔜 Affichage dynamique de la direction du vent
- 🔜 Calibration boussole pour les bouées
- 🔜 Historique graphique des données vent

---

**Bonne navigation ! ⛵**
