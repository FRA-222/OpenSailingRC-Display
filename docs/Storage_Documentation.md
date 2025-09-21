# Storage Class Documentation

## Overview

The `Storage` class is responsible for storing navigation data (GPS and anemometer) to SD card in JSON format. It is part of the OpenSailingRC-Display system and works specifically with the M5Stack Core2.

## Architecture

```
Storage
├── SPI Configuration (Core2 specific)
├── JSON file management
├── Single and batch writing
└── Logging system integration
```

## Hardware Configuration

### SPI Pins (M5Stack Core2)
- **SCK (Serial Clock)**: GPIO 18
- **MISO (Master In Slave Out)**: GPIO 38
- **MOSI (Master Out Slave In)**: GPIO 23
- **CS (Chip Select)**: GPIO 4

### SD Card Requirements
- Format: **FAT32**
- Capacity: Up to 32GB recommended
- Class: Class 10 or higher for better performance

## Data Structure

### StorageData
```cpp
struct StorageData {
    unsigned long timestamp;                    // Timestamp in milliseconds
    struct_message_Boat boatData;              // Boat GPS data
    struct_message_Anemometer anemometerData;  // Anemometer data
};
```

### JSON Output Format
```json
{
  "timestamp": 1234567890,
  "boat": {
    "messageType": 1,
    "latitude": 43.123456,
    "longitude": 2.654321,
    "speed": 5.2,
    "heading": 280,
    "satellites": 8,
    "isGPSRecording": true
  },
  "anemometer": {
    "messageType": 2,
    "anemometerId": 1,
    "windSpeed": 12.5
  }
}
```

## Class API

### Constructor
```cpp
Storage()
```
- Initializes member variables
- Automatically generates a unique filename

### Main Methods

#### `bool initSD()`
Initializes SD card with optimized configuration for Core2.

**Initialization Algorithm:**
1. SPI pin configuration
2. Stabilization (100ms delay)
3. Attempt at 4MHz
4. If failed, attempt at 1MHz
5. Create `/replay/` directory

**Returns:** `true` if successful, `false` if failed

#### `bool writeData(const StorageData& data)`
Writes a single data entry to SD card.

**Usage:**
- Real-time writing
- One JSON entry per line
- FILE_APPEND mode

#### `bool writeDataBatch(const std::vector<StorageData>& dataList)`
Writes a batch of data in a single operation.

**Advantages:**
- Reduced latency
- Less SD card wear
- Better performance for accumulated data

#### `void setLogger(Logger& logger)`
Configures the logging system for debug messages.

#### `String generateFileName()`
Generates a unique filename in format `/replay/replay_[timestamp].json`.

## Typical Usage

### Initialization
```cpp
Storage storage;
Logger logger;

// Configure logging
storage.setLogger(logger);

// Initialize SD card
if (!storage.initSD()) {
    Serial.println("SD initialization error");
    return;
}
```

### Data Writing
```cpp
// Single write
StorageData data;
data.timestamp = millis();
data.boatData = currentBoatData;
data.anemometerData = currentAnemometerData;

if (!storage.writeData(data)) {
    Serial.println("Write error");
}

// Batch write (recommended)
std::vector<StorageData> dataList;
// ... fill dataList ...

if (!storage.writeDataBatch(dataList)) {
    Serial.println("Batch write error");
}
```

## Error Handling

### Common Error Messages
- `"Micro SD card failed to initialise \"Err-4\""`: SD card not detected
- `"SD card not initialized"`: Write methods called before initSD()
- `"Error opening file"`: File access problem

### Diagnostics
1. **SD card not recognized:**
   - Check physical insertion
   - Verify FAT32 format
   - Test with another card

2. **Write error:**
   - Check available space
   - Check permissions
   - SD card may be corrupted

## File Organization

### Directory Structure
```
/
└── replay/
    ├── replay_1234567890.json
    ├── replay_1234567891.json
    └── replay_1234567892.json
```

### Filename Format
- Format: `replay_[timestamp].json`
- Timestamp based on `millis()` at startup
- Guarantees uniqueness even during rapid restarts

## Performance

### Implemented Optimizations
- **Batch writing**: Reduces file operations
- **Adaptive SPI frequency**: 4MHz then 1MHz if necessary
- **Line-by-line JSON format**: Facilitates parsing
- **Fixed buffer size**: 512 bytes per JSON document

### Usage Recommendations
- Use `writeDataBatch()` for > 5 entries
- Limit write frequency (max 1Hz recommended)
- Monitor available disk space

## System Integration

### Dependencies
- `ArduinoJson`: JSON serialization
- `SD`: Arduino SD card interface
- `SPI`: SPI communication
- `Logger`: Logging system (optional)

### Used By
- Main navigation system
- Storage task (FreeRTOS)
- Data replay interface

## Limitations

### Hardware
- Compatible only with M5Stack Core2
- SD card max 32GB (FAT32 limitation)
- No power failure protection

### Software
- Single output file per session
- No data compression
- No data integrity validation

## Maintenance

### Recommended Tests
1. Initialization test with different SD cards
2. Continuous write test over long duration
3. Error recovery test
4. JSON integrity validation

### Possible Enhancements
- gzip compression support
- Automatic file rotation
- Integrity checksums
- Larger SD card support (exFAT)