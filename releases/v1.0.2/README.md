# 📦 OpenSailingRC Display - Release v1.0.2

**Date** : 5 octobre 2025  
**Plateforme** : M5Stack Core2 v1.1

---

## 🎯 Contenu de la Release

Cette archive contient **tous les fichiers nécessaires** pour installer le firmware OpenSailingRC Display v1.0.2 sur votre M5Stack Core2.

```
releases/v1.0.2/
├── 📄 README.md                                    ← Ce fichier
├── 📄 RELEASE_NOTES_V1.0.2.md                      ← Notes de version détaillées
├── 📄 FIRMWARE_INFO.md                             ← Informations techniques
├── 📄 M5BURNER_INSTRUCTIONS.txt                    ← Instructions M5Burner
│
├── 🔥 OpenSailingRC_Display_v1.0.2_MERGED.bin      ← FIRMWARE FUSIONNÉ (M5Burner)
│
├── 🔧 bootloader.bin                               ← Bootloader (ESPTool)
├── 🔧 partitions.bin                               ← Partitions (ESPTool)
└── 🔧 firmware.bin                                 ← Application (ESPTool)
```

---

## ⚡ Installation Rapide

### **📱 Pour Utilisateurs Non Techniques : M5Burner**

**Ce dont vous avez besoin** :
- M5Burner installé ([télécharger ici](https://docs.m5stack.com/en/download))
- M5Stack Core2
- Câble USB-C

**Installation en 3 étapes** :

1️⃣ **Ouvrir M5Burner** → Custom Firmware → Browse  
2️⃣ **Sélectionner** : `OpenSailingRC_Display_v1.0.2_MERGED.bin`  
3️⃣ **⚠️ IMPORTANT** : Changer l'adresse de `0x10000` à **`0x0`**  
4️⃣ **Cocher** : Erase Flash  
5️⃣ **Cliquer** : BURN  

⏱️ **Temps d'installation** : ~1 minute

✅ **Terminé !** Le Core2 redémarre et affiche l'interface.

📖 **Détails complets** : Voir `M5BURNER_INSTRUCTIONS.txt`

---

### **💻 Pour Développeurs : ESPTool**

**Firmware Fusionné (recommandé)** :
```bash
python3 -m esptool --chip esp32 \
    --port /dev/cu.usbserial-XXXXXXXX \
    --baud 115200 \
    write_flash -z \
    --flash_mode dio \
    --flash_freq 80m \
    --flash_size 16MB \
    0x0 OpenSailingRC_Display_v1.0.2_MERGED.bin
```

**Fichiers Séparés (avancé)** :
```bash
python3 -m esptool --chip esp32 \
    --port /dev/cu.usbserial-XXXXXXXX \
    --baud 115200 \
    write_flash -z \
    --flash_mode dio \
    --flash_freq 80m \
    --flash_size 16MB \
    0x1000 bootloader.bin \
    0x8000 partitions.bin \
    0x10000 firmware.bin
```

---

## 📖 Documentation

### **Pour Tous**
- `README.md` (ce fichier) : Vue d'ensemble
- `M5BURNER_INSTRUCTIONS.txt` : Installation M5Burner étape par étape

### **Pour Utilisateurs Avancés**
- `RELEASE_NOTES_V1.0.2.md` : Corrections et nouvelles fonctionnalités
- `FIRMWARE_INFO.md` : Détails techniques (mémoire, partitions, compilation)

### **Documentation Complète**
- `docs/MERGED_FIRMWARE_M5BURNER.md` : Guide complet firmware fusionné

---

## 🆕 Nouveautés v1.0.2

### ✨ **Installation Simplifiée**
- **Firmware fusionné** en un seul fichier
- **Compatible M5Burner** (installation graphique)
- **Fini les reboots infinis** après installation

### 🐛 **Corrections Majeures**
- ✅ **Boutons tactiles** : Plus de multi-déclenchements
- ✅ **Serveur WiFi** : Activation/désactivation stable
- ✅ **Affichage URL** : Reste visible en permanence
- ✅ **Feedback visuel** : Bouton WiFi vert/rouge selon état

📋 **Détails complets** : Voir `RELEASE_NOTES_V1.0.2.md`

---

## ❓ FAQ

### **Q : Pourquoi deux versions du firmware ?**
**R** : 
- **MERGED.bin** : Pour M5Burner (facile, 1 fichier)
- **3 fichiers séparés** : Pour ESPTool/PlatformIO (développeurs)

Les deux sont identiques fonctionnellement.

---

### **Q : Quelle est la différence avec v1.0.1 ?**
**R** : v1.0.2 corrige 5 bugs critiques :
1. Multi-déclenchement des boutons tactiles (3 appuis au lieu de 1)
2. Déclenchement continu quand on maintient le doigt
3. Serveur WiFi qui se désactive immédiatement
4. URL serveur qui disparaît après 3 secondes
5. Bouton WiFi qui reste rouge même quand serveur actif

**Impact** : Interface beaucoup plus stable et utilisable.

---

### **Q : Pourquoi l'adresse 0x0 et pas 0x10000 ?**
**R** : Le firmware fusionné contient **bootloader + partitions + application**.  
- À **0x0** : Tout est flashé correctement → ✅ Fonctionne
- À **0x10000** : Bootloader manquant → ❌ Reboot infini

📖 **Détails techniques** : Voir `docs/MERGED_FIRMWARE_M5BURNER.md`

---

### **Q : Puis-je utiliser v1.0.2 si j'ai v1.0.1 ?**
**R** : Oui ! v1.0.2 est **100% rétrocompatible**.  
Il suffit de flasher normalement, aucune configuration à modifier.

---

### **Q : Le Core2 reboot en boucle après installation ?**
**R** : Vérifiez :
1. ✅ Fichier utilisé : `MERGED.bin` (pas `firmware.bin`)
2. ✅ Adresse : **0x0** (pas 0x10000)
3. ✅ Erase Flash coché dans M5Burner

Si le problème persiste :
```bash
# Effacer complètement la flash
python3 -m esptool --chip esp32 --port PORT erase_flash

# Reflasher
python3 -m esptool --chip esp32 --port PORT write_flash 0x0 MERGED.bin
```

---

### **Q : Puis-je revenir à v1.0.1 ?**
**R** : Oui, flashez simplement v1.0.1 par-dessus v1.0.2.  
Aucun risque de "brick" du Core2.

---

## 🔍 Vérification de l'Installation

### **Test 1 : Splash Screen**
Au démarrage, vous devez voir :
```
┌────────────────────┐
│      WIND          │
│       /            │
│      BOAT          │
└────────────────────┘
```
⏱️ Pendant 2 secondes

---

### **Test 2 : Interface Principale**
```
┌────────────────────────────────────┐
│ BOAT 0.0 KTS      GPS 0 SAT        │
│                                    │
│         N                          │
│        000                         │
│                                    │
│ BUOY 0.0 KTS                       │
│                                    │
├────────┬───────────┬───────────────┤
│  STOP  │   (vide)  │     WIFI      │
│  (RED) │           │     (RED)     │
└────────┴───────────┴───────────────┘
```

---

### **Test 3 : Logs Série**
Connectez-vous à 115200 baud :
```
rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
...
✅ Pas de "invalid header"
✅ Pas de reboot en boucle
```

---

### **Test 4 : Boutons Tactiles**
1. **Appuyer sur STOP** → 1 seule action (enregistrement démarre)
2. **Appuyer sur WIFI** → Serveur démarre, bouton devient VERT
3. **Maintenir le doigt** → Pas de déclenchement multiple

---

## 🛠️ Dépannage

### **Problème : M5Burner n'accepte pas le fichier**
**Solution** : Vérifiez que vous sélectionnez bien `MERGED.bin` (1,1 MB).  
Ne sélectionnez **pas** `firmware.bin` (1 MB).

---

### **Problème : "Error: No serial data received"**
**Solutions** :
1. Vérifier le câble USB-C (certains câbles sont "charge only")
2. Installer les drivers CH9102 : https://docs.m5stack.com/en/download
3. Essayer un autre port USB
4. Redémarrer M5Burner

---

### **Problème : "Hash of data does not match"**
**Solution** : Le fichier est corrompu.
1. Re-télécharger l'archive depuis GitHub
2. Vérifier l'intégrité :
```bash
shasum -a 256 OpenSailingRC_Display_v1.0.2_MERGED.bin
```
3. Comparer avec le hash fourni dans la release GitHub

---

### **Problème : Écran reste noir après flash**
**Solutions** :
1. Appuyer sur le bouton RESET (à côté du port USB)
2. Débrancher/rebrancher l'alimentation
3. Vérifier les logs série (115200 baud)

---

### **Problème : Boutons ne répondent pas**
**Solutions** :
1. Recalibrer l'écran tactile :
   - Maintenir bouton POWER 6 secondes
   - Suivre les instructions à l'écran
2. Vérifier les logs série pour voir si le touch est détecté
3. Essayer de toucher fermement au centre des boutons

---

## 📞 Support

### **Documentation**
- GitHub Wiki : https://github.com/philippe-hub/OpenSailingRC-Display/wiki
- Issues : https://github.com/philippe-hub/OpenSailingRC-Display/issues

### **Contact**
- Email : philippe.hubert@example.com
- Discord : OpenSailingRC Community

---

## 📜 Licence

MIT License

Copyright (c) 2025 Philippe Hubert

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

---

## 🙏 Remerciements

- **M5Stack** pour le hardware Core2
- **Espressif** pour l'ESP32 et l'écosystème
- **PlatformIO** pour l'excellent environnement de développement
- **La communauté** pour les retours et suggestions

---

**🎉 Merci d'utiliser OpenSailingRC Display !**

Si vous aimez ce projet, n'hésitez pas à lui donner une ⭐ sur GitHub !

---

**Version** : 1.0.2  
**Date** : 5 octobre 2025  
**Auteur** : Philippe Hubert

---

*Pour toute question, consultez la documentation complète ou ouvrez une issue sur GitHub.* 🚀