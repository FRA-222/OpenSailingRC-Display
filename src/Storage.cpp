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
 * @file Storage.cpp
 * @brief Implementation of GPS and anemometer data storage manager
 * 
 * This file contains the implementation of the Storage class that manages
 * writing navigation data to SD card in JSON format.
 * 
 * @author Philippe Hubert
 * @date 2025
 */

#include "Storage.h"
#include "Logger.h"
#include <SPI.h>

// SPI pin configuration for SD card (M5Stack Core2)
#define SPI_SCK  18  ///< Serial Clock pin
#define SPI_MISO 38  ///< Master In Slave Out pin (incoming data)
#define SPI_MOSI 23  ///< Master Out Slave In pin (outgoing data)  
#define SPI_CS   4   ///< Chip Select pin (SD card selection)

Storage::Storage() : logger_(nullptr), sdInitialized_(false) {
    // Generate a unique filename based on startup timestamp
    currentFileName_ = generateFileName();
}

void Storage::setLogger(Logger& logger) {
    logger_ = &logger;
}

void Storage::log(const String& message) {
    if (logger_) {
        logger_->log(message);
    }
}

bool Storage::initSD() {
    // Initialize SPI pins for SD card with Core2 configuration
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);
    
    // Delay to stabilize SD card power supply (required at startup)
    delay(100);
    
    // Attempt initialization with reduced frequency for compatibility
    log("Attempting SD card initialization...");
    
    // First attempt with 4MHz (good speed/compatibility compromise)
    if (!SD.begin(SPI_CS, SPI, 4000000)) {
        log("Failed at 4MHz, trying 1MHz...");
        
        // Second attempt with 1MHz (very slow but very compatible)
        if (!SD.begin(SPI_CS, SPI, 1000000)) {
            log("Micro SD card failed to initialise \"Err-4\"");
            log("Check: 1) SD card inserted correctly 2) SD card not corrupted 3) FAT32 format");
            return false;
        }
    }
    
    log("SD card initialized OK");
    sdInitialized_ = true;
    
    // Create log directory if it doesn't exist
    if (!SD.exists("/replay")) {
        SD.mkdir("/replay");
        log("/replay directory created");
    }
    
    return true;
}

String Storage::generateFileName() {
    // Generate a filename based on startup timestamp
    // Format: /replay/replay_[millis].json
    // Example: /replay/replay_1234567890.json
    unsigned long timestamp = millis();
    return "/replay/replay_" + String(timestamp) + ".json";
}

bool Storage::writeData(const StorageData& data) {
    // Check that SD card is initialized
    if (!sdInitialized_) {
        log("SD card not initialized");
        return false;
    }
    
    // Open file in append mode to add data at the end
    File file = SD.open(currentFileName_, FILE_APPEND);
    if (!file) {
        log("Error opening file: " + currentFileName_);
        return false;
    }
    
    // Create JSON object with appropriate size for the data
    DynamicJsonDocument doc(512);
    doc["timestamp"] = data.timestamp;
    
    // Serialize boat data into nested JSON object
    JsonObject boat = doc.createNestedObject("boat");
    boat["messageType"] = data.boatData.messageType;
    boat["latitude"] = data.boatData.latitude;
    boat["longitude"] = data.boatData.longitude;
    boat["speed"] = data.boatData.speed;
    boat["heading"] = data.boatData.heading;
    boat["satellites"] = data.boatData.satellites;
    boat["isGPSRecording"] = data.boatData.isGPSRecording;
    
    // Serialize anemometer data into nested JSON object
    JsonObject anemometer = doc.createNestedObject("anemometer");
    anemometer["messageType"] = data.anemometerData.messageType;
    anemometer["anemometerId"] = data.anemometerData.anemometerId;
    anemometer["windSpeed"] = data.anemometerData.windSpeed;
    
    // Write JSON to file followed by newline
    serializeJson(doc, file);
    file.println(); // Newline to separate JSON entries
    
    file.close();
    return true;
}

bool Storage::writeDataBatch(const std::vector<StorageData>& dataList) {
    // Preliminary checks
    if (!sdInitialized_ || dataList.empty()) {
        if (!sdInitialized_) {
            log("SD card not initialized for batch write");
        }
        return false;
    }
    
    // Open file in append mode for batch writing
    File file = SD.open(currentFileName_, FILE_APPEND);
    if (!file) {
        log("Error opening file for batch write: " + currentFileName_);
        return false;
    }
    
    // Write all data in a single file operation
    // This reduces latency and SD card wear
    for (const auto& data : dataList) {
        // Create JSON document for each entry
        DynamicJsonDocument doc(512);
        doc["timestamp"] = data.timestamp;
        
        // Serialize boat data
        JsonObject boat = doc.createNestedObject("boat");
        boat["messageType"] = data.boatData.messageType;
        boat["latitude"] = data.boatData.latitude;
        boat["longitude"] = data.boatData.longitude;
        boat["speed"] = data.boatData.speed;
        boat["heading"] = data.boatData.heading;
        boat["satellites"] = data.boatData.satellites;
        boat["isGPSRecording"] = data.boatData.isGPSRecording;
        
        // Serialize anemometer data
        JsonObject anemometer = doc.createNestedObject("anemometer");
        anemometer["messageType"] = data.anemometerData.messageType;
        anemometer["anemometerId"] = data.anemometerData.anemometerId;
        anemometer["windSpeed"] = data.anemometerData.windSpeed;
        
        // Write JSON to file with line separator
        serializeJson(doc, file);
        file.println();
    }
    
    file.close();
    log("Batch of " + String(dataList.size()) + " entries written to SD");
    return true;
}