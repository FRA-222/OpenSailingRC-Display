# OpenSailingRC Display - Release V1.0.1

## 🚀 Nouvelle Version Firmware

### 📋 **Résumé des améliorations**

Cette version apporte des améliorations significatives de stabilité et de robustesse au système d'affichage OpenSailingRC.

### ✨ **Nouvelles fonctionnalités**

#### **Gestion robuste des erreurs SD**
- ✅ **Affichage d'erreur visuel** : Message d'erreur sur écran si carte SD manquante/défaillante
- ✅ **Fonctionnement sans SD** : Le système continue de fonctionner même sans carte SD
- ✅ **Réinitialisation interactive** : Toucher l'écran pour réessayer l'initialisation SD
- ✅ **Pas de plantage système** : Plus de `vTaskDelete(NULL)` qui faisait planter le M5Stack

#### **Système de boutons tactiles amélioré**
- ✅ **Debouncing efficace** : Fini les multiples déclenchements (problème des 3 appuis)
- ✅ **Interface non-bloquante** : Les messages du serveur WiFi n'interrompent plus l'interface
- ✅ **Feedback visuel stable** : Les boutons gardent leur état correct sans clignotement

#### **Compatibilité de structures étendue**
- ✅ **Support legacy complet** : Compatible avec les anciennes versions du bateau et anémomètre
- ✅ **Détection automatique** : Reconnaissance automatique du format des données reçues
- ✅ **Migration transparente** : Aucune action requise pour la compatibilité

#### **Gestion WiFi intégrée**
- ✅ **Credentials fallback** : Configuration WiFi intégrée si pas de fichier sur SD
- ✅ **Connexion robuste** : Gestion améliorée des échecs de connexion
- ✅ **Logs détaillés** : Diagnostic complet des problèmes de connexion

### 🔧 **Corrections de bugs**

#### **Stabilité générale**
- 🐛 **Correction plantage SD** : Plus de crash au démarrage si carte SD absente
- 🐛 **Fix boutons tactiles** : Résolution du problème de multi-déclenchement
- 🐛 **Stabilité serveur WiFi** : Correction du clignotement des boutons serveur
- 🐛 **Gestion mémoire** : Optimisation des tâches FreeRTOS

#### **Interface utilisateur**
- 🐛 **Affichage boutons** : État des boutons toujours correct et stable
- 🐛 **Messages temporaires** : Affichage non-bloquant des statuts serveur
- 🐛 **Refresh automatique** : Mise à jour automatique après messages système

### 📊 **Statistiques techniques**

- **Taille firmware** : 1.048.285 bytes (1,0 MB)
- **RAM utilisée** : 48.840 bytes (1,1%)
- **Flash utilisée** : 16,0% du total disponible
- **Compatibilité** : M5Stack Core2 v1.1
- **Framework** : Arduino ESP32 v3.20017

### 🔄 **Compatibilité**

#### **Matériel supporté**
- ✅ M5Stack Core2
- ✅ Carte SD (optionnelle depuis cette version)
- ✅ Connectivité WiFi
- ✅ Communication ESP-NOW

#### **Projets compatibles**
- ✅ OpenSailingRC GPS Boat (toutes versions)
- ✅ OpenSailingRC Anemometer v2 (toutes versions)
- ✅ Structures de données legacy et actuelles

### 📝 **Configuration requise**

#### **Obligatoire**
- M5Stack Core2 avec ESP32
- Alimentation USB ou batterie

#### **Optionnel**
- Carte SD pour stockage des données
- Réseau WiFi pour serveur de fichiers
- Fichier `wifi_config.json` (sinon utilise les credentials intégrés)

### 🚀 **Installation**

1. **Télécharger** le fichier `firmware.bin`
2. **Flasher** sur M5Stack Core2 via esptool ou PlatformIO
3. **Insérer** carte SD (optionnel)
4. **Configurer** WiFi si nécessaire
5. **Démarrer** le système

### 📞 **Support**

Pour tout problème ou question :
- Vérifiez les logs série pour diagnostic
- Consultez la documentation du projet
- Créez un issue sur GitHub si nécessaire

---

**Développé par Philippe Hubert - Septembre 2025**
**Licence : GNU General Public License v3.0**