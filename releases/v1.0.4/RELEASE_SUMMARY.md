# Release v1.0.4 - OpenSailingRC-Display

**Date :** 25 novembre 2025  
**Type :** Feature + UI + Optimisation

## 📦 Contenu de la release

### Fichiers disponibles

| Fichier | Taille | Description |
|---------|--------|-------------|
| `OpenSailingRC_Display_v1.0.4_MERGED.bin` | 1.1 MB | Firmware combiné prêt à flasher |
| `README.md` | 8 KB | Guide d'installation complet |
| `RELEASE_NOTES_V1.0.4.md` | 9 KB | Notes de version détaillées |
| `SHA256SUMS.txt` | 106 B | Checksums de vérification |

### Checksum SHA256

```
1142a7a0e01f3f287249fe3076af914cb6a29023ab5ccfad2e9772f925042597  OpenSailingRC_Display_v1.0.4_MERGED.bin
```

## 🎯 Principales nouveautés

### 1. Indicateur de batterie
- Pictogramme 24x12px avec niveau proportionnel
- Code couleur : Vert (>50%), Orange (20-50%), Rouge (<20%)
- Éclair jaune quand en charge (USB-C)
- Position centrée en haut (X=160, Y=2)

### 2. Pictogramme satellite
- Remplace le texte "SAT"
- Corps blanc + panneaux solaires bleus
- Design 14x12px moderne et international

### 3. Amélioration UI des boutons
- Labels : "RECORD/STOP" et "WIFI/STOP"
- Couleurs : Bleu marine (inactif) / Rouge (actif)
- Plus intuitif et cohérent

### 4. Message serveur repensé
- Serveur actif : Fond rouge
- Serveur arrêté : Fond bleu marine
- Rafraîchissement écran automatique corrigé

### 5. Optimisation ESP-NOW
- **-8 bytes par paquet** (timestamp + boatId supprimés)
- Paquet réduit de 60 → 52 bytes (-13%)
- Meilleure portée et fiabilité
- Support noms personnalisés bateau

## 🚀 Installation

### Via M5Burner (recommandé)

```
1. Connecter M5Stack Core2
2. Charger OpenSailingRC_Display_v1.0.4_MERGED.bin
3. Adresse: 0x0000
4. Cliquer "Burn"
```

### Via esptool

```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 \
  write_flash 0x0000 OpenSailingRC_Display_v1.0.4_MERGED.bin
```

## ✅ Compatibilité

| Composant | Version min | Version recommandée | Status |
|-----------|-------------|---------------------|--------|
| BoatGPS | v1.0.3 | v1.0.4 | ✅ Compatible |
| Anemometer | v1.0.3 | v1.0.3 | ✅ Compatible |
| M5Stack Core2 | Hardware v1.0 | - | ✅ Testé |

## 📊 Tests effectués

### Matériel
- ✅ M5Stack Core2 (390mAh battery)
- ✅ M5Stack AtomS3 BoatGPS v1.0.4
- ✅ M5Stack AtomS3 Anemometer v1.0.3

### Fonctionnalités
- ✅ Indicateur batterie avec code couleur
- ✅ Indicateur de charge (éclair jaune)
- ✅ Pictogramme satellite visible
- ✅ Boutons bleu marine/rouge fonctionnels
- ✅ Message serveur fond rouge/bleu marine
- ✅ Réception paquets optimisés
- ✅ Affichage noms personnalisés
- ✅ Rafraîchissement écran correct
- ✅ Logs SD avec noms personnalisés
- ✅ Portée 90m (75% réception)

## 🔄 Migration depuis v1.0.3

### Changements visuels

**Écran principal :**
- 🆕 Indicateur batterie en haut centre
- 🆕 Pictogramme satellite remplace "SAT"
- 🔄 Boutons : Gris/Vert → Bleu marine/Rouge
- 🔄 Labels : "REC/STOP/SERV" → "RECORD/STOP/STOP"

### Changements techniques

**Structure des données :**
- ❌ `timestamp` supprimé → Variables globales
- ❌ `boatId` supprimé → Calculé localement
- ✅ Support `name` personnalisé

**Impact :**
- BoatGPS v1.0.3 : ⚠️ Fonctionne (MAC address)
- BoatGPS v1.0.4 : ✅ Optimisé (noms + -8 bytes)

**Action requise :**
- Mettre à jour BoatGPS en v1.0.4 pour optimisations complètes

## 📚 Documentation

### Guides disponibles
- `README.md` - Installation complète
- `RELEASE_NOTES_V1.0.4.md` - Notes de version
- Interface utilisateur illustrée

### Documentation technique
- Structure des paquets ESP-NOW
- Gestion des timestamps
- API batterie M5Stack

## 🐛 Problèmes connus

- ⚠️ Warnings compilation ArduinoJson (fonctionnel, migration v1.1.0)

## 🔮 Roadmap v1.0.5

- Direction du vent depuis bouées GPS
- Graphiques de tendance
- Mode nuit avec luminosité adaptative

## 📊 Statistiques

### Firmware
- **Taille :** 1047 KB (16% flash)
- **RAM :** 48 KB (1.1%)
- **Temps compilation :** 2.84s

### Batterie
- **Capacité :** 390mAh
- **Autonomie normale :** 2-3h
- **Autonomie économie :** 4-5h
- **Charge USB-C :** 5V/1A

### Performance
- **Démarrage :** <5s
- **Latence affichage :** <100ms
- **Portée ESP-NOW :** 90m testé (75% réception)

## 📄 Licence

GNU General Public License v3.0

## 👥 Contributeurs

- Philippe Hubert (@FRA-222)
- OpenSailingRC Community

---

**Installation recommandée :** M5Burner à l'adresse 0x0000

**Compatibilité optimale :** Avec BoatGPS v1.0.4

**Prochaine étape :** Mettre à jour BoatGPS pour bénéficier des noms personnalisés
