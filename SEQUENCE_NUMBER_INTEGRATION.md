# Intégration du Numéro de Séquence - Projet Display

## Modifications Apportées

### 1. Structure `struct_message_Boat` (DisplayTypes.h)

**Ajout du champ `sequenceNumber` :**
```cpp
typedef struct struct_message_Boat {
    int8_t messageType;       // 1 = Boat, 2 = Anemometer
    char name[18];            // MAC address as string
    int boatId;               // Numeric boat ID
    uint32_t sequenceNumber;  // ⭐ Nouveau champ pour détection de perte
    uint32_t gpsTimestamp;    // GPS timestamp
    float latitude;
    float longitude;
    float speed;
    float heading;
    uint8_t satellites;
} struct_message_Boat;
```

**Ordre des champs :** Le champ est placé exactement à la même position que dans `GPSBroadcastPacket` du projet BoatGPS pour assurer la compatibilité binaire.

### 2. Structure `BoatInfo` (main.cpp)

**Ajout des compteurs de statistiques :**
```cpp
typedef struct BoatInfo {
    struct_message_Boat data;
    uint8_t macAddress[6];
    unsigned long lastUpdate;
    String boatId;
    uint32_t lastSequenceNumber;  // ⭐ Dernier seq reçu
    uint32_t receivedPackets;     // ⭐ Compteur de paquets reçus
    uint32_t lostPackets;         // ⭐ Compteur de paquets perdus
} BoatInfo;
```

### 3. Détection de Perte de Paquets (main.cpp)

**Logique implémentée dans `onDataRecv()` :**

```cpp
// Détection de paquets perdus (seulement si le bateau existait déjà)
if (!isNewBoat && boat.receivedPackets > 0) {
    uint32_t expectedSeq = boat.lastSequenceNumber + 1;
    uint32_t receivedSeq = incomingBoatData.sequenceNumber;
    
    if (receivedSeq == expectedSeq) {
        // ✅ Séquence correcte
        boat.receivedPackets++;
    } else if (receivedSeq > expectedSeq) {
        // ⚠️  Perte détectée
        uint32_t lost = receivedSeq - expectedSeq;
        boat.lostPackets += lost;
        boat.receivedPackets++;
        logger.log(String("⚠️  Perte détectée! ") + String(lost) + 
                  " paquet(s) (attendu #" + String(expectedSeq) + 
                  ", reçu #" + String(receivedSeq) + ")");
    } else if (receivedSeq < boat.lastSequenceNumber) {
        // 🔄 Possible redémarrage du GPS
        if (receivedSeq < 100 && boat.lastSequenceNumber > 1000) {
            logger.log("✓ GPS redémarré (seq reset à #" + String(receivedSeq) + ")");
            boat.receivedPackets = 1;
        }
    }
}
```

**Gestion des cas particuliers :**
- ✅ **Premier paquet** : Initialisation des compteurs
- ⚠️ **Perte de paquets** : Calcul du nombre perdu et log d'alerte
- 🔄 **Redémarrage GPS** : Détection automatique (seq < 100 après seq > 1000)
- 📊 **Statistiques** : Affichage tous les 10 paquets

### 4. Stockage SD (Storage.cpp)

**Ajout du champ dans le JSON :**

```cpp
// Dans writeData()
JsonObject boat = doc.createNestedObject("boat");
boat["messageType"] = data.boatData.messageType;
boat["name"] = data.boatData.name;
boat["sequenceNumber"] = data.boatData.sequenceNumber;  // ⭐ Nouveau
boat["gpsTimestamp"] = data.boatData.gpsTimestamp;
// ... autres champs
```

**Également dans `writeDataBatch()` pour l'écriture groupée.**

**Format JSON résultant :**
```json
{
  "timestamp": 12345,
  "type": 1,
  "boat": {
    "messageType": 1,
    "name": "AA:BB:CC:DD:EE:FF",
    "sequenceNumber": 123,
    "gpsTimestamp": 12345,
    "latitude": 48.123456,
    "longitude": -4.123456,
    "speed": 5.2,
    "heading": 45.0,
    "satellites": 12,
    "boatId": 255
  }
}
```

### 5. Logs de Debug (main.cpp)

**Ajout de l'offset du champ dans les logs de démarrage :**
```cpp
logger.log(String("Offset sequenceNumber: ") + 
          String(offsetof(struct_message_Boat, sequenceNumber)));
```

**Ajout dans les logs de réception :**
```cpp
logger.log(String("Sequence Number: ") + String(incomingBoatData.sequenceNumber));
```

## Affichages dans les Logs

### Au Démarrage
```
--- OFFSETS BATEAU ACTUEL ---
Offset messageType: 0
Offset sequenceNumber: 22
Offset gpsTimestamp: 26
Offset latitude: 30
...
```

### Réception d'un Paquet
```
=== DONNÉES BATEAU REÇUES ===
Message Type: 1
Boat ID: 255
Sequence Number: 123
GPS Timestamp: 12345
Latitude: 48.123456
...
```

### Premier Paquet
```
✓ Nouveau bateau détecté! Total: 1
✓ Premier paquet du bateau (seq #1)
```

### Perte Détectée
```
⚠️  Perte détectée! 2 paquet(s) (attendu #5, reçu #7)
```

