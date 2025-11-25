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

// Variables globales de timestamp (définies dans main.cpp)
extern unsigned long boatDataTimestamp;
extern unsigned long anemometerDataTimestamp;

/**
 * @brief Displays the initial splash screen with project identification
 * 
 * Shows "OpenSailingRC Display" and "V1.0" centered on screen for 2 seconds, then clears the display.
 * Uses white text on black background with large font size.
 */

void Display::showSplashScreen() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM); // Centre
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString("OpenSailingRC", centerX, centerY - 80);
    M5.Lcd.drawString("Display", centerX, centerY - 40);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("V1.0.4", centerX, centerY + 10);
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
void Display::drawDisplay(const struct_message_Boat& boatData, const struct_message_Anemometer& anemometerData, bool isRecording, bool isServerActive, int boatCount, float windDirection, unsigned long windDirTimestamp) {
    float speedKnots = boatData.speed * 1.94384;
    float windSpeedKnots = anemometerData.windSpeed * 1.94384;
    
    // Dessiner les labels fixes une seule fois au démarrage
    if (!labelsDrawn) {
        M5.Lcd.fillRect(0, 0, screenWidth, 180, BLACK);
        
        // Ligne 1 - BOAT vitesse (police taille 3)
        M5.Lcd.setTextDatum(TL_DATUM);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(10, 40);
        M5.Lcd.print("BOAT");
        
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(240, 40);
        M5.Lcd.print("KTS");
        
        // Ligne 2 - BOAT orientation
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(240, 80);
        M5.Lcd.print("DEG");
        
        // Ligne 3 - WIND vitesse
        M5.Lcd.setTextColor(RED);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(10, 120);
        M5.Lcd.print("WIND");
        
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(240, 120);
        M5.Lcd.print("KTS");
        
        // Ligne 4 - WIND orientation
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(240, 160);
        M5.Lcd.print("DEG");
        
        // Dessiner les boutons au premier affichage
        drawButtonLabels(isRecording, isServerActive, boatCount);
        lastIsRecording = isRecording;
        lastIsServerActive = isServerActive;
        lastBoatCount = boatCount;
        
        labelsDrawn = true;
    }
    
    // Vérifier timeout des données (5 secondes)
    unsigned long currentTime = millis();
    bool boatDataValid = (currentTime - boatDataTimestamp) < 5000;
    bool windDataValid = (currentTime - anemometerDataTimestamp) < 5000;
    bool windDirValid = (currentTime - windDirTimestamp) < 5000;
    
    // Mettre à jour uniquement la vitesse si elle a changé ou si le timeout a changé
    if (abs(speedKnots - lastSpeedKnots) > 0.05 || (boatDataValid != (lastSpeedKnots > -998))) {
        // Effacer l'ancienne valeur
        M5.Lcd.fillRect(120, 36, 115, 28, BLACK);
        
        // Dessiner la nouvelle valeur
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(120, 50);
        if (boatDataValid) {
            M5.Lcd.printf("%.1f", speedKnots);
            lastSpeedKnots = speedKnots;
        } else {
            M5.Lcd.print("---");
            lastSpeedKnots = -998; // Valeur spéciale pour indiquer timeout
        }
    }
    
    // Mettre à jour uniquement le cap si il a changé ou si le timeout a changé
    if (abs(boatData.heading - lastHeading) > 0.5 || (boatDataValid != (lastHeading > -998))) {
        // Effacer l'ancienne valeur
        M5.Lcd.fillRect(120, 76, 115, 28, BLACK);
        
        // Dessiner la nouvelle valeur
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(120, 90);
        if (boatDataValid) {
            M5.Lcd.printf("%.0f", boatData.heading);
            lastHeading = boatData.heading;
        } else {
            M5.Lcd.print("---");
            lastHeading = -998;
        }
    }
    
    // Mettre à jour uniquement les satellites si le nombre a changé
    if (boatData.satellites != lastSatellites) {
        // Effacer l'ancienne valeur
        M5.Lcd.fillRect(240, 0, 80, 20, BLACK);
        
        // Dessiner le pictogramme de satellite
        int satX = 245;
        int satY = 3;
        
        // Panneau solaire gauche (lignes horizontales bleues)
        M5.Lcd.drawRect(satX, satY, 4, 12, WHITE);
        M5.Lcd.drawLine(satX + 1, satY + 2, satX + 2, satY + 2, BLUE);
        M5.Lcd.drawLine(satX + 1, satY + 4, satX + 2, satY + 4, BLUE);
        M5.Lcd.drawLine(satX + 1, satY + 6, satX + 2, satY + 6, BLUE);
        M5.Lcd.drawLine(satX + 1, satY + 8, satX + 2, satY + 8, BLUE);
        M5.Lcd.drawLine(satX + 1, satY + 10, satX + 2, satY + 10, BLUE);
        
        // Corps du satellite (rectangle central blanc)
        M5.Lcd.fillRect(satX + 5, satY + 3, 4, 6, WHITE);
        
        // Panneau solaire droit (lignes horizontales bleues)
        M5.Lcd.drawRect(satX + 10, satY, 4, 12, WHITE);
        M5.Lcd.drawLine(satX + 11, satY + 2, satX + 12, satY + 2, BLUE);
        M5.Lcd.drawLine(satX + 11, satY + 4, satX + 12, satY + 4, BLUE);
        M5.Lcd.drawLine(satX + 11, satY + 6, satX + 12, satY + 6, BLUE);
        M5.Lcd.drawLine(satX + 11, satY + 8, satX + 12, satY + 8, BLUE);
        M5.Lcd.drawLine(satX + 11, satY + 10, satX + 12, satY + 10, BLUE);
        
        // Dessiner le nombre de satellites avec un espace
        M5.Lcd.setCursor(265, 8);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextSize(2);
        M5.Lcd.printf("%d", boatData.satellites);
        
        lastSatellites = boatData.satellites;
    }
    
    // Mettre à jour l'affichage de la batterie (centré en haut)
    int batteryPercent = M5.Power.getBatteryLevel();
    bool isCharging = M5.Power.isCharging();
    
    if (batteryPercent != lastBatteryPercent || isCharging != lastIsCharging) {
        // Effacer la zone centrale en haut
        M5.Lcd.fillRect(90, 0, 140, 25, BLACK);
        
        // Position centrée (écran 320px de large)
        int centerX = 160;
        
        // Dessiner le picto batterie à gauche du pourcentage
        int batteryX = centerX - 45;
        int batteryY = 2;  // Remonté pour aligner avec le texte (Y plus petit = plus haut)
        
        // Dessiner le corps de la batterie (rectangle)
        M5.Lcd.drawRect(batteryX, batteryY, 24, 12, WHITE);
        M5.Lcd.fillRect(batteryX + 1, batteryY + 1, 22, 10, BLACK);
        
        // Dessiner la borne + de la batterie
        M5.Lcd.fillRect(batteryX + 24, batteryY + 3, 2, 6, WHITE);
        
        // Remplir la batterie selon le niveau (proportionnel avec couleur)
        int fillWidth = (batteryPercent * 20) / 100;
        if (fillWidth > 20) fillWidth = 20;
        
        if (batteryPercent > 50) {
            M5.Lcd.fillRect(batteryX + 2, batteryY + 2, fillWidth, 8, GREEN);
        } else if (batteryPercent > 20) {
            M5.Lcd.fillRect(batteryX + 2, batteryY + 2, fillWidth, 8, ORANGE);
        } else {
            M5.Lcd.fillRect(batteryX + 2, batteryY + 2, fillWidth, 8, RED);
        }
        
        // Dessiner l'icône éclair si en charge
        if (isCharging) {
            // Éclair jaune sur la batterie
            M5.Lcd.fillTriangle(batteryX + 14, batteryY + 2,
                               batteryX + 10, batteryY + 7,
                               batteryX + 12, batteryY + 7, YELLOW);
            M5.Lcd.fillTriangle(batteryX + 10, batteryY + 7,
                               batteryX + 14, batteryY + 12,
                               batteryX + 12, batteryY + 7, YELLOW);
        }
        
        // Dessiner le pourcentage à droite du picto (avec couleur selon niveau)
        M5.Lcd.setCursor(batteryX + 32, 8);
        M5.Lcd.setTextSize(2);
        
        if (batteryPercent > 50) {
            M5.Lcd.setTextColor(GREEN);
        } else if (batteryPercent > 20) {
            M5.Lcd.setTextColor(ORANGE);
        } else {
            M5.Lcd.setTextColor(RED);
        }
        
        M5.Lcd.printf("%d%%", batteryPercent);
        
        lastBatteryPercent = batteryPercent;
        lastIsCharging = isCharging;
    }
    
    // Mettre à jour uniquement la vitesse du vent si elle a changé ou si le timeout a changé
    if (abs(windSpeedKnots - lastWindSpeedKnots) > 0.05 || (windDataValid != (lastWindSpeedKnots > -998))) {
        // Effacer l'ancienne valeur
        M5.Lcd.fillRect(120, 116, 115, 28, BLACK);
        
        // Dessiner la nouvelle valeur
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(120, 130);
        if (windDataValid) {
            M5.Lcd.printf("%.1f", windSpeedKnots);
            lastWindSpeedKnots = windSpeedKnots;
        } else {
            M5.Lcd.print("---");
            lastWindSpeedKnots = -998;
        }
    }
    
    // Mettre à jour la direction du vent si elle a changé ou si le timeout a changé
    if (abs(windDirection - lastWindDirection) > 0.5 || (windDirValid != (lastWindDirection > -998))) {
        // Effacer l'ancienne valeur
        M5.Lcd.fillRect(120, 156, 115, 28, BLACK);
        
        // Dessiner la nouvelle valeur
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(120, 170);
        if (windDirValid) {
            M5.Lcd.printf("%.0f", windDirection);
            lastWindDirection = windDirection;
        } else {
            M5.Lcd.print("---");
            lastWindDirection = -998;
        }
    }
    
    // Redessiner les boutons uniquement si leur état a changé
    if (isRecording != lastIsRecording || isServerActive != lastIsServerActive || boatCount != lastBoatCount) {
        drawButtonLabels(isRecording, isServerActive, boatCount);
        lastIsRecording = isRecording;
        lastIsServerActive = isServerActive;
        lastBoatCount = boatCount;
    }
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
    // Démarrer l'affichage temporaire non-bloquant
    showingServerMessage = true;
    serverMessageStartTime = millis();
    serverMessageActive = active;
    serverMessageIP = ipAddress;
    
    // Afficher immédiatement le message une première fois
    updateServerMessageDisplay();
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
void Display::drawButtonLabels(bool isRecording, bool isServerActive, int boatCount) {
    // Debug: Afficher l'état des boutons
    static bool lastServerActive = false;
    static bool lastRecording = false;
    
    if (isServerActive != lastServerActive || isRecording != lastRecording) {
        // Utiliser Serial.print pour éviter les dépendances circulaires
        Serial.print("DEBUG: drawButtonLabels - Recording: ");
        Serial.print(isRecording ? "TRUE" : "FALSE");
        Serial.print(", ServerActive: ");
        Serial.println(isServerActive ? "TRUE" : "FALSE");
        
        lastServerActive = isServerActive;
        lastRecording = isRecording;
    }
    
    int buttonY = 200;
    int buttonHeight = 40;
    int button1Width = 107;  // Premier tiers
    int button2Width = 106;  // Deuxième tiers (107 à 213)
    int button3Width = 107;  // Dernier tiers
    int button2X = 107;      // Position X du 2ème bouton
    int button3X = 213;      // Position X du 3ème bouton
    
    // Bouton 1 (gauche) - Enregistrement GPS
    M5.Lcd.fillRect(0, buttonY, button1Width, buttonHeight, isRecording ? RED : NAVY);
    M5.Lcd.drawRect(0, buttonY, button1Width, buttonHeight, WHITE);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString(isRecording ? "STOP" : "RECORD", button1Width/2, buttonY + buttonHeight/2);
    
    // Bouton 2 (centre) - Sélection bateau
    if (boatCount > 1) {
        // Plusieurs bateaux détectés : bouton rouge avec "Boat ?"
        M5.Lcd.fillRect(button2X, buttonY, button2Width, buttonHeight, RED);
        M5.Lcd.drawRect(button2X, buttonY, button2Width, buttonHeight, WHITE);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("BOAT ?", button2X + button2Width/2, buttonY + buttonHeight/2);
    } else {
        // Un seul ou aucun bateau : zone bleu marine (non utilisée)
        M5.Lcd.fillRect(button2X, buttonY, button2Width, buttonHeight, NAVY);
        M5.Lcd.drawRect(button2X, buttonY, button2Width, buttonHeight, WHITE);
    }
    
    // Bouton 3 (droite) - Serveur de fichiers
    M5.Lcd.fillRect(button3X, buttonY, button3Width, buttonHeight, isServerActive ? RED : NAVY);
    M5.Lcd.drawRect(button3X, buttonY, button3Width, buttonHeight, WHITE);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString(isServerActive ? "STOP" : "WIFI", button3X + button3Width/2, buttonY + buttonHeight/2);
}

/**
 * @brief Affiche un message d'erreur SD au centre de l'écran
 * 
 * @param errorMessage Le message d'erreur à afficher
 * 
 * Affiche un message d'erreur centré avec un fond rouge pour indiquer
 * un problème avec la carte SD (manquante, corrompue, etc.)
 */
void Display::showSDError(const String& errorMessage) {
    // Constantes pour le M5Stack Core2 (320x240)
    const int screenWidth = 320;
    const int centerX = 160;
    
    // Effacer la zone d'affichage principale
    M5.Lcd.fillRect(0, 0, screenWidth, 180, BLACK);
    
    // Fond rouge pour l'erreur
    M5.Lcd.fillRect(10, 60, screenWidth - 20, 60, RED);
    M5.Lcd.drawRect(10, 60, screenWidth - 20, 60, WHITE);
    
    // Texte d'erreur en blanc
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString("ERREUR SD", centerX, 75);
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString(errorMessage, centerX, 95);
    
    // Message d'instruction
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString("Insérer carte SD", centerX, 140);
}

/**
 * @brief Met à jour l'affichage temporaire du message serveur de manière non-bloquante
 */
void Display::updateServerMessageDisplay() {
    if (!showingServerMessage) return;
    
    // Si le serveur est ACTIF, ne jamais effacer le message (affichage permanent)
    // Si le serveur est INACTIF, effacer après 3 secondes
    if (!serverMessageActive && millis() - serverMessageStartTime >= 3000) {
        // Effacer le message et revenir à l'affichage normal
        showingServerMessage = false;
        needsRefreshAfterServerMessage = true; // Marquer qu'un refresh est nécessaire
        
        // Forcer un rafraîchissement complet de l'écran
        labelsDrawn = false;
        lastSpeedKnots = -999;
        lastHeading = -999;
        lastSatellites = -1;
        lastWindSpeedKnots = -999;
        lastWindDirection = -999;
        lastBatteryPercent = -1;
        lastIsCharging = false;
        
        return;
    }
    
    // Continuer à afficher le message seulement une fois par cycle pour éviter le clignotement
    static unsigned long lastRedraw = 0;
    if (millis() - lastRedraw < 100) return; // Redessiner max toutes les 100ms
    lastRedraw = millis();
    
    // Continuer à afficher le message
    const int screenWidth = 320;
    const int centerX = 160;
    const int centerY = 120;
    int messageY = centerY - 30;
    int messageHeight = 60;
    
    if (serverMessageActive) {
        M5.Lcd.fillRect(0, messageY, screenWidth, messageHeight, RED);
        M5.Lcd.setTextColor(BLACK);
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.setTextSize(3);
        M5.Lcd.drawString("SERVEUR ACTIF", centerX, centerY - 10);
        M5.Lcd.setTextColor(BLACK);
        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("http://" + serverMessageIP, centerX, centerY + 15);
    } else {
        M5.Lcd.fillRect(0, messageY, screenWidth, messageHeight, NAVY);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.setTextSize(3);
        M5.Lcd.drawString("SERVEUR ARRETE", centerX, centerY - 10);
        M5.Lcd.setTextSize(2);
        if (serverMessageIP != "") {
            M5.Lcd.drawString(serverMessageIP, centerX, centerY + 15);
        } else {
            M5.Lcd.drawString("Mode normal restaure", centerX, centerY + 15);
        }
    }
}

/**
 * @brief Vérifie si un refresh de l'affichage est nécessaire après un message serveur
 */
bool Display::needsRefresh() {
    if (needsRefreshAfterServerMessage) {
        needsRefreshAfterServerMessage = false; // Reset du flag
        return true;
    }
    return false;
}

/**
 * @brief Force un rafraîchissement complet de l'affichage
 * 
 * Réinitialise les variables statiques de drawDisplay pour forcer
 * le redessin complet de tous les éléments à l'écran.
 * Utile lors du changement de bateau sélectionné.
 */
void Display::forceFullRefresh() {
    // Réinitialiser toutes les variables d'état pour forcer un redessin complet
    labelsDrawn = false;
    lastSpeedKnots = -999;
    lastHeading = -999;
    lastSatellites = 255;
    lastWindSpeedKnots = -999;
    lastWindDirection = -999;
    lastIsRecording = false;
    lastIsServerActive = false;
    lastBoatCount = 0;
    
    // Effacer l'écran
    M5.Lcd.fillRect(0, 0, screenWidth, 180, BLACK);
}
