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
  
public:
  void showSplashScreen();
  void drawSpeedBar(float speedKnots);
  void drawDisplay(const struct_message_Boat& boatData, const struct_message_Anemometer& anemometerData, bool isRecording, bool isServerActive = false);
  void drawCompass(float heading);
  void checkButtons(struct_message_display_to_boat& outgoingData, uint8_t* boatAddress, Logger& logger);
  void showFileServerStatus(bool active, const String& ipAddress);
  void updateServerMessageDisplay(); // Nouvelle fonction pour l'affichage non-bloquant
  bool needsRefresh(); // Vérifie si un refresh est nécessaire après message serveur
  void drawButtonLabels(bool isRecording, bool isServerActive);
  void showSDError(const String& errorMessage);
};
