# FileServerManager Class Documentation

## Overview

The `FileServerManager` class provides HTTP-based access to SD card files through a web browser interface. It manages WiFi connectivity, HTTP server operations, and seamless switching between ESPNow and WiFi modes for the OpenSailingRC-Display system running on M5Stack Core2.

## Architecture

```
FileServerManager
├── WiFi Configuration Management
├── HTTP Web Server (Port 80)
├── File System Interface
├── Web UI Generation
└── ESPNow/WiFi Mode Switching
```

## Features

### Core Functionality
- **WiFi Configuration Loading**: Reads credentials from `wifi_config.json` on SD card
- **HTTP Web Server**: Provides web interface on port 80
- **File Browsing**: Lists files and directories from SD card
- **File Download**: Serves files with appropriate MIME types
- **Mode Switching**: Seamless transition between ESPNow and WiFi modes
- **Responsive Web UI**: Mobile-friendly interface with icons and styling

### WiFi Management
- **Automatic Connection**: Connects to configured WiFi network
- **Connection Validation**: Verifies network connectivity before starting server
- **Clean Disconnection**: Returns to ESPNow mode when server stops
- **Error Handling**: Comprehensive error reporting for connection issues

## Class Structure

### WiFiConfig Structure
```cpp
struct WiFiConfig {
    String ssid;        // WiFi network name
    String password;    // WiFi network password
    bool isValid;       // Configuration validity flag
};
```

### Private Members
- `Logger* logger_`: Logging system pointer
- `WebServer* webServer_`: HTTP server instance (port 80)
- `bool serverActive_`: Server running status
- `bool sdInitialized_`: SD card accessibility status
- `bool wifiConnected_`: WiFi connection status
- `WiFiConfig wifiConfig_`: WiFi credentials storage

## API Reference

### Initialization Methods

#### `FileServerManager()`
**Purpose**: Constructor that initializes member variables and sets up static instance pointer.

**Implementation**:
- Sets all boolean flags to false
- Initializes pointers to nullptr
- Registers static instance for HTTP callbacks

#### `bool initFileServer()`
**Purpose**: Initialize the HTTP server without starting it.

**Process**:
1. Verify SD card accessibility
2. Create WebServer instance on port 80
3. Register HTTP route handlers:
   - `/` → Main page (handleRoot)
   - `/list` → File listing (handleFileList)
   - `/download` → File download (handleFileDownload)
   - `404` → Error page (handleNotFound)

**Returns**: `true` if initialization succeeds, `false` otherwise

### Server Control Methods

#### `bool startFileServer()`
**Purpose**: Start the HTTP file server with WiFi connection.

**Process**:
1. Verify server initialization
2. Load WiFi configuration from SD card
3. Connect to WiFi network
4. Start HTTP server
5. Log server URL with IP address

**Returns**: `true` if server starts successfully, `false` otherwise

**Note**: Switches system from ESPNow to WiFi mode

#### `bool stopFileServer()`
**Purpose**: Stop the HTTP server and return to ESPNow mode.

**Process**:
1. Stop HTTP server
2. Disconnect from WiFi
3. Set WiFi mode to OFF
4. Reset server status flags

**Returns**: `true` if server stops successfully, `false` otherwise

### WiFi Management Methods

#### `bool loadWiFiConfig()`
**Purpose**: Load WiFi credentials from SD card configuration file.

**Configuration File**: `/wifi_config.json`
```json
{
  "ssid": "YourNetworkName",
  "password": "YourPassword"
}
```

**Process**:
1. Check for config file existence
2. Parse JSON content
3. Validate required fields (ssid, password)
4. Store credentials in wifiConfig_ structure

**Returns**: `true` if configuration loads successfully, `false` otherwise

#### `bool connectToWiFi()`
**Purpose**: Establish WiFi connection using loaded credentials.

**Process**:
1. Load WiFi config if not already loaded
2. Set WiFi mode to Station (WIFI_STA)
3. Begin connection with credentials
4. Wait up to 10 seconds for connection (20 attempts × 500ms)
5. Verify connection status

**Returns**: `true` if connection succeeds, `false` otherwise

