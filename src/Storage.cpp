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
    // Delegate to writeDataBatch for consistent Kepler-compatible format
    std::vector<StorageData> batch;
    batch.push_back(data);
    return writeDataBatch(batch);
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
    
    // Read Display RTC once for consistent timestamps across all entries
    auto dt = M5.Rtc.getDateTime();
    unsigned long millisNow = millis();
    
    // Convert RTC datetime to epoch for arithmetic
    struct tm rtcTm = {};
    rtcTm.tm_year = dt.date.year - 1900;
    rtcTm.tm_mon = dt.date.month - 1;
    rtcTm.tm_mday = dt.date.date;
    rtcTm.tm_hour = dt.time.hours;
    rtcTm.tm_min = dt.time.minutes;
    rtcTm.tm_sec = dt.time.seconds;
    rtcTm.tm_isdst = -1;
    time_t rtcEpoch = mktime(&rtcTm);
    
    // Open file as proper JSON array: [{...},{...},...]
    // First batch creates file with "[", subsequent batches
    // seek before closing "]" and append with ","
    File file;
    bool isNewFile = true;
    
    if (SD.exists(currentFileName_)) {
        // Try opening in read+write mode to update existing array
        file = SD.open(currentFileName_, "r+");
        if (file && file.size() >= 3) {
            isNewFile = false;
            // Seek before closing "\n]" (2 bytes from end)
            file.seek(file.size() - 2);
            file.print(",\n");
        } else {
            // File exists but is too small/corrupt - recreate
            if (file) file.close();
            file = SD.open(currentFileName_, FILE_WRITE);
            if (!file) {
                log("Error creating file: " + currentFileName_);
                return false;
            }
            file.print("[\n");
        }
    } else {
        // New file - start JSON array
        file = SD.open(currentFileName_, FILE_WRITE);
        if (!file) {
            log("Error creating file: " + currentFileName_);
            return false;
        }
        file.print("[\n");
    }
    
    // Write all entries as flat Kepler-compatible JSON objects
    for (size_t i = 0; i < dataList.size(); i++) {
        if (i > 0) file.print(",\n");
        
        const auto& data = dataList[i];
        
        // Compute entry's epoch timestamp from millis offset vs Display RTC
        long offsetSec = (long)((millisNow - data.timestamp) / 1000);
        time_t entryEpoch = rtcEpoch - offsetSec;
        
        // Create flat JSON document (Kepler-compatible: lat/lon at top level)
        JsonDocument doc;
        doc["datetime"] = (long long)entryEpoch;
        
        if (data.dataType == DATA_TYPE_BOAT) {
            doc["device_type"] = "boat";
            doc["device_name"] = data.boatData.name;
            doc["latitude"] = data.boatData.latitude;
            doc["longitude"] = data.boatData.longitude;
            doc["speed"] = data.boatData.speed;
            doc["heading"] = data.boatData.heading;
            doc["satellites"] = data.boatData.satellites;
            doc["sequenceNumber"] = data.boatData.sequenceNumber;
        } else if (data.dataType == DATA_TYPE_ANEMOMETER) {
            doc["device_type"] = "anemometer";
            doc["device_name"] = data.anemometerData.anemometerId;
            doc["windSpeed"] = data.anemometerData.windSpeed;
            doc["windDirection"] = data.windDirection;
            doc["sequenceNumber"] = data.anemometerData.sequenceNumber;
        } else if (data.dataType == DATA_TYPE_BUOY) {
            doc["device_type"] = "buoy";
            String buoyName = "Buoy_" + String(data.buoyData.buoyId);
            doc["device_name"] = buoyName;
            doc["latitude"] = data.buoyData.latitude;
            doc["longitude"] = data.buoyData.longitude;
            doc["autoPilotThrottleCmde"] = data.buoyData.autoPilotThrottleCmde;
            doc["autoPilotTrueHeadingCmde"] = data.buoyData.autoPilotTrueHeadingCmde;
        }
        
        serializeJson(doc, file);
    }
    
    // Close the JSON array - file is always a valid JSON
    file.print("\n]");
    file.close();
    log("Batch of " + String(dataList.size()) + " Kepler entries written to SD");
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