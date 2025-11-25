#pragma once
#include <M5Unified.h>
#include <esp_now.h>
#include "Logger.h"
#include "DisplayTypes.h"



class Display {
private:
  // Variables pour l'affichage temporaire du statut serveur
  bool showingServerMessage = false;
  unsigned long serverMessageStartTime = 0;
  bool serverMessageActive = false;
  String serverMessageIP = "";
  bool needsRefreshAfterServerMessage = false; // Flag pour indiquer qu'un refresh est nécessaire
  
  // Variables statiques pour l'optimisation de l'affichage
  bool labelsDrawn = false;
  float lastSpeedKnots = -999;
  float lastHeading = -999;
  uint8_t lastSatellites = 255;
  float lastWindSpeedKnots = -999;
  float lastWindDirection = -999;
  bool lastIsRecording = false;
  bool lastIsServerActive = false;
  int lastBoatCount = 0;
  int lastBatteryPercent = -1;
  bool lastIsCharging = false;
  
public:
  void showSplashScreen();
  void drawSpeedBar(float speedKnots);
  void drawDisplay(const struct_message_Boat& boatData, const struct_message_Anemometer& anemometerData, bool isRecording, bool isServerActive = false, int boatCount = 0, float windDirection = 0, unsigned long windDirTimestamp = 0);
  void drawCompass(float heading);
  void showFileServerStatus(bool active, const String& ipAddress);
  void updateServerMessageDisplay(); // Nouvelle fonction pour l'affichage non-bloquant
  bool needsRefresh(); // Vérifie si un refresh est nécessaire après message serveur
  void drawButtonLabels(bool isRecording, bool isServerActive, int boatCount = 0);
  void showSDError(const String& errorMessage);
  void forceFullRefresh(); // Force un rafraîchissement complet de l'affichage
};
