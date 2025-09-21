#pragma once
#include <M5Unified.h>
#include <esp_now.h>
#include "Logger.h"
#include "DisplayTypes.h"



class Display {
public:
  void showSplashScreen();
  void drawSpeedBar(float speedKnots);
  void drawDisplay(const struct_message_Boat& boatData, const struct_message_Anemometer& anemometerData, bool isRecording, bool isServerActive = false);
  void drawCompass(float heading);
  void checkButtons(struct_message_display_to_boat& outgoingData, uint8_t* boatAddress, Logger& logger);
  void showFileServerStatus(bool active, const String& ipAddress);
  void drawButtonLabels(bool isRecording, bool isServerActive);
};
