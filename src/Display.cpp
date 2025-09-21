// Copyright (C) 2025 Philippe Hubert
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

/**
 * @file Display.cpp
 * @brief Display management class for OpenSailingRC project
 * 
 * This file contains the implementation of the Display class which handles
 * all display operations for a sailing boat monitoring system using M5Stack.
 * It provides functions for showing boat data, wind information, GPS status,
 * and user interaction through touch interface.
 */
#include "Display.h"

/**
 * @brief Displays the initial splash screen with boat identification
 * 
 * Shows "GPS" and "FRA222" centered on screen for 2 seconds, then clears the display.
 * Uses white text on black background with large font size.
 */

void Display::showSplashScreen() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM); // Centre
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString("GPS", centerX, centerY - 60);
    M5.Lcd.drawString("FRA222", centerX, centerY + 0);
    delay(2000);
    M5.Lcd.fillScreen(BLACK);
}

/**
 * @brief Draws a vertical speed bar indicator on the right side of the screen
 * 
 * @param speedKnots Current boat speed in knots
 * 
 * Creates a color-coded vertical bar showing speed relative to maximum (6 knots).
 * Colors: Green (0-2 knots), Orange (2-4 knots), Red (>4 knots).
 * Bar is positioned at the right edge of screen with white border.
 */

void Display::drawSpeedBar(float speedKnots) {
    M5.Lcd.fillRect(screenWidth - 20, 100, 10, 120, BLACK);
    float maxSpeed = 6.0;
    int barHeight = (int)((speedKnots / maxSpeed) * 120);
    if (barHeight > 120) barHeight = 120;
    uint32_t barColor = GREEN;
    if (speedKnots > 2.0 && speedKnots <= 4.0) {
      barColor = ORANGE;
    } else if (speedKnots > 4.0) {
      barColor = RED;
    }
    int barY = 220 - barHeight;
    M5.Lcd.fillRect(screenWidth - 20, barY, 10, barHeight, barColor);
    M5.Lcd.drawRect(screenWidth - 20, 100, 10, 120, WHITE);
}

/**
 * @brief Main display function that renders all boat and wind data
 * 
 * @param boatData Structure containing boat telemetry (speed, heading, GPS info)
 * @param anemometerData Structure containing wind measurement data
 * 
 * Displays:
 * - Boat identifier (F222) in red
 * - Current speed in knots with color coding
 * - Boat heading in degrees
 * - GPS satellite count
 * - Wind speed from buoy sensor
 * - GPS recording status indicator (green "RECORD" button when active)
 */
void Display::drawDisplay(const struct_message_Boat& boatData, const struct_message_Anemometer& anemometerData, bool isRecording, bool isServerActive) {
    M5.Lcd.fillRect(0, 0, screenWidth, 180, BLACK);
    float speedKnots = boatData.speed * 1.94384;
    float windSpeedKnots = anemometerData.windSpeed * 1.94384;
    uint32_t speedColor = GREEN;
    if (speedKnots > 2.0 && speedKnots <= 4.0) {
      speedColor = ORANGE;
    } else if (speedKnots > 4.0) {
      speedColor = RED;
    }
    M5.Lcd.setTextDatum(TL_DATUM);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.println("F222");
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(140, 50);
    M5.Lcd.printf("%.1f", speedKnots);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(240, 50);
    M5.Lcd.println("KTS");
    M5.Lcd.setTextSize(4);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(140, 90);
    M5.Lcd.printf("%.0f", boatData.heading);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(240, 90);
    M5.Lcd.println("DEG");
    M5.Lcd.setCursor(240, 10);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("%d SAT", boatData.satellites);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(10, 150);
    M5.Lcd.println("BUOY1");
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(140, 150);
    M5.Lcd.printf("%.1f", windSpeedKnots);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(240, 150);
    M5.Lcd.println("KTS");
    //drawSpeedBar(speedKnots);
    
    // Dessiner les boutons avec leurs statuts
    drawButtonLabels(isRecording, isServerActive);
}

/**
 * @brief Draws a compass with directional arrow showing boat heading
 * 
 * @param heading Current boat heading in degrees (0-360)
 * 
 * Features:
 * - Circular compass with N/E/S/W labels (drawn once)
 * - Red arrow pointing in current heading direction
 * - Erases previous arrow position to avoid ghosting
 * - White center dot for reference point
 */
void Display::drawCompass(float heading) {
    int x0 = centerX;
    int y0 = centerY;
    if (previousAngle >= 0) {
      float oldRad = (previousAngle - 90) * DEG_TO_RAD;
      int oldX = x0 + cos(oldRad) * arrowLength;
      int oldY = y0 + sin(oldRad) * arrowLength;
      M5.Lcd.drawLine(x0, y0, oldX, oldY, BLACK);
    }
    static bool circleDrawn = false;
    if (!circleDrawn) {
      M5.Lcd.drawCircle(x0, y0, arrowLength, TFT_BLUE);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setTextDatum(MC_DATUM);
      M5.Lcd.drawString("N", x0, y0 - arrowLength - 15);
      M5.Lcd.drawString("E", x0 + arrowLength + 15, y0);
      M5.Lcd.drawString("S", x0, y0 + arrowLength + 15);
      M5.Lcd.drawString("W", x0 - arrowLength - 15, y0);
      circleDrawn = true;
    }
    float angleRad = (heading - 90) * DEG_TO_RAD;
    int x1 = x0 + cos(angleRad) * arrowLength;
    int y1 = y0 + sin(angleRad) * arrowLength;
    M5.Lcd.drawLine(x0, y0, x1, y1, RED);
    M5.Lcd.fillCircle(x0, y0, 5, WHITE);
    previousAngle = heading;
}