#### `void disconnectWiFi()`
**Purpose**: Disconnect from WiFi and disable WiFi mode.

**Process**:
1. Call WiFi.disconnect()
2. Set WiFi mode to OFF
3. Update connection status flag
4. Log disconnection status

### HTTP Request Handlers

#### `void handleRoot()`
**Purpose**: Serve the main web interface page.

**Features**:
- Responsive HTML design
- File browser interface
- Navigation breadcrumbs
- Modern styling with CSS
- Mobile-friendly layout

#### `void handleFileList()`
**Purpose**: Generate JSON file listing for AJAX requests.

**Output Format**:
```json
{
  "files": [
    {
      "name": "filename.txt",
      "size": 1024,
      "type": "file"
    },
    {
      "name": "subdirectory",
      "type": "directory"
    }
  ]
}
```

#### `void handleFileDownload()`
**Purpose**: Serve file downloads with appropriate MIME types.

**Supported Types**:
- `.json` → `application/json`
- `.txt` → `text/plain`
- `.csv` → `text/csv`
- Others → `application/octet-stream`

**Process**:
1. Validate file path parameter
2. Check file existence and type
3. Set appropriate Content-Type header
4. Stream file content to client

#### `void handleNotFound()`
**Purpose**: Serve 404 error page for invalid requests.

**Features**:
- Styled error page
- Navigation link back to home
- Clear error message

### Utility Methods

#### `void handleClient()`
**Purpose**: Process incoming HTTP requests.

**Usage**: Must be called regularly in main loop when server is active.

**Implementation**: Delegates to `webServer_->handleClient()`

#### `String getServerIP()`
**Purpose**: Get the server's IP address for client access.

**Returns**: 
- IP address string if WiFi connected
- "Not connected" if WiFi disconnected

#### `void setLogger(Logger& logger)`
**Purpose**: Configure logging system for debug messages.

#### `void log(const String& message)`
**Purpose**: Send messages to configured logging system.

## Usage Examples

### Basic Initialization
```cpp
FileServerManager fileServer;
Logger logger;

// Setup logging
fileServer.setLogger(logger);

// Initialize server (doesn't start it)
if (!fileServer.initFileServer()) {
    logger.log("Failed to initialize file server");
}
```

### Starting/Stopping Server
```cpp
// Start server (connects WiFi and starts HTTP)
if (fileServer.startFileServer()) {
    String ip = fileServer.getServerIP();
    logger.log("Server running at: http://" + ip);
} else {
    logger.log("Failed to start file server");
}

// In main loop
if (fileServer.isServerActive()) {
    fileServer.handleClient();
}

// Stop server (disconnects WiFi, returns to ESPNow)
fileServer.stopFileServer();
```

### Status Checking
```cpp
bool serverRunning = fileServer.isServerActive();
bool wifiConnected = fileServer.isWiFiConnected();
String serverURL = fileServer.getServerIP();
```

## Configuration Requirements

### WiFi Configuration File
**Location**: `/wifi_config.json` (SD card root)

**Format**:
```json
{
  "ssid": "YourWiFiNetwork",
  "password": "YourPassword123"
}
```

**Requirements**:
- Valid JSON syntax
- Both "ssid" and "password" fields required
- Network must be 2.4GHz (ESP32 limitation)
- Network must be accessible from device location

### SD Card Requirements
- FAT32 format
- Readable file system
- Sufficient space for configuration file
- Proper insertion in M5Stack Core2

## Web Interface Features

### Main Page
- **File Browser**: Hierarchical file/folder view
- **Breadcrumb Navigation**: Easy path navigation
- **Download Links**: Direct file download
- **Responsive Design**: Works on mobile devices
- **Modern Styling**: Clean, professional appearance

### File Operations
- **Directory Navigation**: Click folders to browse
- **File Download**: Click files to download
- **Type Recognition**: Different icons for file types
- **Size Display**: File sizes in human-readable format

### Browser Compatibility
- **Modern Browsers**: Chrome, Firefox, Safari, Edge
- **Mobile Browsers**: iOS Safari, Android Chrome
- **JavaScript**: AJAX for dynamic file listing
- **CSS3**: Modern styling and responsive layout

## Error Handling