### Statistiques Périodiques (tous les 10 paquets)
```
📊 Stats bateau 1: Reçus=10, Perdus=0 (0.0%)
📊 Stats bateau 1: Reçus=20, Perdus=2 (9.1%)
```

### Redémarrage GPS
```
✓ GPS redémarré (seq reset à #1)
```

## Calcul du Taux de Perte

```cpp
float lossRate = 100.0f * boat.lostPackets / (boat.receivedPackets + boat.lostPackets);
```

**Exemple :**
- Reçus : 98 paquets
- Perdus : 2 paquets
- Total envoyés : 100
- Taux de perte : 2.0%

## Vérification de la Compatibilité

### Taille des Structures

**BoatGPS (émetteur) :**
```
sizeof(GPSBroadcastPacket) = ? bytes
```

**Display (récepteur) :**
```
sizeof(struct_message_Boat) = ? bytes
```

⚠️ **Important :** Les deux structures doivent avoir **exactement la même taille** et le **même alignement** des champs.

### Test de Compatibilité

Au premier lancement, vérifier les logs :
```
Taille reçue: X bytes
Taille structure: X bytes
```

Si les tailles diffèrent :
```
*** TAILLE INATTENDUE ***
Reçu: X bytes
Attendu: Y bytes
```

## Utilisation pour l'Analyse

### Analyse en Temps Réel

Les logs permettent de :
1. ✅ Voir immédiatement les pertes de paquets
2. 📊 Suivre le taux de perte par bateau
3. 🔍 Identifier les zones de mauvaise réception
4. ⚡ Détecter les problèmes de performance

### Analyse Post-Session (Fichiers JSON)

Les fichiers JSON sur la carte SD contiennent toutes les données nécessaires :

```python
# Script Python pour analyser les pertes
import json

def analyze_session(filename):
    sequences = {}  # Par bateau (name)
    
    with open(filename, 'r') as f:
        for line in f:
            data = json.loads(line)
            if data['type'] == 1:  # Boat data
                name = data['boat']['name']
                seq = data['boat']['sequenceNumber']
                
                if name not in sequences:
                    sequences[name] = []
                sequences[name].append(seq)
    
    # Analyser les pertes par bateau
    for name, seqs in sequences.items():
        seqs.sort()
        expected = list(range(seqs[0], seqs[-1] + 1))
        lost = set(expected) - set(seqs)
        
        print(f"\nBateau {name}:")
        print(f"  Total envoyés: {seqs[-1]}")
        print(f"  Total reçus: {len(seqs)}")
        print(f"  Total perdus: {len(lost)}")
        print(f"  Taux de perte: {100 * len(lost) / seqs[-1]:.2f}%")
        
        if lost:
            print(f"  Paquets perdus: {sorted(lost)}")
```

## Tests Recommandés

### 1. Test de Base
- [x] Compilation BoatGPS ✅
- [x] Compilation Display ✅
- [ ] Upload BoatGPS
- [ ] Upload Display
- [ ] Vérifier réception du `sequenceNumber` dans les logs
- [ ] Vérifier présence dans les fichiers JSON

### 2. Test de Perte
- [ ] Éloigner le GPS du Display pour forcer des pertes
- [ ] Vérifier les logs de détection : `⚠️  Perte détectée!`
- [ ] Vérifier les statistiques : `📊 Stats bateau`

### 3. Test de Redémarrage
- [ ] Redémarrer le BoatGPS en cours de session
- [ ] Vérifier le log : `✓ GPS redémarré`

### 4. Test Multi-Bateaux
- [ ] Connecter plusieurs GPS
- [ ] Vérifier que chaque bateau a ses propres compteurs
- [ ] Vérifier les statistiques individuelles

## Prochaines Améliorations Possibles

### Interface Utilisateur
- [ ] Afficher le taux de perte sur l'écran LCD
- [ ] Indicateur visuel (couleur) selon la qualité du signal
- [ ] Écran de statistiques détaillées par bateau

### Logs Améliorés
- [ ] Logger les statistiques dans un fichier séparé
- [ ] Graphiques de taux de perte sur le Display
- [ ] Export CSV des statistiques

### Alertes
- [ ] Alerte sonore si taux de perte > seuil
- [ ] Notification sur perte prolongée
- [ ] Indicateur de qualité de liaison

## Résumé des Fichiers Modifiés

### Projet BoatGPS
- ✅ `include/Communication.h` - Ajout `sequenceNumber` dans structure
- ✅ `src/Communication.cpp` - Auto-incrémentation et logs
- ✅ `include/Storage.h` - Signature avec sequenceNumber
- ✅ `src/Storage.cpp` - Enregistrement dans JSON
- ✅ `src/main.cpp` - Passage du numéro au stockage

### Projet Display
- ✅ `include/DisplayTypes.h` - Ajout `sequenceNumber` dans structure
- ✅ `src/main.cpp` - Détection de perte et statistiques
- ✅ `src/Storage.cpp` - Enregistrement dans JSON (2 endroits)

### Documentation
- ✅ `ESPNOW_BROADCAST_ACK.md` (BoatGPS) - Explication technique
- ✅ `SEQUENCE_NUMBER.md` (BoatGPS) - Guide d'utilisation
- ✅ Ce fichier (Display) - Résumé des modifications

## Statut

✅ **Compilations réussies**
- BoatGPS : OK
- Display : OK

📝 **Prêt pour les tests sur matériel**
