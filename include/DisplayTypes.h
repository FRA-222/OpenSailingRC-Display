#pragma once
#include <stdint.h>

// Structures de données
typedef struct struct_message_Boat {
    int8_t messageType;  // 1 = Boat, 2 = Anemometer
    char name[18];     // MAC address as string (format: "AA:BB:CC:DD:EE:FF")
    int boatId;          // Numeric boat ID
    uint32_t gpsTimestamp; // Timestamp GPS en millisecondes
    float latitude;
    float longitude;
    float speed;     // en noeuds (knots)
    float heading;   // en degrés (0=N, 90=E, 180=S, 270=W)
    uint8_t satellites; // nombre de satellites visibles
} struct_message_Boat;

typedef struct struct_message_Anemometer {
    int8_t messageType;  // 1 = Boat, 2 = Anemometer
    char anemometerId[18];   // MAC address as string (format: "AA:BB:CC:DD:EE:FF")
    uint8_t macAddress[6];   // MAC address of the device
    float windSpeed;         // Wind speed value
} struct_message_Anemometer;

// Structure legacy pour compatibilité avec ancienne version anémomètre
typedef struct struct_message_Anemometer_Legacy {
    int8_t messageType;  // 1 = Boat, 2 = Anemometer
    uint32_t anemometerId;   // Ancien format: ID numérique
    uint8_t macAddress[6];   // MAC address of the device
    float windSpeed;         // Wind speed value
} struct_message_Anemometer_Legacy;



// Constantes d'affichage
const int screenWidth = 320;
const int screenHeight = 240;
const int centerX = screenWidth / 2;
const int centerY = (screenHeight / 2) + 30;
const int arrowLength = 55;

// Pour la boussole
static float previousAngle = -1;

// Conversion degrés/radian
#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.017453292519943295769236907684886
#endif
