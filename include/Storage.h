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
 * @struct StorageData
 * @brief Structure containing all data to be saved
 * 
 * This structure encapsulates data received from the boat and anemometer
 * with a timestamp to allow chronological reconstruction.
 */
struct StorageData {
    unsigned long timestamp;                    ///< Reception timestamp (milliseconds since startup)
    struct_message_Boat boatData;              ///< GPS and navigation data from boat
    struct_message_Anemometer anemometerData;  ///< Wind speed and direction data
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
     *     "anemometerId": 1,
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
     * @brief Generate a unique filename based on timestamp
     * @return Filename in format "/replay/replay_[timestamp].json"
     * 
     * Uses millis() to create a unique filename avoiding
     * conflicts during rapid system restarts.
     * 
     * @example "/replay/replay_1234567890.json"
     */
    String generateFileName();
};