/**
 * @brief Handles touch screen input for GPS recording control
 * 
 * @param outgoingData Reference to data structure sent to boat
 * @param boatAddress ESP-NOW address of the target boat
 * @param logger Reference to logging system for status messages
 * 
 * Touch zones:
 * - Left half of bottom area: Start GPS recording
 * - Right half of bottom area: Stop GPS recording
 * Sends recording commands to boat via ESP-NOW protocol with error handling.
 */
void Display::checkButtons(struct_message_display_to_boat& outgoingData, uint8_t* boatAddress, Logger& logger) {
    if (M5.Touch.getCount()) {
      auto t = M5.Touch.getDetail();
      if (t.y > 200) {
        if (t.x < 160) {
          outgoingData.recordGPS = true;
        } else {
          outgoingData.recordGPS = false;
        }
        esp_err_t result = esp_now_send(boatAddress, (uint8_t *)&outgoingData, sizeof(outgoingData));
        if (result == ESP_OK) {
          logger.log("Données isGPSRecording envoyées au bateau");
        } else {
          logger.log("Erreur d'envoi de donnée isGPSRecording envoyées au bateau");
        }
      }
    }
}

/**
 * @brief Displays File Server status on screen
 * 
 * @param active True if file server is active, false otherwise
 * @param ipAddress IP address of the server when active, error message when inactive
 * 
 * Shows a colored indicator and text to inform user about file server status.
 * Green background when active with IP address, red when inactive or error.
 * Message is displayed in the center of the screen temporarily.
 */
void Display::showFileServerStatus(bool active, const String& ipAddress) {
    // Sauvegarder la zone à modifier
    int messageY = centerY - 30;
    int messageHeight = 60;
    
    // Afficher le message de statut
    if (active) {
        M5.Lcd.fillRect(0, messageY, screenWidth, messageHeight, GREEN);
        M5.Lcd.setTextColor(BLACK);
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.setTextSize(3);
        M5.Lcd.drawString("SERVEUR ACTIF", centerX, centerY - 10);
        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("http://" + ipAddress, centerX, centerY + 15);
    } else {
        M5.Lcd.fillRect(0, messageY, screenWidth, messageHeight, RED);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.setTextSize(3);
        M5.Lcd.drawString("SERVEUR ARRETE", centerX, centerY - 10);
        M5.Lcd.setTextSize(2);
        if (ipAddress != "") {
            M5.Lcd.drawString(ipAddress, centerX, centerY + 15);
        } else {
            M5.Lcd.drawString("Mode normal restaure", centerX, centerY + 15);
        }
    }
    
    // Afficher le message pendant 3 secondes
    delay(3000);
    
    // Effacer la zone du message
    M5.Lcd.fillRect(0, messageY, screenWidth, messageHeight, BLACK);
}

/**
 * @brief Draws button labels at the bottom of the screen
 * 
 * @param isRecording Current GPS recording state
 * @param isServerActive Current file server state
 * 
 * Displays labeled buttons at the bottom to indicate touch zones:
 * - Left button: GPS recording control (green if recording, red if stopped)
 * - Right button: File server control (green if active, red if stopped)
 * - Middle area: Reserved/unused
 */
void Display::drawButtonLabels(bool isRecording, bool isServerActive) {
    int buttonY = 200;
    int buttonHeight = 40;
    int button1Width = 107;  // Premier tiers
    int button3Width = 107;  // Dernier tiers
    int button3X = 213;      // Position X du 3ème bouton
    
    // Bouton 1 (gauche) - Enregistrement GPS
    M5.Lcd.fillRect(0, buttonY, button1Width, buttonHeight, isRecording ? GREEN : RED);
    M5.Lcd.drawRect(0, buttonY, button1Width, buttonHeight, WHITE);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString(isRecording ? "REC" : "STOP", button1Width/2, buttonY + buttonHeight/2);
    
    // Zone centrale (grise) - Non utilisée
    M5.Lcd.fillRect(button1Width, buttonY, button3X - button1Width, buttonHeight, DARKGREY);
    M5.Lcd.drawRect(button1Width, buttonY, button3X - button1Width, buttonHeight, WHITE);
    
    // Bouton 3 (droite) - Serveur de fichiers
    M5.Lcd.fillRect(button3X, buttonY, button3Width, buttonHeight, isServerActive ? GREEN : RED);
    M5.Lcd.drawRect(button3X, buttonY, button3Width, buttonHeight, WHITE);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString(isServerActive ? "SERV" : "WIFI", button3X + button3Width/2, buttonY + buttonHeight/2);
}