### Common Error Scenarios

#### WiFi Configuration Errors
- **Missing File**: `wifi_config.json` not found on SD card
- **Invalid JSON**: Malformed JSON syntax
- **Missing Fields**: "ssid" or "password" keys missing
- **Network Issues**: WiFi network not accessible

#### Server Startup Errors
- **SD Card Not Accessible**: File system not mounted
- **Server Not Initialized**: `initFileServer()` not called
- **WiFi Connection Failed**: Network connection timeout
- **Port Conflict**: Port 80 already in use (rare)

#### Runtime Errors
- **File Access Errors**: File permissions or corruption
- **Network Disconnection**: WiFi connection lost
- **Memory Issues**: Insufficient heap for web server

### Error Messages
All error messages are logged through the configured Logger system:
- Clear, descriptive error descriptions
- Suggested troubleshooting steps
- Status codes for programmatic handling

## Performance Considerations

### Memory Usage
- **WebServer Instance**: ~2KB heap allocation
- **WiFi Buffer**: ~1KB for connection management
- **HTTP Buffers**: Variable based on file sizes
- **JSON Parsing**: ~512 bytes for config parsing

### Network Performance
- **Connection Time**: 2-10 seconds for WiFi connection
- **HTTP Response**: <100ms for file listings
- **File Transfer**: Limited by WiFi bandwidth and SD card speed
- **Concurrent Clients**: Supports multiple browser connections

### SD Card Access
- **File Reading**: Streaming for large files
- **Directory Listing**: Efficient recursive scanning
- **Cache**: No file caching (always current data)

## Integration with OpenSailingRC System

### ESPNow Compatibility
- **Mode Switching**: Clean transition between ESPNow and WiFi
- **State Preservation**: Navigation data continues when possible
- **Quick Resume**: Fast return to ESPNow mode

### Touch Button Integration
- **Activation**: Right touch button starts/stops server
- **Visual Feedback**: Button color indicates server status
- **Status Display**: IP address shown on M5Stack screen

### Logging Integration
- **Centralized Logging**: Uses system Logger instance
- **Debug Messages**: Detailed operation logging
- **Error Reporting**: Clear error message reporting

## Security Considerations

### Network Security
- **Open Protocol**: HTTP (not HTTPS) for simplicity
- **Local Network**: Designed for local WiFi access only
- **No Authentication**: Direct file access without passwords
- **File System Access**: Limited to SD card content only

### Recommendations
- **Secure Network**: Use on trusted WiFi networks only
- **Temporary Access**: Enable only when needed
- **File Content**: Avoid sensitive data in accessible files
- **Network Isolation**: Consider isolated WiFi network for boat systems

## Troubleshooting Guide

### Server Won't Start
1. **Check SD Card**: Ensure proper insertion and FAT32 format
2. **Verify Config**: Validate `wifi_config.json` syntax and content
3. **Test Network**: Confirm WiFi network is accessible
4. **Check Initialization**: Ensure `initFileServer()` was called successfully

### Can't Access Web Interface
1. **Note IP Address**: Check M5Stack display for server IP
2. **Network Connection**: Ensure device is on same WiFi network
3. **Browser Cache**: Clear browser cache and refresh
4. **Firewall**: Check for blocking software on accessing device

### Files Not Visible
1. **SD Card Content**: Verify files exist on SD card
2. **File Permissions**: Ensure files are readable
3. **Path Issues**: Check for special characters in filenames
4. **Refresh Browser**: Reload page to update file listing

### Performance Issues
1. **WiFi Signal**: Improve WiFi signal strength
2. **File Size**: Large files may transfer slowly
3. **Multiple Clients**: Limit concurrent browser connections
4. **SD Card Speed**: Use Class 10 or faster SD cards

## Future Enhancement Possibilities

### Security Improvements
- HTTPS support with SSL certificates
- Basic authentication system
- File access permissions
- Upload functionality with validation

### Performance Optimizations
- File caching for faster access
- Compressed file transfer
- Background file scanning
- Progressive loading for large directories

### Feature Additions
- File upload capability
- Real-time log viewing
- Configuration file editing
- System status monitoring
- Remote system control interface