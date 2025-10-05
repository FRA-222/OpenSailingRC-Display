# 🔥 Guide d'Installation M5Burner - OpenSailingRC Display V1.0.1

## 📖 Guide Illustré Complet

Ce guide vous accompagne étape par étape pour installer le firmware OpenSailingRC Display V1.0.1 sur votre M5Stack Core2 en utilisant M5Burner.

---

## 🚀 **Étape 1 : Téléchargement et Installation de M5Burner**

### **1.1 Téléchargement**

1. **Rendez-vous** sur le site officiel M5Stack :
   ```
   🌐 URL : https://docs.m5stack.com/en/download
   📦 Fichier : M5Burner (Windows/Mac/Linux)
   ```

2. **Téléchargez** la version correspondant à votre système :
   - **Windows** : `M5Burner-v3.x.x-win.zip`
   - **macOS** : `M5Burner-v3.x.x-mac.zip`
   - **Linux** : `M5Burner-v3.x.x-linux.zip`

### **1.2 Installation**

#### **Windows :**
```bash
# Décompresser l'archive
# Double-cliquer sur M5Burner.exe
# Aucune installation requise
```

#### **macOS :**
```bash
# Décompresser l'archive
# Glisser M5Burner.app vers Applications
# Accepter les permissions de sécurité si demandées
```

#### **Linux :**
```bash
# Décompresser l'archive
unzip M5Burner-v3.x.x-linux.zip
cd M5Burner
chmod +x M5Burner
./M5Burner
```

---

## 🔌 **Étape 2 : Préparation du Matériel**

### **2.1 Connexion du M5Stack Core2**

```
📱 M5Stack Core2
    ↓ (Câble USB-C)
💻 Ordinateur
```

**Points importants :**
- ✅ Utilisez un câble USB-C **de données** (pas seulement charge)
- ✅ Connectez directement (évitez les hubs USB si possible)
- ✅ Le M5Stack peut être allumé ou éteint

### **2.2 Identification du Port Série**

#### **Windows :**
1. **Gestionnaire de périphériques** → **Ports (COM et LPT)**
2. **Recherchez** : `Silicon Labs CP210x USB to UART Bridge (COMx)`
3. **Notez** le numéro : `COM3`, `COM4`, etc.

#### **macOS :**
```bash
# Dans Terminal
ls /dev/cu.usbserial*
# Résultat : /dev/cu.usbserial-XXXXXXXX
```

#### **Linux :**
```bash
# Dans Terminal
ls /dev/ttyUSB* /dev/ttyACM*
# Résultat : /dev/ttyUSB0 ou /dev/ttyACM0
```

---

## 🔥 **Étape 3 : Utilisation de M5Burner**

### **3.1 Lancement de M5Burner**

