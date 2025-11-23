# Améliorations du Display OpenSailingRC

## Date : 23 novembre 2025

## Modifications apportées

### 1. Changement de terminologie : BUOY → WIND
- Remplacement de tous les labels "BUOY" par "WIND" dans l'interface
- Meilleure représentation sémantique : les données affichées proviennent de l'anémomètre et représentent le vent

### 2. Ajout d'une ligne pour la direction du vent
L'affichage a été réorganisé pour afficher 4 lignes de données au lieu de 3 :

**Ancien affichage (3 lignes) :**
- BOAT : Vitesse (KTS)
- BOAT : Cap (DEG)
- BUOY : Vitesse vent (KTS)

**Nouvel affichage (4 lignes) :**
- BOAT : Vitesse (KTS)
- BOAT : Cap (DEG)
- WIND : Vitesse (KTS)
- WIND : Direction (DEG)

### 3. Réduction de la taille de police
- Police réduite de taille 4 à taille 3
- Permet l'affichage de 4 lignes d'information tout en gardant une bonne lisibilité
- Positionnement vertical ajusté pour un espacement optimal

### 4. Gestion des timeouts avec affichage "--"
Implémentation d'un système de timeout de 5 secondes :
- Si aucune donnée n'est reçue pendant 5 secondes, affiche "--" au lieu de la dernière valeur
- S'applique à :
  - Vitesse du bateau (BOAT KTS)
  - Cap du bateau (BOAT DEG)
  - Vitesse du vent (WIND KTS)
  - Direction du vent (WIND DEG)
- Permet de distinguer visuellement les données récentes des données obsolètes

### 5. Préparation pour les données futures des bouées

#### Structure `struct_message_Anemometer` enrichie
Ajout du champ timestamp :
```cpp
typedef struct struct_message_Anemometer {
    int8_t messageType;
    char anemometerId[18];
    uint8_t macAddress[6];
    uint32_t sequenceNumber;
    float windSpeed;
    unsigned long timestamp; // NOUVEAU : Horodatage de la mesure
} struct_message_Anemometer;
```

**Note :** La direction du vent (`windDirection`) sera fournie par les bouées GPS dans une structure séparée, pas par l'anémomètre.

#### Structure `struct_message_Boat` enrichie
Ajout d'un timestamp :
```cpp
typedef struct struct_message_Boat {
    // ... champs existants ...
    unsigned long timestamp; // NOUVEAU : Horodatage de la mesure
} struct_message_Boat;
```

## Compatibilité

### Rétrocompatibilité
- Le code reste compatible avec l'anémomètre actuel (format legacy)
- Si `windDirection` n'est pas fournie, elle est initialisée à 0
- L'affichage "--" apparaîtra pour la direction du vent jusqu'à ce que les bouées envoient cette donnée

### Future intégration des bouées GPS avec direction du vent
Lorsque les bouées GPS enverront la direction du vent :
1. Les bouées enverront `windDirection` dans une **structure séparée** (pas dans `struct_message_Anemometer`)
2. Le Display recevra cette donnée via les paramètres `windDirection` et `windDirTimestamp` de la fonction `drawDisplay()`
3. L'affichage basculera automatiquement de "--" vers la valeur en degrés
4. Le timeout garantira que seules les données récentes (< 5 secondes) sont affichées

#### Signature de la fonction `drawDisplay()` :
```cpp
void drawDisplay(
    const struct_message_Boat& boatData,
    const struct_message_Anemometer& anemometerData,
    bool isRecording,
    bool isServerActive = false,
    int boatCount = 0,
    float windDirection = 0,           // Direction du vent des bouées
    unsigned long windDirTimestamp = 0 // Timestamp de la direction
);
```

## Détails techniques

### Logique de timeout
```cpp
// Vérification effectuée à chaque cycle d'affichage
unsigned long currentTime = millis();
bool boatDataValid = (currentTime - boatData.timestamp) < 5000;     // 5 secondes
bool windDataValid = (currentTime - anemometerData.timestamp) < 5000;

// Si timeout dépassé : affiche "--"
// Sinon : affiche la valeur numérique
```

### Positions d'affichage
```
Y=10  : BOAT vitesse (KTS)
Y=50  : BOAT orientation (DEG)
Y=90  : WIND vitesse (KTS)
Y=130 : WIND direction (DEG)
Y=200 : Boutons (REC / BOAT? / WIFI)
```

### Zones d'effacement
Chaque valeur numérique a sa zone d'effacement optimisée :
- X: 120 à 235 (largeur 115 pixels)
- Hauteur : 28 pixels
- Permet de nettoyer complètement l'ancienne valeur avant d'afficher la nouvelle

## Tests recommandés

1. **Test au démarrage** : Vérifier que "--" s'affiche pour toutes les valeurs avant réception de données
2. **Test réception normale** : Vérifier l'affichage des valeurs numériques
3. **Test timeout bateau** : Éteindre le bateau et vérifier que "--" apparaît après 5 secondes
4. **Test timeout anémomètre** : Éteindre l'anémomètre et vérifier que "--" apparaît après 5 secondes
5. **Test reconnexion** : Rallumer les appareils et vérifier le retour aux valeurs numériques

## Notes pour le développement futur

### Intégration des bouées GPS avec anémomètre
Pour que les bouées envoient la direction du vent :
1. Ajouter `windDirection` dans la structure d'envoi de la bouée
2. S'assurer que `timestamp` est correctement mis à jour à chaque envoi
3. La direction devra être calibrée par rapport au Nord magnétique

### Amélioration possible
- Ajouter un indicateur visuel (icône/couleur) pour signaler les données en timeout
- Historique graphique des données vent sur 30 secondes
- Moyenne glissante pour lisser les valeurs affichées
