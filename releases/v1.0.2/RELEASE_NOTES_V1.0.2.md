# 🚀 OpenSailingRC Display - Version 1.0.2

**Date de release** : 5 octobre 2025  
**Plateforme** : M5Stack Core2 v1.1  
**Firmware** : ESP32

---

## 🎯 Résumé

Cette version corrige plusieurs problèmes critiques d'interface tactile et de serveur WiFi, et introduit une **solution de firmware fusionné** pour faciliter l'installation via M5Burner pour les utilisateurs non techniques.

---

## ✨ Nouvelles Fonctionnalités

### 🔥 **Firmware Fusionné pour M5Burner**
- **Fichier unique** : `OpenSailingRC_Display_v1.0.2_MERGED.bin` (1,1 MB)
- **Flash à l'adresse 0x0** : Contient bootloader + partitions + firmware
- **Installation simplifiée** : Compatible M5Burner en 3 clics
- **Solution au reboot infini** : Plus besoin de flasher 3 fichiers séparément
- **Documentation complète** : `M5BURNER_INSTRUCTIONS.txt` inclus

**Impact** : Permet à des utilisateurs sans connaissances techniques d'installer le firmware facilement.

---

## 🐛 Corrections de Bugs

### **1. Boutons Tactiles - Multi-Déclenchement (3 appuis)**

**Problème** :  
Un seul appui sur un bouton tactile déclenchait l'action **3 fois** consécutivement.

**Cause** :  
Utilisation d'un seul timer de debouncing `lastTouchTime` partagé entre les 3 boutons, causant des interférences.

**Solution** :
```cpp
// Avant (v1.0.1) - Timer unique
static unsigned long lastTouchTime = 0;
const unsigned long debounceDelay = 500;

// Après (v1.0.2) - Timer par bouton
static unsigned long lastTouchTimeButton1 = 0;
static unsigned long lastTouchTimeButton2 = 0;
static unsigned long lastTouchTimeButton3 = 0;
const unsigned long debounceDelay = 500;
```

**Résultat** : ✅ Chaque bouton a maintenant son propre timer de debouncing (500 ms).

---

### **2. Détection d'Appui Tactile - Déclenchement Continu**

**Problème** :  
Garder le doigt appuyé sur un bouton déclenchait l'action **en continu** toutes les 500 ms.

**Cause** :  
Utilisation de `button.getCount()` qui retourne `true` tant que le doigt reste sur le bouton.

**Solution** :
```cpp
// Avant (v1.0.1)
if (button1.getCount()) {
    // Déclenche à chaque cycle tant que le doigt est appuyé
}

// Après (v1.0.2)
if (button1.wasPressed()) {
    // Déclenche UNIQUEMENT au moment du PRESS initial
}
```

**Résultat** : ✅ Un appui = une action, même si le doigt reste appuyé.

---

### **3. Serveur WiFi - Activation/Désactivation Instable**

**Problème** :  
Appuyer sur le bouton WiFi activait le serveur, puis **le désactivait immédiatement**.

**Cause** :  
Combinaison de deux problèmes :
1. Multi-déclenchement du bouton (voir bug #1)
2. `wasPressed()` non utilisé (voir bug #2)

**Solution** :  
Corrections #1 et #2 combinées + condition de sécurité :

```cpp
// Avant (v1.0.1)
if (button3.getCount() && (millis() - lastTouchTime > debounceDelay)) {
    // Toggle serveur - mais déclenche plusieurs fois
}

// Après (v1.0.2)
if (button3.wasPressed() && (millis() - lastTouchTimeButton3 > debounceDelay)) {
    lastTouchTimeButton3 = millis();
    // Toggle serveur - une seule fois
}
```

**Résultat** : ✅ Le bouton WiFi active/désactive le serveur de façon stable.

---

### **4. Affichage URL Serveur - Disparition Après 3 Secondes**

**Problème** :  
L'URL du serveur WiFi (`192.168.4.1:80`) s'affichait puis **disparaissait après 3 secondes**.

**Cause racine #1** : Timeout dans `Display.cpp`
```cpp
// Display.cpp - Ancien code
if (millis() - serverMessageStartTime >= 3000) {
    serverMessageActive = false; // ← Désactivait après 3 secondes
}
```

**Cause racine #2** : Rafraîchissement continu de l'écran dans `main.cpp`
```cpp
// main.cpp - Ancien code
drawDisplay(gpsData, boatSpeed); // ← Appelé toutes les 100 ms
```

Ce refresh redessine tout l'écran, écrasant le message du serveur.

**Solution** :

**A) Display.cpp - Désactiver le timeout si serveur actif :**
```cpp
// Après (v1.0.2)
if (!serverMessageActive && millis() - serverMessageStartTime >= 3000) {
    serverMessageActive = false; // Timeout SEULEMENT si serveur inactif
}
```

**B) main.cpp - Suspendre le refresh quand serveur actif :**
```cpp
// Après (v1.0.2)
if (!fileServer.isServerActive()) {
    drawDisplay(gpsData, boatSpeed);
    drawButtonLabels(isRecording, fileServer.isServerActive());
}
```

