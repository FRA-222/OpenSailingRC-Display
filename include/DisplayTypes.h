#pragma once
#include <stdint.h>

// Message type constants (must match Hub PacketTypes.h)
static constexpr uint8_t MSG_TYPE_BOAT_GPS     = 1;
static constexpr uint8_t MSG_TYPE_ANEMOMETER   = 2;
static constexpr uint8_t MSG_TYPE_HUB_STATUS   = 10;

// Hub status packet (received from Hub every 5s)
typedef struct __attribute__((packed)) struct_message_HubStatus {
    uint8_t messageType;        // MSG_TYPE_HUB_STATUS = 10
    uint8_t hubId;
    uint32_t uptimeMs;
    uint32_t relayedCommands;
    uint32_t relayedStates;
    uint32_t relayedGPS;
    uint32_t relayedAnemometer;
    float batteryVoltage;
    int8_t rssiAvg;
    uint8_t connectedBuoys;
    uint8_t connectedBoats;
    bool anemometerSeen;
} struct_message_HubStatus;

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
    uint8_t ttl;     // Time-To-Live: 1=original, 0=relayed by Hub
} struct_message_Boat;

typedef struct struct_message_Anemometer {
    int8_t messageType;  // 1 = Boat, 2 = Anemometer
    char anemometerId[18];   // MAC address as string (format: "AA:BB:CC:DD:EE:FF")
    uint8_t macAddress[6];   // MAC address of the device
    uint32_t sequenceNumber; // Sequence number for packet loss detection
    float windSpeed;         // Wind speed value
    unsigned long timestamp; // Timestamp of the measurement
    uint8_t ttl;             // Time-To-Live: 1=original, 0=relayed by Hub
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

// IMPORTANT: Cette struct doit être identique à BuoyState dans
// ESPNowDataLinkManagement.h du projet Autonomous-GPS-Buoy
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

    // v2 additions for Hub relay
    uint16_t sequenceNumber;            // Sequence number for deduplication
    uint8_t ttl;                        // Time-To-Live: 1=original, 0=relayed by Hub
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
