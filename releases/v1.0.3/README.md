# OpenSailingRC v1.0.3

## 📦 Contenu de la release

Cette release contient 3 firmwares merged bin prêts à flasher avec M5Burner :

### Fichiers
- ✅ `OpenSailingRC_Display_v1.0.3_MERGED.bin` (1.05 MB) - M5Stack Core2
- ✅ `OpenSailingRC_Anemometer_v1.0.3_MERGED.bin` (896 KB) - M5Stack AtomS3
- ✅ `OpenSailingRC_BoatGPS_v1.0.3_MERGED.bin` (929 KB) - M5Stack AtomS3

### Documentation
- 📝 `RELEASE_NOTES_V1.0.3.md` - Nouveautés et changements
- 📝 `FIRMWARE_INFO.md` - Spécifications techniques détaillées
- 📝 `M5BURNER_INSTRUCTIONS.txt` - Guide d'installation rapide
- 🔒 `SHA256SUMS.txt` - Checksums de vérification

## 🎯 Nouveautés v1.0.3

### Affichage 4 lignes
- BOAT vitesse & cap
- WIND vitesse & direction (préparé pour bouées)

### Système de timeout
- Affichage `--` après 5 secondes sans données
- Détection automatique de perte de signal

### Amélioration terminologie
- BUOY → WIND (plus logique)

### Structures mises à jour
- Ajout timestamp pour timeout
- Suppression code legacy
- Meilleure compatibilité

## 📱 Installation rapide

### Via M5Burner
1. Télécharger M5Burner
2. Flasher chaque appareil avec son `.bin` à l'adresse `0x0`
3. C'est tout ! ⛵

### Ordre recommandé
1. Display (Core2)
2. Anémomètre (AtomS3)
3. BoatGPS (AtomS3)

## ⚠️ Important

**Tous les appareils doivent être en v1.0.3**
- Incompatibilité avec les versions précédentes
- Mise à jour complète requise

## 🔗 Liens utiles

- **Guide complet** : `M5BURNER_INSTALLATION_GUIDE.md` (dossier releases)
- **M5Burner** : https://docs.m5stack.com/en/download
- **GitHub** : https://github.com/FRA-222/Boat-GPS-Display

## 📊 Checksums SHA256

```
37747c0c8de64d965b0049bf26a07e7facb3260c956d23fd37a279d2ce094dde  OpenSailingRC_Anemometer_v1.0.3_MERGED.bin
47824faba6a42447122673b4a7028520d3977a7b9c21592f4fe0deb4ceb03ae3  OpenSailingRC_BoatGPS_v1.0.3_MERGED.bin
3afb2f6bfc82f4be241cd3a2b57dfd1ec97780393b2f73a6600154a0cc82d21f  OpenSailingRC_Display_v1.0.3_MERGED.bin
```

## 🎉 Prêt à naviguer !

Cette version améliore considérablement l'affichage et la fiabilité du système.

**Bonne navigation ! ⛵**

---

**Version** : 1.0.3  
**Date** : 23 novembre 2025  
**Compatibilité** : M5Stack Core2, M5Stack AtomS3
