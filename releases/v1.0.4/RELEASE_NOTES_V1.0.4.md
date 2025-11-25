# OpenSailingRC-Display v1.0.4

**Date de publication :** 25 novembre 2025  
**Type :** Feature + UI + Optimisation

## 🎯 Nouveautés

### ✨ Indicateur de batterie
- Affichage du **pourcentage de batterie** en haut au centre de l'écran
- **Pictogramme batterie** (24x12px) avec niveau de charge proportionnel
- **Code couleur** selon le niveau :
  - 🟢 **Vert** : >50% (autonomie confortable)
  - 🟠 **Orange** : 20-50% (surveillance recommandée)
  - 🔴 **Rouge** : <20% (recharge urgente)
- **Indicateur de charge** : Éclair jaune quand USB-C branché
- Mise à jour optimisée (uniquement si changement)

**Position :** Centré en haut (X=160, Y=2)  
**Dimensions :** 140px large, 25px haut

### 🛰️ Pictogramme satellite
- Remplacement du texte "SAT" par un **pictogramme satellite**
- Design : Corps central blanc + panneaux solaires bleus (14x12px)
- Affichage plus moderne et international
- Meilleur espacement avec le nombre de satellites

**Position :** En haut à droite (X=245, Y=3)

### 🎨 Amélioration des boutons
- **Labels plus clairs** :
  - "RECORD" (inactif) → "STOP" (actif) pour l'enregistrement GPS
  - "WIFI" (inactif) → "STOP" (actif) pour le serveur de fichiers
- **Nouveau code couleur** :
  - 🔵 **Bleu marine** : Fonction inactive
  - 🔴 **Rouge** : Fonction active
  - (Remplace gris/vert de v1.0.3)