**Résultat** : ✅ L'URL reste affichée **en permanence** tant que le serveur est actif.

---

### **5. Bouton WiFi - Feedback Visuel Incorrect**

**Problème** :  
Le bouton WiFi restait **rouge** même quand le serveur était actif (il devrait être **vert**).

**Cause** :  
`drawButtonLabels()` n'était pas appelé après le changement d'état du serveur.

**Solution** :
```cpp
// main.cpp - Après démarrage serveur
if (fileServer.startServer(ssid, password)) {
    display.drawButtonLabels(isRecording, true); // ← Force mise à jour
    display.updateServerMessageDisplay(true, ssid, password);
}

// main.cpp - Après arrêt serveur
fileServer.stopServer();
display.drawButtonLabels(isRecording, false); // ← Force mise à jour
display.updateServerMessageDisplay(false, "", "");
```

**Résultat** : ✅ Bouton WiFi passe au **vert** quand serveur actif, **rouge** quand inactif.

---

## 🔧 Améliorations Techniques

### **Debouncing Amélioré**
- **500 ms** par bouton (au lieu d'un timer partagé)
- **Indépendance** : Un appui sur un bouton n'affecte plus les autres
- **Robustesse** : Élimine les faux positifs

### **Gestion de l'Affichage**
- **Suspension conditionnelle** : Le refresh s'arrête quand le serveur est actif
- **Timeout intelligent** : Le message du serveur ne disparaît jamais tant que le serveur tourne
- **Mise à jour synchronisée** : Les boutons changent de couleur instantanément

### **Architecture du Firmware**
- **Firmware fusionné** : Bootloader + Partitions + Application en un seul fichier
- **Compatible M5Burner** : Flashable à l'adresse 0x0
- **Documentation** : Instructions détaillées pour utilisateurs non techniques

---

## 📦 Fichiers de la Release

### **Pour M5Burner (Utilisateurs Non Techniques)**
```
OpenSailingRC_Display_v1.0.2_MERGED.bin     (1,1 MB)
M5BURNER_INSTRUCTIONS.txt                   (Instructions)
```
→ **Flash à l'adresse 0x0** avec M5Burner

### **Pour ESPTool / PlatformIO (Développeurs)**
```
bootloader.bin    (17 KB)  → Flash à 0x1000
partitions.bin    (3 KB)   → Flash à 0x8000
firmware.bin      (1 MB)   → Flash à 0x10000
```

### **Documentation**
```
RELEASE_NOTES_V1.0.2.md
FIRMWARE_INFO.md
MERGED_FIRMWARE_M5BURNER.md
```

---

## 🧪 Tests de Validation

### **Tests Manuels Effectués**
- ✅ **Bouton STOP** : 1 appui = 1 enregistrement démarré/arrêté
- ✅ **Bouton WiFi** : 1 appui = serveur activé, 1 appui = serveur désactivé
- ✅ **Appui maintenu** : Pas de déclenchement multiple pendant qu'on garde le doigt appuyé
- ✅ **Affichage URL** : URL reste visible en permanence quand serveur actif
- ✅ **Couleur bouton WiFi** : Vert quand actif, rouge quand inactif
- ✅ **Flash M5Burner** : Firmware fusionné flashé à 0x0, Core2 démarre correctement

### **Tests Série (115200 baud)**
```
rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
...
✅ Pas de "invalid header"
✅ Pas de reboot en boucle
✅ Démarrage normal en 2 secondes
```

---

## 📊 Comparaison v1.0.1 → v1.0.2

| Aspect | v1.0.1 | v1.0.2 |
|--------|--------|--------|
| **Appui bouton** | 3 déclenchements | ✅ 1 déclenchement |
| **Appui maintenu** | Déclenche en continu | ✅ 1 seule fois |
| **Serveur WiFi** | Activation instable | ✅ Toggle stable |
| **Affichage URL** | Disparaît après 3s | ✅ Permanent |
| **Couleur WiFi** | Toujours rouge | ✅ Vert/Rouge dynamique |
| **Installation** | 3 fichiers (esptool) | ✅ 1 fichier (M5Burner) |
| **Public cible** | Développeurs | ✅ Tous publics |

---

## 🚀 Installation

### **Méthode 1 : M5Burner (Recommandée)**

**Pour qui ?** Utilisateurs non techniques, installation rapide

**Prérequis** :
- M5Burner installé ([télécharger ici](https://docs.m5stack.com/en/download))
- M5Stack Core2 connecté en USB

**Procédure** :
1. Ouvrir M5Burner
2. Aller dans "Custom Firmware"
3. Cliquer "Browse" → Sélectionner `OpenSailingRC_Display_v1.0.2_MERGED.bin`
4. **⚠️ IMPORTANT** : Changer l'adresse de `0x10000` à **`0x0`**
5. ✅ Cocher "Erase Flash"
6. Sélectionner le port série
7. Cliquer "BURN"
8. Attendre ~1 minute
9. ✅ Le Core2 redémarre et affiche l'interface

**Détails** : Voir `M5BURNER_INSTRUCTIONS.txt`

---

### **Méthode 2 : ESPTool (Avancée)**

**Pour qui ?** Développeurs, utilisateurs avancés

**Firmware Fusionné** :
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

**Fichiers Séparés** :
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

### **Méthode 3 : PlatformIO (Développement)**

**Pour qui ?** Développeurs souhaitant compiler le projet

```bash
# Cloner le projet
git clone https://github.com/philippe-hub/OpenSailingRC-Display.git
cd OpenSailingRC-Display

# Compiler et uploader
platformio run --target upload

# Moniteur série
platformio device monitor --baud 115200
```

---

## 📝 Fichiers Modifiés

### **Code Source**
- `src/main.cpp` : Debouncing par bouton, wasPressed(), suspension refresh
- `src/Display.cpp` : Timeout conditionnel pour message serveur
- `include/Display.h` : Signature méthode updateServerMessageDisplay

### **Scripts**
- `create_merged_firmware.sh` : **NOUVEAU** - Création firmware fusionné
- `platformio.ini` : Paramètres flash (dio, 80m, 16MB)

### **Documentation**
- `docs/MERGED_FIRMWARE_M5BURNER.md` : **NOUVEAU** - Guide complet firmware fusionné
- `releases/v1.0.2/M5BURNER_INSTRUCTIONS.txt` : **NOUVEAU** - Instructions M5Burner
- `releases/v1.0.2/RELEASE_NOTES_V1.0.2.md` : **NOUVEAU** - Ce document

---

## ⚠️ Breaking Changes

**Aucun**. Cette version est **100% rétrocompatible** avec v1.0.1.

Les modifications sont uniquement correctives et d'amélioration UX.

---

## 🔄 Migration depuis v1.0.1

### **Si vous utilisez ESPTool ou PlatformIO**
→ Aucune action requise. Flashez normalement.

### **Si vous utilisez M5Burner**
→ **IMPORTANT** : Utilisez le nouveau firmware fusionné à l'adresse **0x0** (voir instructions ci-dessus).

---

## 🐛 Problèmes Connus

Aucun problème connu dans cette version.

---

## 🆘 Support

### **Problème : Core2 Reboot en Boucle**
→ Vérifiez que vous avez flashé le firmware fusionné à l'adresse **0x0**, pas 0x10000.  
→ Cochez "Erase Flash" dans M5Burner avant de flasher.

### **Problème : Bouton Ne Répond Pas**
→ Vérifiez les logs série (115200 baud) pour voir si les événements tactiles sont détectés.  
→ Essayez de recalibrer l'écran tactile (maintenir bouton POWER 6 secondes).

### **Contact**
- **GitHub Issues** : https://github.com/philippe-hub/OpenSailingRC-Display/issues
- **Email** : philippe.hubert@example.com

---

## 🎯 Prochaines Versions

### **v1.1.0 (Planifiée)**
- [ ] Configuration WiFi via interface web
- [ ] OTA (Over-The-Air) firmware updates
- [ ] Logs détaillés sur carte SD
- [ ] Interface de calibration GPS/compas

### **v1.2.0 (Future)**
- [ ] Support Bluetooth pour GPS externe
- [ ] Mode nuit (luminosité réduite)
- [ ] Alarmes configurables (vitesse, cap, distance)

---

## 📚 Ressources

- **Documentation complète** : `docs/MERGED_FIRMWARE_M5BURNER.md`
- **Instructions M5Burner** : `releases/v1.0.2/M5BURNER_INSTRUCTIONS.txt`
- **M5Stack Docs** : https://docs.m5stack.com/en/core/core2
- **ESPTool** : https://docs.espressif.com/projects/esptool/
- **M5Burner** : https://docs.m5stack.com/en/download

---

## 🙏 Remerciements

Merci aux utilisateurs qui ont signalé les problèmes de boutons tactiles et de serveur WiFi.

Cette release n'aurait pas été possible sans vos retours détaillés ! 🎉

---

**Version** : 1.0.2  
**Date** : 5 octobre 2025  
**Auteur** : Philippe Hubert  
**Licence** : MIT

---

*Pour toute question ou problème, n'hésitez pas à ouvrir une issue sur GitHub.* 🚀