**Interface M5Burner :**
```
┌─────────────────────────────────────────────────────────────┐
│ M5Burner v3.x.x                                     [─][□][×]│
├─────────────────────────────────────────────────────────────┤
│ [DOWNLOAD] [BURN] [COM] [CONFIG]                            │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  🔹 Official Firmware                                       │
│  📦 M5Stack Core2 - Basic                                   │
│  📦 M5Stack Core2 - Factory                                │
│  📦 M5Stack Core2 - Demo                                    │
│                                                             │
│  🔸 Custom Firmware                                         │
│  📁 Browse for .bin file...                                │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### **3.2 Configuration Initiale**

1. **Cliquez** sur l'onglet **"BURN"** (en haut)
2. **Dans la section "Custom Firmware"** → Cliquez **"Browse"**

**Sélection du Firmware :**
```
📁 Naviguez vers : releases/v1.0.1/
📄 Sélectionnez : OpenSailingRC_Display_v1.0.1.bin
✅ Cliquez : "Ouvrir"
```

### **3.3 Configuration des Paramètres**

**Interface de Configuration :**
```
┌─── Flash Configuration ─────────────────────────────────────┐
│                                                             │
│ 📁 Firmware File: OpenSailingRC_Display_v1.0.1.bin        │
│ 🔌 COM Port:     [COM3          ▼]  [Refresh]              │
│ ⚡ Baud Rate:     [921600        ▼]                        │
│ 📍 Address:      [0x10000      ]                           │
│                                                             │
│ Advanced Settings:                                          │
│ ☑️ Erase Flash   ☑️ Verify   📊 Flash Size: 16MB          │
│                                                             │
│ [                BURN                ]                      │
└─────────────────────────────────────────────────────────────┘
```

**Paramètres Recommandés :**
- **✅ COM Port** : Sélectionnez votre port (ex: COM3, /dev/cu.usbserial-xxx)
- **✅ Baud Rate** : `921600` (rapide) ou `115200` (stable)
- **✅ Address** : `0x10000` (adresse firmware)
- **✅ Erase Flash** : Coché (installation propre)
- **✅ Verify** : Coché (vérification après flash)

---

## ⚡ **Étape 4 : Processus de Flashage**

### **4.1 Démarrage du Flash**

1. **Vérifiez** que tous les paramètres sont corrects
2. **Cliquez** sur le bouton **"BURN"** (grand bouton vert)

### **4.2 Progression du Flash**

**Écran de Progression :**
```
┌─── Flashing Progress ───────────────────────────────────────┐
│                                                             │
│ 🔄 Connecting to device...                    [    5%]     │
│ ▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░              │
│                                                             │
│ Status: Erasing flash...                                    │
│ ⏱️  Estimated time: 45 seconds                              │
│                                                             │
│ [              CANCEL              ]                        │
└─────────────────────────────────────────────────────────────┘
```

**Phases du Processus :**
1. **🔄 Connecting** (5-10 sec) : Connexion au M5Stack
2. **🧹 Erasing** (10-20 sec) : Effacement de la flash
3. **📤 Writing** (20-40 sec) : Écriture du firmware
4. **✅ Verifying** (5-10 sec) : Vérification des données

### **4.3 Finalisation**

**Écran de Succès :**
```
┌─── Flash Complete ──────────────────────────────────────────┐
│                                                             │
│                    ✅ FLASH SUCCESS!                       │
│                                                             │
│ 📊 Firmware: OpenSailingRC_Display_v1.0.1.bin             │
│ 💾 Size: 1,048,285 bytes                                   │
│ ⏱️  Time: 42 seconds                                        │
│ 🔌 Port: COM3                                              │
│                                                             │
│ Your M5Stack Core2 is ready to use!                        │
│                                                             │
│ [               OK               ]                          │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎯 **Étape 5 : Vérification et Premier Démarrage**

### **5.1 Redémarrage du M5Stack**

1. **Débranchez** le câble USB-C
2. **Attendez** 2 secondes
3. **Rebranchez** ou **appuyez** sur le bouton power

### **5.2 Écran de Démarrage Attendu**

**Splash Screen (2 secondes) :**
```
┌─────────────────────────────────────┐
│                                     │
│            🌊 WIND 🌊               │
│                                     │
│            ⛵ BOAT ⛵               │
│                                     │
└─────────────────────────────────────┘
```

**Interface Principale :**
```
┌─────────────────────────────────────┐
│ BOAT    0.0 KTS    📡 0 SAT        │
│                                     │
│         000 DEG                     │
│                                     │
│ BUOY    0.0 KTS                     │
│                                     │
├─────────┬─────────┬─────────────────┤
│  STOP   │         │      WIFI       │
│  (RED)  │ (GREY)  │     (RED)       │
└─────────┴─────────┴─────────────────┘
```

### **5.3 Message d'Erreur SD (Normal)**

Si pas de carte SD insérée :
```
┌─────────────────────────────────────┐
│                                     │
│        🚫 ERREUR SD 🚫             │
│                                     │
│     Carte SD non détectée          │
│                                     │
│   💡 Toucher écran pour réessayer  │
│                                     │
└─────────────────────────────────────┘
```

**✅ C'est normal !** Le système fonctionne sans carte SD.

---

## 🆘 **Dépannage M5Burner**

### **❌ Problème 1 : Port Non Détecté**

**Symptômes :**
- Liste des ports vide
- "No device detected"

**Solutions :**
1. **Installer le driver USB :**
   - **Windows** : Driver CP210x de Silicon Labs
   - **macOS** : Généralement inclus
   - **Linux** : `sudo apt install python3-serial`

2. **Vérifier le câble :**
   - Tester avec un autre câble USB-C
   - S'assurer que c'est un câble de données

3. **Redémarrer M5Burner** et refresh la liste des ports

### **❌ Problème 2 : Flash Failed**

**Symptômes :**
- "Flash failed at xxx%"
- "Timeout error"

