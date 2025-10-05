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
#include <M5Unified.h>
#include <WiFi.h>
#include <time.h>

// SPI pin configuration for SD card (M5Stack Core2)
#define SPI_SCK  18  ///< Serial Clock pin
#define SPI_MISO 38  ///< Master In Slave Out pin (incoming data)
#define SPI_MOSI 23  ///< Master Out Slave In pin (outgoing data)  
#define SPI_CS   4   ///< Chip Select pin (SD card selection)

Storage::Storage() : logger_(nullptr), sdInitialized_(false) {
    // Filename will be generated later when RTC is initialized
    currentFileName_ = "";
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

bool Storage::initializeFileName() {
    if (currentFileName_.isEmpty()) {
        currentFileName_ = generateFileName();
        log("Filename initialized: " + currentFileName_);
        return true;
    }
    return false; // Already initialized
}

String Storage::generateFileName() {
    // Generate a filename based on RTC timestamp
    // Format: /replay/YYYY-MM-DD_HH-MM-SS.json
    // Example: /replay/2025-09-21_14-30-45.json
    
    auto dt = M5.Rtc.getDateTime();
    
    // If RTC is not set (year < 2023), use unique identifier as fallback
    if (dt.date.year < 2023) {
        // Get MAC address for uniqueness
        String mac = WiFi.macAddress();
        mac.replace(":", "");  // Remove colons
        
        // Get last 4 characters of MAC for shorter name
        String macSuffix = mac.substring(mac.length() - 4);
        
        // Find next available session number
        int sessionNumber = 1;
        String baseName = "/replay/session_" + macSuffix + "_";
        
        // Check for existing files and increment session number
        while (sessionNumber < 1000) {
            String testName = baseName + String(sessionNumber) + ".json";
            if (!SD.exists(testName)) {
                break;
            }
            sessionNumber++;
        }
        
        return baseName + String(sessionNumber) + ".json";
    }
    
    // Format with zero padding: YYYY-MM-DD_HH-MM-SS
    char filename[40];
    snprintf(filename, sizeof(filename), "/replay/%04d-%02d-%02d_%02d-%02d-%02d.json",
             dt.date.year, dt.date.month, dt.date.date,
             dt.time.hours, dt.time.minutes, dt.time.seconds);
    
    return String(filename);
}

bool Storage::writeData(const StorageData& data) {
    // Check that SD card is initialized
    if (!sdInitialized_) {
        log("SD card not initialized");
        return false;
    }
    
    // Initialize filename if not already done
    if (currentFileName_.isEmpty()) {
        if (!initializeFileName()) {
            log("Error: Unable to initialize filename");
            return false;
        }
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
    doc["type"] = data.dataType;
    
    // Serialize data based on type
    if (data.dataType == DATA_TYPE_BOAT) {
        // Serialize boat data into nested JSON object
        JsonObject boat = doc.createNestedObject("boat");
        boat["messageType"] = data.boatData.messageType;
        boat["gpsTimestamp"] = data.boatData.gpsTimestamp;
        boat["latitude"] = data.boatData.latitude;
        boat["longitude"] = data.boatData.longitude;
        boat["speed"] = data.boatData.speed;
        boat["heading"] = data.boatData.heading;
        boat["satellites"] = data.boatData.satellites;
        boat["isGPSRecording"] = data.boatData.isGPSRecording;
    } else if (data.dataType == DATA_TYPE_ANEMOMETER) {
        // Serialize anemometer data into nested JSON object
        JsonObject anemometer = doc.createNestedObject("anemometer");
        anemometer["messageType"] = data.anemometerData.messageType;
        anemometer["anemometerId"] = data.anemometerData.anemometerId;
        anemometer["windSpeed"] = data.anemometerData.windSpeed;
        
        // Add MAC address for device identification
        JsonArray macArray = anemometer.createNestedArray("macAddress");
        for (int i = 0; i < 6; i++) {
            macArray.add(data.anemometerData.macAddress[i]);
        }
    }
    
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
    
    // Initialize filename if not already done
    if (currentFileName_.isEmpty()) {
        if (!initializeFileName()) {
            log("Error: Unable to initialize filename for batch write");
            return false;
        }
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
        doc["type"] = data.dataType;
        
        // Serialize data based on type
        if (data.dataType == DATA_TYPE_BOAT) {
            // Serialize boat data into nested JSON object
            JsonObject boat = doc.createNestedObject("boat");
            boat["messageType"] = data.boatData.messageType;
            boat["gpsTimestamp"] = data.boatData.gpsTimestamp;
            boat["latitude"] = data.boatData.latitude;
            boat["longitude"] = data.boatData.longitude;
            boat["speed"] = data.boatData.speed;
            boat["heading"] = data.boatData.heading;
            boat["satellites"] = data.boatData.satellites;
            boat["isGPSRecording"] = data.boatData.isGPSRecording;
        } else if (data.dataType == DATA_TYPE_ANEMOMETER) {
            // Serialize anemometer data into nested JSON object
            JsonObject anemometer = doc.createNestedObject("anemometer");
            anemometer["messageType"] = data.anemometerData.messageType;
            anemometer["anemometerId"] = data.anemometerData.anemometerId;
            anemometer["windSpeed"] = data.anemometerData.windSpeed;
            
            // Add MAC address for device identification
            JsonArray macArray = anemometer.createNestedArray("macAddress");
            for (int i = 0; i < 6; i++) {
                macArray.add(data.anemometerData.macAddress[i]);
            }
        }
        
        // Write JSON to file with line separator
        serializeJson(doc, file);
        file.println();
    }
    
    file.close();
    log("Batch of " + String(dataList.size()) + " entries written to SD");
    return true;
}

bool Storage::syncRTCFromNTP(const char* ntpServer, long gmtOffset, int daylightOffset) {
    // Check if WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
        log("WiFi not connected - cannot sync RTC");
        return false;
    }
    
    log("Synchronizing RTC with NTP server: " + String(ntpServer));
    
    // Configure time server
    configTime(gmtOffset, daylightOffset, ntpServer);
    
    // Wait for time synchronization (timeout after 10 seconds)
    unsigned long startTime = millis();
    time_t now = 0;
    
    while (now < 1000000000 && (millis() - startTime) < 10000) {
        time(&now);
        delay(100);
    }
    
    if (now < 1000000000) {
        log("Failed to synchronize with NTP server");
        return false;
    }
    
    // Convert to local time structure
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        log("Failed to get local time structure");
        return false;
    }
    
    // Set RTC with NTP time
    m5::rtc_datetime_t datetime;
    datetime.date.year = timeinfo.tm_year + 1900;
    datetime.date.month = timeinfo.tm_mon + 1;
    datetime.date.date = timeinfo.tm_mday;
    datetime.time.hours = timeinfo.tm_hour;
    datetime.time.minutes = timeinfo.tm_min;
    datetime.time.seconds = timeinfo.tm_sec;
    
    M5.Rtc.setDateTime(datetime);
    
    log("RTC synchronized successfully: " + 
        String(datetime.date.year) + "-" + 
        String(datetime.date.month) + "-" + 
        String(datetime.date.date) + " " +
        String(datetime.time.hours) + ":" + 
        String(datetime.time.minutes) + ":" + 
        String(datetime.time.seconds));
    
    return true;
}

time_t Storage::getCurrentTimestamp() {
    // Get current time from M5Stack Core2 RTC
    auto datetime = M5.Rtc.getDateTime();
    
    // Check if RTC has valid time (year >= 2023)
    if (datetime.date.year < 2023) {
        log("RTC not set or invalid time - returning 0");
        return 0;
    }
    
    // Convert to Unix timestamp
    struct tm timeinfo;
    timeinfo.tm_year = datetime.date.year - 1900;  // tm_year is years since 1900
    timeinfo.tm_mon = datetime.date.month - 1;     // tm_mon is 0-11
    timeinfo.tm_mday = datetime.date.date;
    timeinfo.tm_hour = datetime.time.hours;
    timeinfo.tm_min = datetime.time.minutes;
    timeinfo.tm_sec = datetime.time.seconds;
    timeinfo.tm_isdst = -1;  // Let mktime determine DST
    
    time_t timestamp = mktime(&timeinfo);
    
    if (timestamp == -1) {
        log("Error converting RTC time to timestamp");
        return 0;
    }
    
    return timestamp;
}