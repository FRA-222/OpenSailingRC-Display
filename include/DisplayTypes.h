#pragma once
#include <stdint.h>

// Structures de données
typedef struct struct_message_Boat {
    int8_t messageType;  // 1 = Boat, 2 = Anemometer
    char name[18];     // Custom boat name or MAC address (max 17 chars + null terminator)
    uint32_t sequenceNumber; // Sequence number for packet loss detection
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
    uint32_t sequenceNumber; // Sequence number for packet loss detection
    float windSpeed;         // Wind speed value
    unsigned long timestamp; // Timestamp of the measurement
} struct_message_Anemometer;

// Énumérations pour les bouées GPS autonomes
enum tEtatsGeneral {
    BUOY_INIT = 0,
    BUOY_READY,
    BUOY_MAINTENANCE,
    BUOY_HOME_DEFINITION,
    BUOY_NAV
};

enum tEtatsNav {
    NAV_NOTHING = 0,
    NAV_HOME,
    NAV_HOLD,
    NAV_STOP,
    NAV_BASIC,
    NAV_CAP,
    NAV_TARGET
};

typedef struct struct_message_Buoy {
    uint8_t buoyId;                     // Buoy ID (0-5)
    uint32_t timestamp;                 // Message timestamp
    
    // General state
    tEtatsGeneral generalMode;          // General state
    tEtatsNav navigationMode;           // Current navigation mode
    
    // Sensor status
    bool gpsOk;                         // GPS sensor status
    bool headingOk;                     // Heading sensor status
    bool yawRateOk;                     // Yaw rate sensor status
    
    // GPS position
    double latitude;                    // Latitude in degrees
    double longitude;                   // Longitude in degrees
    
    // Environmental data
    float temperature;                  // Temperature in °C
    
    // Battery data
    float remainingCapacity;            // Remaining battery capacity in mAh
    
    // Navigation data
    float distanceToCons;               // Distance to consigne/waypoint in meters
    
    // Autopilot commands
    int8_t autoPilotThrottleCmde;       // Autopilot throttle command (-100 to +100%)
    float autoPilotTrueHeadingCmde;     // Autopilot heading command in degrees
    int8_t autoPilotRudderCmde;         // Autopilot rudder command (-100 to +100%)
    
    // Forced commands
    int8_t forcedThrottleCmde;          // Forced throttle command (-100 to +100%)
    bool forcedThrottleCmdeOk;          // Forced throttle command active flag
    float forcedTrueHeadingCmde;        // Forced heading command in degrees
    bool forcedTrueHeadingCmdeOk;       // Forced heading command active flag
    int8_t forcedRudderCmde;            // Forced rudder command (-100 to +100%)
    bool forcedRudderCmdeOk;            // Forced rudder command active flag
} struct_message_Buoy;

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
