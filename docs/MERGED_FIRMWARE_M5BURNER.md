# 🎯 Solution Firmware Fusionné pour M5Burner

## 📅 Date : 5 octobre 2025

## 🔍 Problème Initial

### **Symptôme :**
Quand on utilise M5Burner avec seulement `OpenSailingRC_Display_v1.0.1.bin`, le M5Stack Core2 **reboot en boucle** et n'arrive pas à exécuter le programme.

### **Cause :**
Un firmware ESP32 complet nécessite **3 fichiers** :
1. **Bootloader** (0x1000) : 17 KB - Initialise l'ESP32
2. **Partitions** (0x8000) : 3 KB - Table des partitions
3. **Firmware** (0x10000) : 1 MB - Programme principal

**Problème** : M5Burner ne peut flasher qu'**un seul fichier .bin** à la fois, et ne gère pas automatiquement le bootloader et les partitions.

---

## ✅ Solution : Firmware Fusionné (Merged Binary)

### **Principe :**
Fusionner les 3 fichiers en **un seul fichier .bin** qui contient tout, flashable à l'adresse **0x0**.

```
┌──────────────────────────────────────────────────┐
│  FIRMWARE FUSIONNÉ (1.1 MB)                      │
├──────────────────────────────────────────────────┤
│  0x0000   : Padding/Header                       │
│  0x1000   : Bootloader (17 KB)                   │
│  0x8000   : Partitions (3 KB)                    │
│  0x10000  : Firmware Application (1 MB)          │
└──────────────────────────────────────────────────┘
        ↓ Flash en une seule fois à 0x0
┌──────────────────────────────────────────────────┐
│              MÉMOIRE FLASH ESP32                 │
└──────────────────────────────────────────────────┘
```

---

## 🔧 Script de Création

### **Fichier : create_merged_firmware.sh**

Le script utilise `esptool merge_bin` pour fusionner les fichiers :

```bash
python3 -m esptool --chip esp32 merge_bin \
    -o OpenSailingRC_Display_v1.0.2_MERGED.bin \
    --flash_mode dio \
    --flash_freq 80m \
    --flash_size 16MB \
    0x1000 bootloader.bin \
    0x8000 partitions.bin \
    0x10000 firmware.bin
```

### **Résultat :**
- **Fichier** : `OpenSailingRC_Display_v1.0.2_MERGED.bin`
- **Taille** : 1,1 MB (1.120.896 bytes)
- **Adresse de flash** : **0x0** (très important !)

---

## 📋 Instructions M5Burner

### **⚠️ CONFIGURATION CRITIQUE**

La différence entre succès et reboot infini tient à **un seul paramètre** :

| Paramètre | ❌ Échec (Reboot) | ✅ Succès |
|-----------|-------------------|-----------|
| **Fichier** | OpenSailingRC_Display_v1.0.1.bin | OpenSailingRC_Display_v1.0.2_MERGED.bin |
| **Adresse** | 0x10000 | **0x0** |
| **Erase Flash** | Optionnel | **OBLIGATOIRE** |

### **Procédure Complète :**

```
1. Ouvrir M5Burner
2. Custom Firmware
3. Browse → Sélectionner : OpenSailingRC_Display_v1.0.2_MERGED.bin
4. ⚠️  CHANGER L'ADRESSE : 0x10000 → 0x0
5. ✅ Cocher : Erase Flash
6. Sélectionner le port série
7. BURN
8. Attendre ~1 minute
9. ✅ Le Core2 redémarre et fonctionne !
```

---

## 🧪 Test de Validation

### **Test 1 : Flash avec esptool**
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

**Résultat attendu :**
```
Wrote 1120896 bytes (680388 compressed) at 0x00000000
Hash of data verified.
Leaving...
Hard resetting via RTS pin...
```

### **Test 2 : Démarrage du Core2**
```
1. Splash "WIND / BOAT" (2 secondes)
2. Interface principale :
   - BOAT 0.0 KTS | GPS 0 SAT
   - Boussole 000 DEG
   - BUOY 0.0 KTS
   - Boutons : STOP | (vide) | WIFI
```

### **Test 3 : Logs Série (115200 baud)**
```
ets Jul 29 2019 12:21:46
rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:1184
...
✅ Pas de "invalid header" ou reboot en boucle
```

---

## 📊 Comparaison des Méthodes

| Méthode | Fichiers | Adresse | Complexité | Public |
|---------|----------|---------|------------|--------|
| **PlatformIO** | 3 séparés | 0x1000, 0x8000, 0x10000 | Élevée | Développeurs |
| **ESPTool Manuel** | 3 séparés | 0x1000, 0x8000, 0x10000 | Moyenne | Avancés |
| **M5Burner (classique)** | 1 seul | 0x10000 | ❌ Ne fonctionne pas | - |
| **M5Burner (fusionné)** | 1 fusionné | **0x0** | ✅ Simple | **Tous publics** |

---

## 🎯 Avantages du Firmware Fusionné

### **Pour les Utilisateurs Non Techniques :**
- ✅ **Un seul fichier** à gérer
- ✅ **Interface graphique** (M5Burner)
- ✅ **Installation simple** (3 clics)
- ✅ **Pas de ligne de commande**
- ✅ **Compatible Windows/Mac/Linux**

