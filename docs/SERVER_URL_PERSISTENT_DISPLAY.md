# 🔧 Correctif Affichage Permanent URL Serveur - OpenSailingRC Display

## 📅 Date : 5 octobre 2025

## 🔍 Problème Identifié

### **Symptômes :**
- L'URL du serveur web s'affiche brièvement après activation
- L'URL disparaît rapidement, remplacée par l'interface BOAT/BUOY
- Impossible de lire l'adresse IP pour se connecter au serveur

### **Cause Racine :**
L'interface était **rafraîchie en continu** même quand le serveur était actif, écrasant le message avec l'URL :

```cpp
// ❌ ANCIEN CODE - Rafraîchit toujours
if (newData) {
    display.drawDisplay(...); // Efface l'URL !
}

// Refresh périodique
if (millis() - lastPeriodicRefresh > 1000) {
    display.drawDisplay(...); // Efface l'URL !
}

// Après message serveur
if (display.needsRefresh()) {
    display.drawDisplay(...); // Efface l'URL !
}
```

**Résultat :** L'URL apparaît pendant ~2 secondes puis est effacée par le prochain rafraîchissement.

---

## ✅ Solution Implémentée

### **Suspension de l'Affichage Quand Serveur Actif**

L'interface BOAT/BUOY n'est **plus rafraîchie** quand le serveur de fichiers est actif :

```cpp
// ✅ NOUVEAU CODE - Suspend l'affichage si serveur actif
if (!fileServer.isServerActive()) {
    // Affichage normal BOAT/BUOY
    if (newData) {
        display.drawDisplay(...);
        newData = false;
    }
    
    // Refresh périodique
    if (millis() - lastPeriodicRefresh > 1000) {
        display.drawDisplay(...);
    }
} else {
    // Serveur actif : consommer les données mais ne pas afficher
    if (newData) {
        newData = false;
        logger.log("Données reçues mais affichage suspendu (serveur actif)");
    }
}

// Refresh après message serveur uniquement si serveur INACTIF
if (display.needsRefresh() && !fileServer.isServerActive()) {
    display.drawDisplay(...);
}
```

---

## 🎯 Comportement Attendu

### **Scénario 1 : Activation du Serveur**
```
1. Utilisateur appuie sur bouton WiFi (droite)
2. Serveur démarre et se connecte au WiFi
3. showFileServerStatus(true, IP) affiche l'URL
4. L'URL RESTE AFFICHÉE en permanence ✅
5. Les données BOAT/BUOY continuent d'arriver mais ne s'affichent pas
```

### **Scénario 2 : Utilisation du Serveur**
```
1. URL visible à l'écran: http://192.168.68.79
2. Utilisateur accède aux fichiers depuis son navigateur
3. L'écran continue d'afficher l'URL (pas de rafraîchissement)
4. Le serveur reste actif et accessible
```

### **Scénario 3 : Désactivation du Serveur**
```
1. Utilisateur appuie à nouveau sur bouton WiFi
2. Serveur s'arrête et se déconnecte du WiFi
3. showFileServerStatus(false, "") efface le message
4. L'affichage BOAT/BUOY reprend normalement ✅
5. ESP-NOW est réinitialisé
```

---

## 📊 Modifications Apportées

### **src/main.cpp - Ligne ~747-778**

**Avant :**
```cpp
if (newData) {
    display.drawDisplay(...);
    newData = false;
}

if (millis() - lastPeriodicRefresh > 1000) {
    display.drawDisplay(...);
}
```

**Après :**
```cpp
if (!fileServer.isServerActive()) {
    if (newData) {
        display.drawDisplay(...);
        newData = false;
    }
    
    if (millis() - lastPeriodicRefresh > 1000) {
        display.drawDisplay(...);
    }
} else {
    if (newData) {
        newData = false;
        logger.log("Données reçues mais affichage suspendu");
    }
}
```

### **src/main.cpp - Ligne ~741-745**

**Avant :**
```cpp
if (display.needsRefresh()) {
    display.drawDisplay(...);
}
```

**Après :**
```cpp
if (display.needsRefresh() && !fileServer.isServerActive()) {
    display.drawDisplay(...);
}
```

---

## 🔄 Cycle de Vie de l'Affichage

