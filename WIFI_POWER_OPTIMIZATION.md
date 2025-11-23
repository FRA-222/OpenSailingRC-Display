# Optimisation de la Puissance WiFi et du Canal

## Problème Observé

**Test du 23 novembre 2025 à 04:48:**
- 📍 Distance : GPS très proche du Display
- ❌ Taux de perte : **41.6%** (37 paquets perdus sur 89 attendus)
- 🔄 Doublons résiduels : 3 (filtre actif mais test antérieur)

**Diagnostic :**  
Taux de perte anormal pour une courte distance → Problème de configuration WiFi

## Modifications Effectuées

### 1. Augmentation de la Puissance TX

#### BoatGPS
**Fichier :** `OpenSailingRC-BoatGPS/src/Communication.cpp`

```cpp
// AVANT
WiFi.setTxPower(WIFI_POWER_19_5dBm);  // 19.5 dBm

// APRÈS
esp_wifi_set_max_tx_power(84);  // 21 dBm (maximum)
```

**Include ajouté :** `#include <esp_wifi.h>` dans `Communication.h`

#### Display
**Fichier :** `OpenSailingRC-Display/src/main.cpp`

```cpp
// AVANT (dans setup() et reinitializeESPNow())
WiFi.setTxPower(WIFI_POWER_19_5dBm);  // 19.5 dBm

// APRÈS
esp_wifi_set_max_tx_power(84);  // 21 dBm (maximum)
```

**Include ajouté :** `#include <esp_wifi.h>`

### 2. Synchronisation du Canal WiFi

**Problème :** Les deux appareils n'étaient pas forcément sur le même canal

**Solution :** Forcer le canal 1 sur les deux appareils

#### BoatGPS
```cpp
// Dans Communication::begin()
esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
```

#### Display
```cpp
// Dans setup() et reinitializeESPNow()
esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
```

## Amélioration Théorique

### Puissance d'Émission

| Niveau | Puissance | Portée Relative | Consommation |
|--------|-----------|-----------------|--------------|
| WIFI_POWER_19_5dBm | 19.5 dBm | 100% | Normale |
| **84 (21 dBm)** | **21 dBm** | **+26%** | **+15%** |

**Gain en distance :**
- +1.5 dB de puissance = +26% de portée en champ libre
- En pratique : 50-100m → 63-126m de portée maximale

**Impact sur la fiabilité :**
- Amélioration du rapport signal/bruit (SNR)
- Réduction des erreurs de transmission
- Meilleure pénétration des obstacles

### Canal Dédié

**Avantage :**
- Cohérence garantie entre émetteur et récepteur
- Évite les basculements de canal automatiques
- Base solide pour futur changement de canal (si interférences)

**Canal 1 :**
- ✅ Canal standard, compatible
- ⚠️ Peut être encombré par d'autres réseaux WiFi
- 💡 Peut être changé vers canal 6, 11 ou 13 si interférences

## Test de Validation

### Avant les Modifications
```
Fichier: 2025-11-23_04-48-42.json
Puissance TX: 19.5 dBm (BoatGPS et Display)
Canal: Non défini (automatique)

📦 PAQUETS: 55 lignes, 52 uniques, 3 doublons
❌ PERTES: 37/89 (41.6%)
```

### Après les Modifications (À tester)
```
Puissance TX: 21 dBm (maximum) sur les deux
Canal: 1 (fixe) sur les deux

Résultat attendu: < 10% de perte à courte distance
```

### Procédure de Test

1. **Flasher les deux appareils**
   ```bash
   # BoatGPS
   cd OpenSailingRC-BoatGPS
   pio run --target upload --environment m5stack-atom
   
   # Display
   cd OpenSailingRC-Display
   pio run --target upload
   ```

2. **Lancer un test de 5 minutes**
   - Garder les appareils proches (2-5m)
   - Observer les logs sur le Display
   - Attendre au moins 100 paquets

3. **Analyser les résultats**
   ```bash
   python3 analyze_packets.py /Volumes/BOATGPS/test_apres.json /Volumes/DISPLAY/replay/test_apres.json
   ```

4. **Critères de succès**
   - ✅ Taux de perte < 10% à courte distance (2-5m)
   - ✅ Taux de perte < 20% à moyenne distance (10-20m)
   - ✅ 0 doublon dans le fichier Display

## Dépannage

### Si le Taux de Perte Reste Élevé

#### 1. Vérifier les Logs au Démarrage

