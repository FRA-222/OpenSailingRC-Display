# Amélioration de la Fiabilité ESP-NOW

## Problème Observé

📊 **Symptômes :**
- ✅ BoatGPS : Séquence parfaite (1, 2, 3, 4, ...)
- ❌ Display : Séquences manquantes + doublons
- 📉 Taux de perte élevé (à quantifier avec `analyze_packets.py`)

## Utilisation du Script d'Analyse

```bash
# Analyser un fichier Display
python3 analyze_packets.py /Volumes/DISPLAY/replay/2025-11-23_04-33-59.json

# Comparer Bateau vs Display
python3 analyze_packets.py /Volumes/BOATGPS/2025-11-23_04-33-59.json /Volumes/DISPLAY/replay/2025-11-23_04-33-59.json
```

Le script affiche :
- 📦 Nombre total de paquets, doublons, séquences uniques
- 📈 Plage de séquences (min, max)
- ❌ Taux de perte détaillé
- 🔄 Liste des doublons les plus fréquents
- 🕳️ Séquences manquantes groupées
- 🔗 Correspondance Bateau ↔ Display

## Causes de Perte de Paquets

### 1. Distance et Obstacles 📡
**Caractéristiques ESP-NOW :**
- Portée théorique : ~100m en champ libre
- Portée réelle : 20-50m avec obstacles
- Sensible aux murs, eau, métal

**Solutions :**
- ✅ Réduire la distance entre BoatGPS et Display
- ✅ Éviter les obstacles métalliques
- ✅ Position antenne optimale (verticale)

### 2. Interférences WiFi 📶
**Problème :**
- ESP-NOW utilise les canaux WiFi 2.4 GHz (1-13)
- Partage le spectre avec les réseaux WiFi environnants

**Solution : Choisir le meilleur canal**

Actuellement, le canal est défini dans le code. Vérifions :

```bash
# Chercher la configuration du canal
grep -r "WiFi.channel\|esp_wifi_set_channel" OpenSailingRC-*/src/
```

**Recommandations de canal :**
- **Canal 1** : Très encombré (WiFi par défaut)
- **Canal 6** : Très encombré (WiFi par défaut)
- **Canal 11** : Très encombré (WiFi par défaut)
- **Canal 13** : Moins utilisé ✅
- **Canal 3, 4, 8, 9** : Bons compromis

**Comment choisir :**
1. Scanner les canaux WiFi environnants (app smartphone)
2. Choisir un canal peu utilisé
3. Modifier dans `Communication.cpp`

### 3. Puissance d'Émission 🔋
**Actuellement :** Vérifier la configuration

```cpp
// Dans Communication.cpp
esp_wifi_set_max_tx_power(84);  // 84 = 21 dBm (max)
```

**Niveaux disponibles :**
- `84` = 21 dBm (maximum, consommation élevée)
- `78` = 19.5 dBm
- `76` = 19 dBm
- `70` = 17.5 dBm
- `60` = 15 dBm

**Recommandation :** Utiliser le maximum (84) pour les tests

### 4. Taux de Rafraîchissement ⏱️
**Actuellement :** 1 paquet par seconde (1000 ms)

```cpp
const uint32_t BROADCAST_INTERVAL = 1000;  // 1 Hz
```

**Impact :**
- ✅ 1 Hz : Faible charge réseau, fiable
- ⚠️ 2-5 Hz : Charge modérée, risque de collisions
- ❌ >10 Hz : Surcharge, pertes importantes

**Recommandation :** Garder 1 Hz pour GPS (position change lentement)

### 5. Nombre de Retries 🔄
**Actuellement :** 2 tentatives (1 envoi + 1 retry)

```cpp
bool broadcastGPSData(const GPSData& data, uint8_t retries = 2);
```

**Options :**
- `retries = 0` : Aucun retry (1 seule tentative)
- `retries = 2` : 1 envoi + 2 retries = 3 tentatives ✅ (défaut actuel)
- `retries = 4` : 1 envoi + 4 retries = 5 tentatives (plus fiable, plus lent)

**Délai entre retries :** 10 ms

**Calcul du temps total :**
- `retries = 2` : 0 + 10 + 10 = 20 ms max
- `retries = 4` : 0 + 10 + 10 + 10 + 10 = 40 ms max

**Recommandation :** Augmenter à 4 retries si le taux de perte est > 20%

## Modifications Proposées

### Option A : Augmenter les Retries (Simple)

**Fichier :** `OpenSailingRC-BoatGPS/src/main.cpp`

```cpp
// Ancien (ligne ~238)
bool success = comm.broadcastGPSData(data);

// Nouveau
bool success = comm.broadcastGPSData(data, 4);  // 5 tentatives totales
```

**Impact :**
- ✅ Améliore la fiabilité
- ✅ Temps d'exécution : +20 ms par paquet
- ✅ Pas de changement de protocole

### Option B : Optimiser la Puissance WiFi (Avancé)

**Fichier :** `OpenSailingRC-BoatGPS/src/Communication.cpp`

Vérifier si la puissance est au maximum :

```cpp
bool Communication::begin() {
    // ... code existant ...
    
    // Définir la puissance maximale
    esp_wifi_set_max_tx_power(84);  // 21 dBm = maximum
    
    // ... suite du code ...
}
```

### Option C : Optimiser le Canal WiFi

**Fichier :** `OpenSailingRC-BoatGPS/src/Communication.cpp`

```cpp
bool Communication::begin() {
    WiFi.mode(WIFI_STA);
    
    // Choisir un canal peu encombré
    esp_wifi_set_channel(13, WIFI_SECOND_CHAN_NONE);  // Canal 13
    
    // ... suite du code ...
}
```

