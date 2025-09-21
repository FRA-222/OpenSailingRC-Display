# Configuration WiFi pour le serveur de fichiers

## Description

Le M5Stack Core2 fonctionne normalement en mode ESPNow pour recevoir les données GPS du bateau. Lorsque vous activez le serveur de fichiers via le bouton tactile droit, l'appareil bascule temporairement en mode WiFi pour permettre l'accès aux fichiers depuis un navigateur web.

## Configuration WiFi

### 1. Créer le fichier de configuration

Créez un fichier nommé `wifi_config.json` à la racine de votre carte SD avec le contenu suivant :

```json
{
  "ssid": "NomDeVotreReseauWiFi",
  "password": "VotreMotDePasse"
}
```

### 2. Exemple de fichier

```json
{
  "ssid": "MonReseauWiFi",
  "password": "MonMotDePasse123"
}
```

## Utilisation

### Boutons tactiles (en bas de l'écran)

- **Bouton gauche** : Active/désactive l'enregistrement GPS
  - Vert = Enregistrement actif
  - Rouge = Enregistrement arrêté

- **Bouton droit** : Active/désactive le serveur de fichiers
  - Première pression : Se connecte au WiFi et démarre le serveur HTTP
  - Seconde pression : Arrête le serveur et revient en mode ESPNow

### Mode serveur de fichiers

1. Appuyez sur le bouton tactile droit
2. L'appareil lit le fichier `wifi_config.json` depuis la carte SD
3. Se connecte au réseau WiFi configuré
4. Démarre un serveur HTTP sur le port 80
5. L'adresse IP s'affiche sur l'écran du Core2

### Accès aux fichiers

1. Connectez votre PC/Mac au même réseau WiFi
2. Ouvrez un navigateur web
3. Saisissez l'adresse IP affichée sur le Core2
4. Naviguez et téléchargez les fichiers GPS stockés

### Retour en mode ESPNow

Appuyez à nouveau sur le bouton tactile droit pour :
- Arrêter le serveur HTTP
- Déconnecter le WiFi
- Réinitialiser ESPNow pour recevoir les données du bateau

## Structure des fichiers

Les fichiers GPS sont stockés dans des dossiers par date :
```
/2024/
  /01/
    /15/
      gps_data_20240115_143022.json
      gps_data_20240115_153045.json
```

## Dépannage

### Erreur "Erreur config WiFi"
- Vérifiez que le fichier `wifi_config.json` existe sur la carte SD
- Vérifiez la syntaxe JSON (guillemets, virgules)
- Vérifiez que les clés "ssid" et "password" sont présentes

### Impossible de se connecter au WiFi
- Vérifiez que le SSID et le mot de passe sont corrects
- Vérifiez que le réseau WiFi est accessible
- Le réseau doit être en 2.4GHz (l'ESP32 ne supporte pas le 5GHz)

### Retour automatique en ESPNow
Si le serveur de fichiers s'arrête de façon inattendue, l'appareil revient automatiquement en mode ESPNow pour continuer à recevoir les données GPS du bateau.