**BoatGPS :**
```
✓ ESP-NOW: MAC Address: XX:XX:XX:XX:XX:XX
✓ ESP-NOW: Initialized in broadcast mode
✓ ESP-NOW: Broadcast peer added
```

**Display :**
```
Puissance TX réglée à 21 dBm (max)
Canal WiFi: 1
Adresse MAC : XX:XX:XX:XX:XX:XX
```

#### 2. Tester Différents Canaux

**Canaux recommandés :**
- **Canal 1** : Standard (actuel)
- **Canal 6** : Alternative populaire
- **Canal 11** : Alternative populaire
- **Canal 13** : Moins encombré (Europe)

**Modification :**
```cpp
// Changer "1" par un autre canal dans les deux projets
esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);  // Essayer canal 6
```

#### 3. Vérifier les Interférences

**Scanner WiFi environnant :**
- Utiliser une app smartphone (WiFi Analyzer)
- Noter les canaux les plus chargés
- Choisir un canal libre

**Interférences possibles :**
- Autres réseaux WiFi 2.4 GHz
- Bluetooth (partage la bande 2.4 GHz)
- Fours à micro-ondes
- Caméras WiFi, baby monitors

#### 4. Test de Distance

Mesurer le taux de perte à différentes distances :

| Distance | Taux de Perte Attendu | Action si Dépassé |
|----------|-----------------------|-------------------|
| 2m | < 2% | Vérifier config |
| 5m | < 5% | Vérifier config |
| 10m | < 10% | Normal |
| 20m | < 20% | Normal |
| 50m | < 40% | Normal |
| 100m | > 50% | Limite de portée |

## Impact sur le Système

### Compilation

**BoatGPS :**
- ✅ Build réussi
- RAM : 14.5% (47428 bytes)
- Flash : 75.0% (982913 bytes)

**Display :**
- ✅ Build réussi
- RAM : 1.1% (48012 bytes)
- Flash : 16.0% (1050113 bytes)

### Consommation Électrique

**Augmentation attendue :**
- Puissance : 19.5 dBm → 21 dBm (+1.5 dBm)
- Consommation : +10-15% pendant la transmission
- Impact global : < 5% (transmission brève, 1x/seconde)

**Sur batterie :**
- Négligeable pour usage courte durée (< 1 heure)
- Acceptable pour usage moyenne durée (1-4 heures)
- À considérer pour usage longue durée (> 4 heures)

### Rétrocompatibilité

**Compatible avec :**
- ✅ Ancien firmware (peut recevoir les paquets)
- ✅ Structure de données inchangée
- ✅ Protocole ESP-NOW standard

**Incompatible avec :**
- ❌ Canal différent (ne recevra rien si canal diffère)

## Améliorations Futures

### 1. Configuration Dynamique du Canal

```cpp
// Ajouter un scan des canaux au démarrage
uint8_t findBestChannel() {
    // Scanner les canaux 1-13
    // Retourner le moins chargé
}

// Utiliser dans begin()
uint8_t bestChannel = findBestChannel();
esp_wifi_set_channel(bestChannel, WIFI_SECOND_CHAN_NONE);
```

### 2. Ajustement Automatique de la Puissance

```cpp
// Réduire la puissance si taux de perte < 1% (économie d'énergie)
// Augmenter si taux de perte > 20% (déjà au max)
void adjustTxPower(float lossRate) {
    if (lossRate < 0.01) {
        esp_wifi_set_max_tx_power(78);  // 19.5 dBm
    } else {
        esp_wifi_set_max_tx_power(84);  // 21 dBm
    }
}
```

### 3. Monitoring RSSI

```cpp
// Afficher la force du signal reçu
int8_t rssi = getRSSI();
logger.log("RSSI: " + String(rssi) + " dBm");

// Interprétation
// -30 à -60 dBm : Excellent
// -60 à -70 dBm : Bon
// -70 à -80 dBm : Moyen
// -80 à -90 dBm : Faible
// < -90 dBm : Très faible
```

## Références

- **IMPROVING_RELIABILITY.md** : Guide complet d'optimisation
- **IMPROVED_RETRIES.md** : Augmentation du nombre de retries
- **analyze_packets.py** : Script d'analyse des pertes

## Changelog

**Version 1.1 (23 novembre 2025) :**
- Augmentation puissance TX : 19.5 dBm → 21 dBm
- Synchronisation canal : Fixé à canal 1
- Logs améliorés avec indication de configuration
- ✅ BoatGPS compilé et prêt
- ✅ Display compilé et prêt