**⚠️ Important :** Le Display doit aussi être configuré sur le même canal !

**Fichier :** `OpenSailingRC-Display/src/main.cpp`

```cpp
void setup() {
    // ... code existant ...
    
    // Même canal que le BoatGPS
    esp_wifi_set_channel(13, WIFI_SECOND_CHAN_NONE);
    
    // ... suite du code ...
}
```

### Option D : Ajouter un Indicateur de Qualité Signal (RSSI)

Modifier la structure pour inclure le RSSI (puissance du signal reçu) :

**Avantage :** Permet de diagnostiquer les problèmes de distance/obstacles

```cpp
// Dans DisplayTypes.h
typedef struct struct_message_Boat {
    // ... champs existants ...
    int8_t rssi;  // Signal strength (dBm)
} struct_message_Boat;
```

Puis dans le callback :

```cpp
void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    // ... code existant ...
    
    // Obtenir RSSI (force du signal)
    wifi_promiscuous_pkt_t *promiscuous_pkt = (wifi_promiscuous_pkt_t*)data;
    int8_t rssi = promiscuous_pkt->rx_ctrl.rssi;
    
    logger.log("📶 RSSI: " + String(rssi) + " dBm");
}
```

**Interprétation RSSI :**
- `-30 dBm` : Excellent (très proche)
- `-50 dBm` : Très bon
- `-60 dBm` : Bon
- `-70 dBm` : Moyen
- `-80 dBm` : Faible (pertes possibles)
- `-90 dBm` : Très faible (beaucoup de pertes)

## Plan d'Action Recommandé

### Phase 1 : Diagnostic (Actuel)
1. ✅ Ajouter le filtre de doublons (fait)
2. ✅ Créer `analyze_packets.py` (fait)
3. 🔄 Analyser les fichiers JSON pour quantifier les pertes

### Phase 2 : Optimisations Simples
1. **Augmenter les retries à 4** (2 minutes de modification)
2. **Vérifier la puissance WiFi au maximum** (84)
3. **Tester et comparer** les statistiques

### Phase 3 : Optimisations Avancées (Si nécessaire)
1. **Scanner les canaux WiFi** environnants
2. **Changer le canal** pour un canal moins encombré
3. **Ajouter le monitoring RSSI** pour diagnostiquer

### Phase 4 : Solutions Hardware (Si vraiment nécessaire)
1. **Antenne externe** sur le BoatGPS
2. **Répéteur ESP-NOW** (un ESP32 intermédiaire)
3. **Changer de protocole** (LoRa pour longue distance)

## Tests à Effectuer

### Test 1 : Baseline (Situation actuelle)
```bash
# 1. Flasher le Display avec le filtre de doublons
# 2. Enregistrer 5 minutes
# 3. Analyser
python3 analyze_packets.py /Volumes/BOATGPS/test1.json /Volumes/DISPLAY/replay/test1.json
```

**Objectif :** Établir le taux de perte de référence

### Test 2 : Avec Plus de Retries
```bash
# 1. Modifier main.cpp : broadcastGPSData(data, 4)
# 2. Flasher le BoatGPS
# 3. Enregistrer 5 minutes
# 4. Analyser
python3 analyze_packets.py /Volumes/BOATGPS/test2.json /Volumes/DISPLAY/replay/test2.json
```

**Objectif :** Mesurer l'amélioration

### Test 3 : Distance
```bash
# Tester à différentes distances :
# - 5m, 10m, 20m, 50m, 100m
# Analyser les taux de perte pour chaque distance
```

**Objectif :** Déterminer la portée effective

## Résultats Attendus

### Taux de Perte "Normal" pour ESP-NOW

| Distance | Sans Obstacles | Avec Obstacles |
|----------|----------------|----------------|
| 5m       | < 1%           | < 5%           |
| 10m      | < 2%           | < 10%          |
| 20m      | < 5%           | < 20%          |
| 50m      | < 15%          | < 50%          |
| 100m     | < 50%          | N/A            |

**Si vos taux sont plus élevés :**
- ❌ Problème de configuration
- ❌ Interférences importantes
- ❌ Hardware défectueux
- ❌ Obstacles majeurs (métal, eau)

### Doublons Attendus

**Après le filtre :**
- ✅ **0 doublon** dans le fichier SD du Display
- ⚠️ Messages dans les logs : "Paquet doublon détecté"

**Si vous voyez encore des doublons dans le fichier :**
- ❌ Le filtre n'est pas activé
- ❌ Firmware pas à jour

## Commandes Utiles

```bash
# Analyser un fichier Display
./analyze_packets.py /Volumes/DISPLAY/replay/2025-11-23_04-33-59.json

# Comparer Bateau vs Display
./analyze_packets.py /Volumes/BOATGPS/2025-11-23_04-33-59.json /Volumes/DISPLAY/replay/2025-11-23_04-33-59.json

# Trouver les fichiers les plus récents
ls -lht /Volumes/DISPLAY/replay/*.json | head -5
ls -lht /Volumes/BOATGPS/*.json | head -5

# Compter rapidement les paquets
wc -l /Volumes/DISPLAY/replay/2025-11-23_04-33-59.json
```

## Conclusion

**Priorités :**
1. ✅ **Filtre de doublons** : Implémenté
2. 🔄 **Analyse des pertes** : Utiliser `analyze_packets.py`
3. ⭐ **Augmenter les retries** : Passer de 2 à 4 (recommandé)
4. 🔍 **Monitoring RSSI** : Ajout optionnel pour diagnostic

**Objectif réaliste :**
- Taux de perte < 10% à 10-20m en intérieur
- Taux de perte < 5% à 5m en extérieur
- 0 doublon dans les fichiers SD