**Solutions :**
1. **Réduire la vitesse :**
   ```
   Baud Rate: 921600 → 115200
   ```

2. **Mode boot manuel :**
   - Maintenir **bouton Reset** du M5Stack
   - Cliquer **BURN** dans M5Burner
   - Relâcher **Reset** après "Connecting..."

3. **Effacement complet :**
   ```
   ☑️ Erase Flash (cocher)
   Address: 0x0 (au lieu de 0x10000)
   ```

### **❌ Problème 3 : M5Stack ne Démarre Pas**

**Symptômes :**
- Écran noir après flash
- Pas de splash screen

**Solutions :**
1. **Flash complet avec bootloader :**
   ```
   Files à flasher :
   0x1000  → bootloader.bin
   0x8000  → partitions.bin
   0x10000 → OpenSailingRC_Display_v1.0.1.bin
   ```

2. **Reset factory :**
   - Maintenir **boutons A+C** pendant 10 sec
   - Relâcher et redémarrer

### **❌ Problème 4 : Erreur "invalid header: 0x2068746f"**

**Symptômes :**
- Messages en boucle : `invalid header: 0x2068746f`
- M5Stack ne démarre pas après flash M5Burner
- Port série affiche des caractères corrompus

**Cause :**
M5Burner n'a flashé que le firmware principal (0x10000) mais pas le **bootloader** (0x1000) ni les **partitions** (0x8000).

**Solution ESPTool (Recommandée) :**
1. **Installer esptool :**
   ```bash
   pip3 install esptool
   ```

2. **Effacer complètement la flash :**
   ```bash
   python3 -m esptool --chip esp32 --port /dev/cu.usbserial-XXXXXXXX --baud 115200 erase_flash
   ```

3. **Flash système complet :**
   ```bash
   python3 -m esptool --chip esp32 --port /dev/cu.usbserial-XXXXXXXX --baud 115200 \
     --before default_reset --after hard_reset write_flash -z \
     --flash_mode dio --flash_freq 80m --flash_size 16MB \
     0x1000 bootloader.bin \
     0x8000 partitions.bin \
     0x10000 OpenSailingRC_Display_v1.0.1.bin
   ```

**✅ Résultat :** Le M5Stack devrait maintenant démarrer correctement avec le splash "WIND/BOAT".

### **❌ Problème 5 : Permissions macOS**

**Symptômes :**
- "App cannot be opened"
- Blocage sécurité macOS

**Solutions :**
1. **Autoriser dans Préférences Système :**
   ```
   Préférences → Sécurité → Général
   → "Autoriser M5Burner"
   ```

2. **Terminal alternative :**
   ```bash
   sudo xattr -rd com.apple.quarantine M5Burner.app
   ```

---

## 📋 **Checklist de Vérification**

### **Avant le Flash :**
- [ ] M5Burner téléchargé et lancé
- [ ] M5Stack Core2 connecté via USB-C
- [ ] Port série détecté dans M5Burner
- [ ] Firmware .bin sélectionné
- [ ] Paramètres configurés (921600, 0x10000, Erase coché)

### **Pendant le Flash :**
- [ ] Progression sans erreur
- [ ] Pas de déconnexion du câble
- [ ] M5Stack reste connecté

### **Après le Flash :**
- [ ] "Flash Success" affiché
- [ ] M5Stack redémarre automatiquement
- [ ] Splash screen "WIND/BOAT" visible
- [ ] Interface principale fonctionnelle

### **Fonctionnalités à Tester :**
- [ ] Boutons tactiles répondent
- [ ] Message SD normal (si pas de carte)
- [ ] Pas de plantage système
- [ ] Logs série corrects (115200 baud)

---

## 🎯 **Résumé Rapide - 5 Minutes Chrono**

```
1. 📥 Télécharger M5Burner → Lancer
2. 🔌 Connecter M5Stack → Vérifier port
3. 📁 BURN → Custom Firmware → Browse → .bin
4. ⚙️  Port + 921600 + 0x10000 + Erase ✓
5. 🔥 BURN → Attendre → Flash Success ✅
```

**Votre M5Stack Core2 est maintenant équipé du firmware OpenSailingRC Display V1.0.1 !** 🎉

---

*Guide créé pour OpenSailingRC Display V1.0.1 - Septembre 2025*
*M5Burner compatible toutes versions 3.x.x*