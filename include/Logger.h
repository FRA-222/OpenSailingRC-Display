#pragma once
#include <Arduino.h>
#include <M5Unified.h>

/**
 * @brief Logger class for serial, screen, and SD card logging.
 *
 * This class provides logging functionalities for serial output, screen display (AtomS3),
 * and SD card file logging. Logging channels can be enabled or disabled independently.
 */
class Logger {
private:
    bool sdLogging;        // Enable/disable SD card logging
    bool serialLogging;    // Enable/disable serial logging
    bool screenLogging;    // Enable/disable screen logging
    int screenLine;        // Current line on the screen
    static const int MAX_LINES = 8;    // Maximum lines for AtomS3 screen
    static const int LINE_HEIGHT = 16; // Height of each line on screen

public:
    Logger(bool enableSDLogging = false, bool enableSerialLogging = true, bool enableScreenLogging = true);
    void log(const String& message);
    void enableSerialLogging(bool enable);
    void enableScreenLogging(bool enable);
    void enableSDLogging(bool enable);
};
