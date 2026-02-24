# OpenSailingRC-Display v1.0.5

**Date de publication :** 24 février 2026  
**Type :** Feature + Bugfix UI

## 🎯 Nouveautés

### 🚤 Affichage du nom de bateau sélectionné
- Affichage en haut à gauche du **nom du bateau** sélectionné (tronqué à 6 caractères) en **jaune**
- Suivi de l'**indice x/y** (bateau sélectionné / nombre total de bateaux émetteurs) en blanc
- Exemple : `FRA001 1/4`
- Affichage automatique `NO BOAT` en rouge si aucun bateau ne transmet
- Mise à jour optimisée (uniquement si le nom, l'index ou le total change)

### 🔋 Repositionnement batterie et satellites
- **Jauge batterie** et pourcentage décalés à droite pour laisser la place au nom du bateau
- Position batterie centrée autour de X=190 (au lieu de X=160)
- Meilleur équilibre visuel de la barre de statut en haut

## 🐛 Corrections

### 🔧 Fix du décalage vertical des données (TextDatum)
- **Bug :** Les 4 valeurs numériques du milieu (vitesse/cap bateau, vitesse/direction vent) changeaient de position verticale de manière aléatoire
- **Cause :** La fonction `drawButtonLabels()` changeait le `TextDatum` à `MC_DATUM` (centré) pour dessiner les boutons, mais ne le restaurait pas. Lors des appels suivants à `drawDisplay()`, les valeurs se dessinaient avec le mauvais datum
- **Fix :** Ajout de `M5.Lcd.setTextDatum(TL_DATUM)` systématique en début de `drawDisplay()` pour garantir un datum cohérent

### 📐 Alignement des zones d'effacement et d'écriture
- Les `fillRect` (effacement) et `setCursor` (écriture) des 4 valeurs centrales utilisent maintenant exactement les mêmes coordonnées Y que les labels fixes (BOAT/WIND/KTS/DEG)
- Élimine tout saut de position entre l'affichage initial et les mises à jour

## 📦 Firmware

| Fichier | Taille | SHA256 |
|---------|--------|--------|
| `OpenSailingRC_Display_v1.0.5_MERGED.bin` | ~1.1 MB | `0f63afb1...` |

### Installation avec M5Burner
1. Ouvrir M5Burner
2. Cliquer sur "Custom Firmware"
3. Sélectionner `OpenSailingRC_Display_v1.0.5_MERGED.bin`
4. **⚠️ Adresse de flash : 0x0** (pas 0x10000)
5. Cocher "Erase Flash"
6. Cliquer "Burn"

## 📊 Compatibilité

- **Hardware :** M5Stack Core2
- **ESP-NOW :** Compatible avec BoatGPS v1.0.4+ et Anémomètre
- **Multi-bateaux :** Supporte jusqu'à N bateaux simultanés avec sélection via bouton "BOAT ?"