- Labels plus intuitifs (affichent l'action à effectuer)

### 📶 Message serveur amélioré
- **Serveur actif** : Fond rouge, texte noir, URL en noir
- **Serveur arrêté** : Fond bleu marine, texte blanc
- Meilleure visibilité et cohérence avec les boutons
- Rafraîchissement automatique de l'écran après message

### 📦 Optimisation de la communication ESP-NOW

**Réception de paquets optimisés du BoatGPS v1.0.4 :**
- ❌ Suppression de `timestamp` (4 bytes) - géré localement
- ❌ Suppression de `boatId` (4 bytes) - calculé localement
- **Économie totale : 8 bytes par paquet** (de ~60 à ~52 bytes)

**Bénéfices :**
- 🚀 Meilleure portée ESP-NOW
- 📶 Fiabilité accrue à longue distance (90m+)
- 📉 Réduction de la charge réseau de ~13%

**Gestion des timestamps :**
- Nouvelles variables globales : `boatDataTimestamp` et `anemometerDataTimestamp`
- Timestamp défini lors de la réception du paquet
- Détection de timeout (5 secondes) toujours fonctionnelle

### 📝 Support des noms personnalisés
- Affichage du **nom personnalisé du bateau** si défini dans BoatGPS v1.0.4
- Compatible avec l'adresse MAC (fallback automatique)
- Noms personnalisés enregistrés dans les logs SD
- Identification facilitée avec plusieurs bateaux

## 🔧 Améliorations techniques

### Gestion des timestamps
- Séparation entre timestamp GPS et timestamp de réception
- Variables globales pour éviter la duplication dans les structures
- Optimisation mémoire et performance

### Rafraîchissement de l'écran
- Correction du bug d'effacement partiel après message serveur
- Réinitialisation complète des labels (BOAT, WIND, KTS, DEG)
- Pas de zones fantômes après arrêt du serveur

### Compatibilité
- ✅ **Rétrocompatible** avec BoatGPS v1.0.3 (adresse MAC utilisée)
- ✅ **Compatible** avec BoatGPS v1.0.4 (nom personnalisé + optimisations)
- ✅ **Compatible** avec Anemometer v1.0.3

## 📋 Structure des données reçues

```cpp
struct struct_message_Boat {
    int8_t messageType;           // 1 = Boat
    char name[18];                // Nom personnalisé ou MAC
    uint32_t sequenceNumber;      // Numéro de séquence
    uint32_t gpsTimestamp;        // Timestamp GPS
    float latitude;               // Latitude en degrés
    float longitude;              // Longitude en degrés
    float speed;                  // Vitesse en nœuds
    float heading;                // Cap en degrés
    uint8_t satellites;           // Nombre de satellites
    // timestamp supprimé (géré localement)
    // boatId supprimé (calculé localement)
};
```

**Taille du paquet : ~52 bytes** (optimisé avec BoatGPS v1.0.4)

## 📊 Spécifications batterie

### Autonomie typique (M5Stack Core2)

| Mode | Autonomie | Conditions |
|------|-----------|------------|
| Normal | 2-3 heures | Display actif, WiFi ON, enregistrement GPS |
| Économie | 4-5 heures | Luminosité réduite, WiFi OFF |

### Seuils d'alerte

| Niveau | Couleur | Action recommandée |
|--------|---------|-------------------|
| >50% | 🟢 Vert | Utilisation normale |
| 20-50% | 🟠 Orange | Surveiller le niveau |
| <20% | 🔴 Rouge | Recharger rapidement |

### Calibration
La calibration de la batterie peut nécessiter 2-3 cycles charge/décharge complets pour une précision optimale.

## 📦 Installation

### Via M5Burner (recommandé)

1. **Télécharger le firmware :**
   - `OpenSailingRC_Display_v1.0.4_MERGED.bin`

2. **Configurer M5Burner :**
   - Port : Sélectionner le port USB du Core2
   - Firmware : Charger le fichier .bin
   - Adresse : `0x0000`

3. **Flasher :**
   - Cliquer sur "Burn"
   - Attendre la fin du flash
   - Redémarrer le Core2

### Via PlatformIO

```bash
cd OpenSailingRC-Display
platformio run --target upload --environment m5stack-core2
```

## 🧪 Tests effectués

### Matériel testé
- ✅ M5Stack Core2 (390mAh battery)
- ✅ M5Stack AtomS3 BoatGPS v1.0.4
- ✅ M5Stack AtomS3 Anemometer v1.0.3

### Fonctionnalités validées
- ✅ Indicateur de batterie avec code couleur
- ✅ Pictogramme satellite avec panneaux bleus
- ✅ Boutons bleu marine/rouge fonctionnels
- ✅ Message serveur avec fond rouge/bleu marine
- ✅ Réception paquets optimisés BoatGPS v1.0.4
- ✅ Affichage nom personnalisé bateau
- ✅ Timestamps locaux fonctionnels
- ✅ Rafraîchissement écran après message serveur
- ✅ Logs SD avec noms personnalisés

### Performances
- Taille du firmware : **1047 KB** (16% de la flash)
- Utilisation RAM : **48 KB** (1.1%)
- Portée ESP-NOW testée : **90m** avec 75% de réception

## 📝 Checksums (SHA256)

```
1142a7a0e01f3f287249fe3076af914cb6a29023ab5ccfad2e9772f925042597  OpenSailingRC_Display_v1.0.4_MERGED.bin
```

## 🔗 Compatibilité

| Composant | Version min | Version recommandée | Optimisations |
|-----------|-------------|---------------------|---------------|
| BoatGPS | v1.0.3 | v1.0.4 | Complètes avec v1.0.4 |
| Anemometer | v1.0.3 | v1.0.3 | ✅ Complètes |

## 📚 Changelog détaillé

### Ajouts
- ✅ Indicateur batterie centré en haut
- ✅ Pictogramme satellite remplaçant "SAT"
- ✅ Support noms personnalisés bateau
- ✅ Variables globales timestamps (boatDataTimestamp, anemometerDataTimestamp)
- ✅ Rafraîchissement complet après message serveur

### Modifications
- 🔄 Labels boutons : "REC/STOP" → "RECORD/STOP", "SERV" → "STOP"
- 🔄 Couleurs boutons : Gris/Vert → Bleu marine/Rouge
- 🔄 Message serveur : Fond vert → Fond rouge (actif), Fond rouge → Fond bleu marine (arrêté)
- 🔄 Structure struct_message_Boat : Suppression timestamp et boatId

### Corrections
- 🐛 Écran partiellement effacé après message serveur
- 🐛 Labels WIND disparaissant après arrêt serveur

## 🐛 Problèmes connus

- ⚠️ Warnings de compilation ArduinoJson (deprecated APIs) - fonctionnel mais à migrer en v1.1.0

## 🚀 Prochaines versions

### Prévu pour v1.0.5
- Direction du vent depuis les bouées GPS
- Graphiques de tendance (vitesse, cap)
- Mode nuit avec luminosité adaptative

### Prévu pour v1.1.0
- Migration ArduinoJson v7 (API moderne)
- Écran tactile pour sélection bateau
- Replay des sessions enregistrées

## 🔧 Développement

### Dépendances
- M5Unified v0.2.5
- ArduinoJson v7.4.2
- WiFi v2.0.0
- WebServer v2.0.0
- SD v2.0.0

### Environnement de build
- PlatformIO 6.1.x
- ESP32 Arduino Framework 2.0.14
- ESP-IDF 4.4.x

## 📄 Licence

GNU General Public License v3.0

Copyright (c) 2025 OpenSailingRC Contributors

## 👥 Contributeurs

- Philippe Hubert (@FRA-222)
- OpenSailingRC Community

## 🙏 Remerciements

Merci à tous les testeurs pour leurs retours sur l'interface utilisateur !

---

**Recommandation :** Utiliser avec BoatGPS v1.0.4 pour bénéficier de toutes les optimisations et du support des noms personnalisés.