### **Pour la Distribution :**
- ✅ **Fichier unique** à héberger sur GitHub
- ✅ **Instructions simplifiées**
- ✅ **Moins d'erreurs** utilisateur
- ✅ **Support plus facile**

---

## 📦 Fichiers de Release V1.0.2

```
releases/v1.0.2/
├── OpenSailingRC_Display_v1.0.2_MERGED.bin     # 1,1 MB - Pour M5Burner
├── M5BURNER_INSTRUCTIONS.txt                   # Instructions détaillées
├── bootloader.bin                              # 17 KB - Pour ESPTool manuel
├── partitions.bin                              # 3 KB - Pour ESPTool manuel
├── firmware.bin                                # 1 MB - Pour ESPTool manuel
├── FIRMWARE_INFO.md                            # Infos techniques
└── RELEASE_NOTES_V1.0.2.md                     # Notes de version
```

---

## 🔄 Processus de Création

### **Étape 1 : Compiler le Projet**
```bash
platformio run
```
→ Génère les fichiers dans `.pio/build/m5stack-core2/`

### **Étape 2 : Créer le Firmware Fusionné**
```bash
./create_merged_firmware.sh
```
→ Fusionne bootloader + partitions + firmware
→ Crée `releases/v1.0.2/OpenSailingRC_Display_v1.0.2_MERGED.bin`

### **Étape 3 : Tester**
```bash
python3 -m esptool --chip esp32 --port PORT write_flash 0x0 MERGED.bin
```
→ Vérifie que le Core2 démarre correctement

### **Étape 4 : Distribuer**
```bash
# Créer l'archive release
zip OpenSailingRC-Display-v1.0.2.zip releases/v1.0.2/*
```
→ Uploader sur GitHub Releases

---

## 🆘 Dépannage

### **❌ Core2 Reboot en Boucle Après Flash M5Burner**

**Cause probable** : Adresse incorrecte (0x10000 au lieu de 0x0)

**Solution** :
1. Vérifier que l'adresse dans M5Burner est **0x0**
2. Cocher "Erase Flash"
3. Reflasher

### **❌ "invalid header: 0x2068746f" dans les Logs**

**Cause** : Bootloader manquant ou corrompu

**Solution** :
```bash
# Effacer complètement la flash
python3 -m esptool --chip esp32 --port PORT erase_flash

# Reflasher le firmware fusionné
python3 -m esptool --chip esp32 --port PORT write_flash 0x0 MERGED.bin
```

### **❌ M5Burner n'Accepte pas le Fichier**

**Cause** : Fichier trop gros (>2 MB) ou format incorrect

**Vérifications** :
```bash
# Vérifier la taille
ls -lh OpenSailingRC_Display_v1.0.2_MERGED.bin
# Doit être ~1,1 MB

# Vérifier le format
file OpenSailingRC_Display_v1.0.2_MERGED.bin
# Doit être "data" ou "DOS executable"
```

---

## 💡 Notes Techniques

### **Pourquoi 0x0 et non 0x10000 ?**

Le firmware fusionné contient **toute la structure mémoire** :
```
0x0000  : Header/Padding
0x1000  : Bootloader ← L'ESP32 commence ici
0x8000  : Partitions
0x10000 : Application
```

Si on flash à 0x10000, on saute le bootloader → **reboot infini**.

### **Espace Mémoire Utilisé**

| Section | Adresse | Taille | Contenu |
|---------|---------|--------|---------|
| Bootstrap | 0x0 - 0x1000 | 4 KB | Padding |
| Bootloader | 0x1000 - 0x8000 | 28 KB | Init ESP32 |
| Partitions | 0x8000 - 0x10000 | 32 KB | Table partitions |
| Application | 0x10000 - 0x111A80 | 1 MB | Firmware |
| **Total** | **0x0 - 0x111A80** | **~1,1 MB** | **Firmware complet** |

---

## 🚀 Prochaines Étapes

### **V1.0.2 Release Package**
- [x] Créer firmware fusionné
- [x] Tester flash à 0x0
- [x] Créer instructions M5Burner
- [ ] Créer archive ZIP complète
- [ ] Uploader sur GitHub Releases
- [ ] Mettre à jour documentation README
- [ ] Créer vidéo démo M5Burner

### **Améliorations Futures**
- [ ] Créer un configurateur web pour WiFi credentials
- [ ] OTA (Over-The-Air) updates via WiFi
- [ ] Interface M5Burner personnalisée
- [ ] QR code pour download rapide

---

## 📚 Références

- **Fichiers** : `create_merged_firmware.sh`, `releases/v1.0.2/`
- **ESPTool Docs** : https://docs.espressif.com/projects/esptool/
- **M5Burner** : https://docs.m5stack.com/en/download
- **ESP32 Memory Map** : https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/general-notes.html
- **Version** : 1.0.2
- **Auteur** : Philippe Hubert
- **Date** : 5 octobre 2025

---

*Solution testée et validée sur M5Stack Core2 v1.1*  
*Le firmware fusionné permet une installation simple pour tous les utilisateurs* 🎉