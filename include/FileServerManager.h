/**
 * @file FileServerManager.h
 * @brief HTTP file server manager for SD card access via WiFi
 * 
 * This class manages an HTTP web server that allows remote access to SD card files
 * through a web browser. It handles WiFi connection, HTTP requests, and file serving
 * with automatic switching between ESPNow and WiFi modes.
 * 
 * @author Philippe Hubert
 * @date 2025
 */

#pragma once
#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Forward declaration
class Logger;

/**
 * @struct WiFiConfig
 * @brief WiFi configuration structure
 * 
 * Contains WiFi credentials loaded from SD card configuration file.
 * Used to establish network connection for the HTTP file server.
 */
struct WiFiConfig {
    String ssid;        ///< WiFi network SSID
    String password;    ///< WiFi network password
    bool isValid;       ///< Indicates if configuration is valid
    
    WiFiConfig() : isValid(false) {}
};

/**
 * @class FileServerManager
 * @brief HTTP file server for remote SD card access
 * 
 * This class provides HTTP-based access to SD card files through a web interface.
 * It automatically manages WiFi connection and provides a web browser interface
 * for browsing and downloading files stored on the SD card.
 * 
 * Features:
 * - WiFi configuration loading from SD card (wifi_config.json)
 * - HTTP web server with file browsing interface
 * - File download capabilities
 * - Automatic WiFi connection/disconnection
 * - ESPNow/WiFi mode switching
 * - Responsive web interface with file type recognition
 * 
 * Usage workflow:
 * 1. Load WiFi credentials from SD card
 * 2. Connect to WiFi network
 * 3. Start HTTP server on port 80
 * 4. Serve web interface for file access
 * 5. Disconnect WiFi and return to ESPNow mode when stopped
 */

class FileServerManager {
private:
    Logger* logger_;           ///< Pointer to logging system
    WebServer* webServer_;     ///< HTTP web server instance
    bool serverActive_;        ///< Server running status
    bool sdInitialized_;      ///< SD card initialization status
    bool wifiConnected_;      ///< WiFi connection status
    WiFiConfig wifiConfig_;   ///< WiFi configuration data
    
    // HTTP request handlers
    void handleRoot();         ///< Handle root URL (main page)
    void handleFileList();     ///< Handle file listing requests
    void handleFileDownload(); ///< Handle file download requests
    void handleNotFound();     ///< Handle 404 errors
    
    // WiFi management methods
    bool loadWiFiConfig();     ///< Load WiFi config from SD card
    bool connectToWiFi();      ///< Connect to WiFi network
    void disconnectWiFi();     ///< Disconnect from WiFi
    
    // Static instance for callbacks
    static FileServerManager* instance_;
    
public:
    /**
     * @brief FileServerManager constructor
     * 
     * Initializes member variables and sets up the static instance
     * pointer for HTTP callback functions.
     */
    FileServerManager();
    
    /**
     * @brief FileServerManager destructor
     * 
     * Cleans up the web server instance if allocated.
     */
    ~FileServerManager();
    
    /**
     * @brief Configure the logging system
     * @param logger Reference to Logger instance
     * 
     * Sets up logging for debug and status messages throughout
     * the file server operations.
     */
    void setLogger(Logger& logger);
    
    /**
     * @brief Initialize the file server (without starting it)
     * @return true if initialization succeeds, false otherwise
     * 
     * This method:
     * - Verifies SD card accessibility
     * - Creates WebServer instance on port 80
     * - Registers HTTP route handlers
     * - Does NOT start the server or connect WiFi
     * 
     * @note Call this during system initialization
     */
    bool initFileServer();
    
    /**
     * @brief Start the HTTP file server
     * @return true if server starts successfully, false otherwise
     * 
     * This method:
     * - Loads WiFi configuration from SD card
     * - Connects to WiFi network
     * - Starts the HTTP server
     * - Makes the server accessible via web browser
     * 
     * @note Switches from ESPNow to WiFi mode
     */
    bool startFileServer();
    
    /**
     * @brief Stop the HTTP file server
     * @return true if server stops successfully, false otherwise
     * 
     * This method:
     * - Stops the HTTP server
     * - Disconnects from WiFi
     * - Returns system to ESPNow mode
     * 
     * @note Allows return to normal ESPNow operation
     */
    bool stopFileServer();
    
    /**
     * @brief Check if server is currently active
     * @return true if server is running, false otherwise
     */
    bool isServerActive() const { return serverActive_; }
    
    /**
     * @brief Check if WiFi is currently connected
     * @return true if WiFi is connected, false otherwise
     */
    bool isWiFiConnected() const { return wifiConnected_; }
    
    /**
     * @brief Send message to logging system
     * @param message Message to log
     * 
     * Utility method for sending debug messages through the
     * configured Logger. Safe to call without configured logger.
     */
    void log(const String& message);
    
    /**
     * @brief Handle incoming HTTP client requests
     * 
     * Must be called regularly in the main loop when server is active
     * to process incoming HTTP requests and serve responses.
     * 
     * @note Call this in loop() when isServerActive() returns true
     */
    void handleClient();
    
    /**
     * @brief Get the server's IP address
     * @return IP address as string, or "Not connected" if WiFi disconnected
     * 
     * Returns the local IP address assigned by the WiFi network.
     * This address is used to access the web interface from browsers.
     */
    String getServerIP();
};