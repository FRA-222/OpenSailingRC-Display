# ⚡ M5Burner - Guide Rapide

## 🚀 Installation Express (2 minutes)

### **1. Préparation**
```
M5Burner → Custom Firmware → OpenSailingRC_Display_v1.0.1.bin
Port: AUTO ou COMx/cu.usbserial-xxx
Baud: 921600
```

### **2. Flash**
```
BURN → Attendre 45s → Flash Success ✅
```

### **3. Test**
```
Redémarrer → Splash WIND/BOAT → Interface principale
```

---

## 🔧 Paramètres Standards

| Paramètre | Valeur | Note |
|-----------|--------|------|
| **Baud Rate** | 921600 | Ou 115200 si problème |
| **Address** | 0x10000 | Adresse firmware standard |
| **Erase Flash** | ✅ Coché | Installation propre |
| **Verify** | ✅ Coché | Vérification recommandée |

---

## 🆘 Dépannage Express

| Problème | Solution Rapide |
|----------|-----------------|
| Port non détecté | Driver CP210x + Cable USB-C données |
| Flash Failed | Baud 115200 + Mode boot manuel |
| **invalid header 0x2068746f** | **ESPTool flash complet bootloader+firmware** |
| Écran noir | Reset A+C 10s + Factory reset |
| Permission macOS | Préférences → Sécurité → Autoriser |

---

## 📱 Interface Résultat

**Démarrage normal :**
```
WIND/BOAT (2s) → Interface principale
↓
BOAT 0.0 KTS | GPS 0 SAT
000 DEG
BUOY 0.0 KTS
[STOP] [    ] [WIFI]
```

**✅ Flash réussi si cette interface apparaît !**

---

*Guide Express - OpenSailingRC Display V1.0.1*