```
┌─────────────────────────────────────────────────┐
│          ÉTAT: SERVEUR INACTIF                  │
│  • Affichage BOAT/BUOY actif                    │
│  • Rafraîchissement toutes les secondes          │
│  • Mise à jour avec nouvelles données ESP-NOW   │
└─────────────────────────────────────────────────┘
                      ↓ Appui bouton WiFi
┌─────────────────────────────────────────────────┐
│          ÉTAT: SERVEUR ACTIF                    │
│  • URL affichée en permanence                   │
│  • Aucun rafraîchissement d'écran               │
│  • Données ESP-NOW reçues mais non affichées    │
│  • Serveur web accessible                       │
└─────────────────────────────────────────────────┘
                      ↓ Appui bouton WiFi
┌─────────────────────────────────────────────────┐
│          ÉTAT: SERVEUR INACTIF                  │
│  • Retour à l'affichage BOAT/BUOY               │
│  • Rafraîchissements reprennent                 │
│  • ESP-NOW réinitialisé                         │
└─────────────────────────────────────────────────┘
```

---

## 🧪 Tests de Validation

### **Test 1 : Affichage Permanent URL**
```
✅ Activer serveur WiFi
   → URL s'affiche
   → Attendre 10 secondes
   → URL toujours visible ✓
```

### **Test 2 : Données en Arrière-Plan**
```
✅ Serveur actif avec URL affichée
   → Bateau envoie données GPS
   → Logs: "Données reçues mais affichage suspendu"
   → URL reste affichée (pas d'écrasement) ✓
```

### **Test 3 : Reprise Affichage**
```
✅ Désactiver serveur WiFi
   → URL disparaît
   → Interface BOAT/BUOY revient immédiatement
   → Données GPS affichées normalement ✓
```

### **Test 4 : Accès Serveur Web**
```
✅ Noter l'IP affichée: http://192.168.68.79
   → Ouvrir dans navigateur
   → Liste des fichiers accessible
   → Téléchargement de logs fonctionnel ✓
```

---

## 💡 Avantages de Cette Approche

| Aspect | Bénéfice |
|--------|----------|
| **UX** | L'utilisateur a le temps de noter l'URL |
| **Stabilité** | Pas de clignotement ou rafraîchissement |
| **Clarté** | Message clair et persistant |
| **Performance** | Économie CPU (pas de drawDisplay() inutile) |
| **Données** | Les données continuent d'arriver en arrière-plan |

---

## 🔧 Comportements Conservés

✅ **ESP-NOW** : Continue de recevoir les données même si non affichées
✅ **Stockage SD** : Les données peuvent toujours être enregistrées
✅ **Boutons** : Tous les boutons restent fonctionnels
✅ **Serveur Web** : Gère les requêtes HTTP normalement
✅ **RTC** : Synchronisation NTP fonctionne au démarrage du serveur

---

## 📊 Métriques

| Métrique | Valeur |
|----------|--------|
| **Taille Firmware** | 1.048.765 bytes |
| **Flash Utilisée** | 16.0% |
| **RAM Utilisée** | 1.1% (48.848 bytes) |
| **Lignes Modifiées** | ~35 lignes |
| **Conditions Ajoutées** | 3 vérifications `!fileServer.isServerActive()` |

---

## 🚀 Prochaines Améliorations Possibles

### **V1.1 - QR Code**
```cpp
// Afficher un QR code pour accès rapide à l'URL
display.showQRCode(fileServer.getServerIP());
```

### **V1.2 - Indicateur de Connexion**
```cpp
// Afficher le nombre de clients connectés
display.showServerStats(fileServer.getClientCount());
```

### **V1.3 - Timeout Auto**
```cpp
// Arrêt automatique du serveur après X minutes d'inactivité
if (fileServer.getIdleTime() > 300000) {
    fileServer.stopFileServer();
}
```

---

## 📝 Notes Importantes

⚠️ **Données Non Affichées** : Quand le serveur est actif, les données GPS/BUOY continuent d'arriver mais ne sont **pas affichées**. Elles sont simplement ignorées pour l'affichage.

⚠️ **Logs Série** : Les logs continuent de fonctionner normalement, vous pouvez voir les données arriver dans le moniteur série.

✅ **Transparent pour l'Utilisateur** : L'utilisateur sait que quand l'URL est affichée, c'est le mode "serveur de fichiers", et quand c'est BOAT/BUOY, c'est le mode normal.

---

## 📚 Références

- **Fichier Source** : `src/main.cpp`
- **Fonctions Modifiées** : `loop()` - sections newData, refresh périodique, needsRefresh
- **Commit** : Suspension affichage BOAT/BUOY quand serveur actif
- **Version Firmware** : 1.0.2-dev
- **Auteur** : Philippe Hubert
- **Date** : 5 octobre 2025

---

*Correctif testé et validé sur M5Stack Core2 v1.1*
*L'URL du serveur web reste maintenant visible en permanence jusqu'à désactivation*