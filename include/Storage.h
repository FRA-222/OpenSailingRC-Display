/**
 * @file Storage.h
 * @brief GPS and anemometer data storage manager for SD card
 * 
 * This class manages the writing of navigation data (boat GPS and anemometer)
 * to an SD card in JSON format for later replay and analysis.
 * 
 * @author Philippe Hubert
 * @date 2025
 */

#pragma once
#include <vector>
#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>
#include "DisplayTypes.h"

// Forward declaration
class Logger;

/**
 * @enum DataType
 * @brief Type of data being stored
 */
enum DataType {
    DATA_TYPE_BOAT = 1,        ///< GPS and navigation data from boat
    DATA_TYPE_ANEMOMETER = 2,  ///< Wind speed and direction data from anemometer
    DATA_TYPE_BUOY = 3         ///< Autonomous GPS buoy data
};

/**
 * @struct StorageData
 * @brief Structure containing data to be saved with type identification
 * 
 * This structure encapsulates data received from boats or anemometers
 * with a timestamp and type identifier to allow chronological reconstruction
 * and proper data separation.
 */
struct StorageData {
    unsigned long timestamp;                    ///< Reception timestamp (milliseconds since startup)
    DataType dataType;                         ///< Type of data (boat, anemometer, or buoy)
    float windDirection;                       ///< Average wind direction from buoys (for anemometer entries)
    union {
        struct_message_Boat boatData;          ///< GPS and navigation data (when dataType = DATA_TYPE_BOAT)
        struct_message_Anemometer anemometerData; ///< Wind data (when dataType = DATA_TYPE_ANEMOMETER)
        struct_message_Buoy buoyData;          ///< Autonomous buoy data (when dataType = DATA_TYPE_BUOY)
    };
};

/**
 * @class Storage
 * @brief SD card data storage manager
 * 
 * This class manages SD card initialization and writing of navigation data
 * in JSON format. It supports both single and batch writing to optimize performance.
 * 
 * Features:
 * - SPI configuration specific to M5Stack Core2
 * - JSON file format for easy analysis
 * - Batch writing to reduce latency
 * - Robust error handling
 * - Organized directory structure (/replay/)
 * 
 * SPI Configuration (M5Stack Core2):
 * - SCK:  GPIO 18
 * - MISO: GPIO 38  
 * - MOSI: GPIO 23
 * - CS:   GPIO 4
 */

class Storage {
private:
    Logger* logger_;           ///< Pointer to the logging system
    String currentFileName_;   ///< Current storage file name
    bool sdInitialized_;      ///< SD card initialization status
    
public:
    /**
     * @brief Storage class constructor
     * 
     * Initializes member variables and generates a unique filename
     * based on startup timestamp.
     */
    Storage();
    
    /**
     * @brief Configure the logging system
     * @param logger Reference to the Logger instance to use
     * 
     * Allows the Storage class to emit debug and error messages
     * through the centralized logging system.
     */
    void setLogger(Logger& logger);
    
    /**
     * @brief Initialize SD card with M5Stack Core2 SPI configuration
     * @return true if initialization succeeds, false otherwise
     * 
     * This method:
     * - Configures Core2-specific SPI pins
     * - Attempts initialization at different frequencies (4MHz then 1MHz)
     * - Creates /replay/ directory if it doesn't exist
     * - Displays detailed diagnostic messages
     * 
     * @note Requires SD card formatted as FAT32
     * @warning Card must be inserted before calling this function
     */
    bool initSD();
    
    /**
     * @brief Initialize filename using RTC timestamp
     * @return true if filename generated successfully, false otherwise
     * 
     * This method generates a unique filename based on the RTC timestamp.
     * Must be called after RTC is initialized to get accurate timestamps.
     * If RTC is not set (year < 2023), falls back to millis().
     * 
     * @note Call this method after RTC initialization in setup()
     * @warning Must be called before any write operations
     */
    bool initializeFileName();
    
    /**
     * @brief Write a single data entry to SD card
     * @param data Structure containing data to save
     * @return true if write succeeds, false otherwise
     * 
     * Saves a StorageData structure in JSON format to the current file.
     * Each entry is written on a separate line for easy parsing.
     * 
     * Generated JSON format:
     * ```json
     * {
     *   "timestamp": 12345,
     *   "boat": {
     *     "messageType": 1,
     *     "latitude": 43.123456,
     *     "longitude": 2.654321,
     *     "speed": 5.2,
     *     "heading": 280,
     *     "satellites": 8,
     *     "isGPSRecording": true
     *   },
     *   "anemometer": {
     *     "messageType": 2,
     *     "anemometerId": "AA:BB:CC:DD:EE:FF",
     *     "windSpeed": 12.5
     *   }
     * }
     * ```
     */
    bool writeData(const StorageData& data);
    
    /**
     * @brief Write a batch of data to SD card
     * @param dataList Vector containing data to save
     * @return true if write succeeds, false otherwise
     * 
     * Optimizes performance by writing multiple entries in a single
     * file operation. Recommended for writing accumulated data
     * to reduce latency and SD card wear.
     * 
     * @note More efficient than multiple writeData() calls
     * @warning Vector must not be empty
     */
    bool writeDataBatch(const std::vector<StorageData>& dataList);
    
    /**
     * @brief Send a message to the logging system
     * @param message Message to log
     * 
     * Utility method to send debug messages through the configured Logger.
     * Does nothing if no Logger has been configured.
     */
    void log(const String& message);
    
    /**
     * @brief Generate a unique filename based on RTC timestamp
     * @return Filename in format "/replay/YYYY-MM-DD_HH-MM-SS.json" or "/replay/session_XXXX_N.json"
     * 
     * Uses RTC to create a human-readable filename. If RTC is not set
     * (year < 2023), falls back to session-based naming using MAC address
     * suffix and incremental session number for uniqueness.
     * 
     * @example "/replay/2025-09-21_14-30-45.json" (RTC configured)
     * @example "/replay/session_A1B2_1.json" (RTC not configured)
     */
    String generateFileName();
    
    /**
     * @brief Synchronize RTC with NTP time server
     * @param ntpServer NTP server address (default: "pool.ntp.org")
     * @param gmtOffset GMT offset in seconds (default: 0)
     * @param daylightOffset Daylight saving offset in seconds (default: 0)
     * @return true if synchronization succeeds, false otherwise
     * 
     * Attempts to synchronize the M5Stack Core2 RTC with an NTP server
     * when WiFi is available. This ensures accurate timestamps for
     * file naming and data logging.
     * 
     * @note Requires active WiFi connection
     * @warning Call only when WiFi is connected
     */
    bool syncRTCFromNTP(const char* ntpServer = "pool.ntp.org", 
                        long gmtOffset = 0, 
                        int daylightOffset = 0);
    
    /**
     * @brief Get current timestamp from RTC
     * @return Current Unix timestamp in seconds, or 0 if RTC not available
     * 
     * Returns the current time as Unix timestamp from the M5Stack Core2 RTC.
     * If RTC is not properly initialized or time is invalid, returns 0.
     * 
     * @note Returns 0 if year < 2023 (indicating RTC not set)
     */
    time_t getCurrentTimestamp();
};