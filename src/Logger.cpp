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
 * @file Logger.cpp
 * @brief Implementation of the Logger class for multi-output logging functionality
 * 
 * This file contains the implementation of a versatile logging system that supports
 * simultaneous output to serial console, M5Stack display, and SD card storage.
 */
#include "Logger.h"

/**
 * @brief Constructs a Logger instance with specified output options
 * 
 * Initializes the logger with the desired output methods. Sets up the M5Stack display
 * with white background and black text if screen logging is enabled. Initializes
 * serial communication at 115200 baud if serial logging is enabled. SD card
 * initialization is planned but not yet implemented.
 * 
 * @param enableSDLogging Enable logging to SD card (currently not implemented)
 * @param serialLogging Enable logging to serial console
 * @param screenLogging Enable logging to M5Stack display
 */
Logger::Logger(bool enableSDLogging, bool serialLogging, bool screenLogging) {
    this->sdLogging = enableSDLogging;
    this->serialLogging = serialLogging;
    this->screenLogging = screenLogging;
    this->screenLine = 0;

    // Initialize screen if screen logging is enabled
    if (this->screenLogging) {
        M5.Display.fillScreen(WHITE);
        M5.Display.setTextColor(BLACK);
        M5.Display.setTextSize(2);
        M5.Display.setCursor(0, 0);
    }

    // Initialize serial if serial logging is enabled
    if (this->serialLogging) {
        Serial.begin(115200);
        while (!Serial); // Wait for Serial to be ready
    }

    // TODO: Initialize SD card if SD logging is enabled
    if (this->sdLogging) {
        // SD card initialization code here
    }
}

/**
 * @brief Logs a message to all enabled output destinations
 * 
 * Outputs the provided message to serial console, M5Stack display, and/or SD card
 * based on the current logging configuration. For screen logging, automatically
 * clears the display and resets cursor position when maximum lines are reached.
 * 
 * @param message The string message to be logged
 */
void Logger::log(const String& message) {
    // Serial logging
    if (serialLogging) {
        Serial.println(message);
    }

    // Screen logging
    if (screenLogging) {
        if (screenLine >= MAX_LINES) {
            M5.Display.fillScreen(WHITE);
            screenLine = 0;
        }
        M5.Display.setCursor(0, screenLine * LINE_HEIGHT);
        M5.Display.println(message);
        screenLine++;
    }

    // SD logging
    if (sdLogging) {
        // TODO: Implement SD card logging
    }
}

/**
 * @brief Enables or disables serial console logging
 * 
 * @param enable True to enable serial logging, false to disable
 */
void Logger::enableSerialLogging(bool enable) {
    serialLogging = enable;
}

/**
 * @brief Enables or disables M5Stack display logging
 * 
 * @param enable True to enable screen logging, false to disable
 */
void Logger::enableScreenLogging(bool enable) {
    screenLogging = enable;
}

/**
 * @brief Enables or disables SD card logging
 * 
 * @param enable True to enable SD logging, false to disable
 * @note SD card logging functionality is not yet implemented
 */
void Logger::enableSDLogging(bool enable) {
    sdLogging = enable;
}
