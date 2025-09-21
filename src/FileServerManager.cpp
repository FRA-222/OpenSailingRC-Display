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
 * @file FileServerManager.cpp
 * @brief Implementation of HTTP file server for SD card access
 * 
 * This file contains the implementation of the FileServerManager class that
 * provides HTTP-based access to SD card files through a web interface.
 * 
 * @author Philippe Hubert
 * @date 2025
 */

#include "FileServerManager.h"
#include "Logger.h"

// Static instance for HTTP callbacks
FileServerManager* FileServerManager::instance_ = nullptr;

/**
 * @brief FileServerManager constructor
 * 
 * Initializes all member variables to their default states and sets up
 * the static instance pointer for HTTP callback functions.
 */
FileServerManager::FileServerManager() : logger_(nullptr), webServer_(nullptr), serverActive_(false), sdInitialized_(false), wifiConnected_(false) {
    instance_ = this;
}

/**
 * @brief FileServerManager destructor
 * 
 * Cleans up allocated resources, specifically the WebServer instance
 * if it was created during initialization.
 */
FileServerManager::~FileServerManager() {
    if (webServer_) {
        delete webServer_;
    }
}

/**
 * @brief Configure the logging system for debug messages
 * @param logger Reference to the Logger instance to use
 * 
 * Sets up the logging system to enable debug and status messages
 * throughout the file server operations.
 */
void FileServerManager::setLogger(Logger& logger) {
    logger_ = &logger;
}

/**
 * @brief Send a message to the configured logging system
 * @param message The message string to log
 * 
 * Utility method that safely sends messages to the Logger if one
 * has been configured. Does nothing if no Logger is set.
 */
void FileServerManager::log(const String& message) {
    if (logger_) {
        logger_->log(message);
    }
}

/**
 * @brief Initialize the HTTP file server without starting it
 * @return true if initialization succeeds, false otherwise
 * 
 * This method sets up the web server infrastructure by:
 * - Verifying SD card accessibility
 * - Creating WebServer instance on port 80
 * - Registering HTTP route handlers for /, /list, /download, and 404 errors
 * 
 * The server is initialized but not started - call startFileServer() to begin operation.
 */
bool FileServerManager::initFileServer() {
    log("Initializing HTTP file server...");
    
    // Verify that SD card is accessible
    if (!SD.exists("/")) {
        log("Error: SD card not accessible for file server");
        return false;
    }
    
    // Create web server on port 80
    webServer_ = new WebServer(80);
    
    // Register route handlers
    webServer_->on("/", [this]() { this->handleRoot(); });
    webServer_->on("/list", [this]() { this->handleFileList(); });
    webServer_->on("/download", [this]() { this->handleFileDownload(); });
    webServer_->onNotFound([this]() { this->handleNotFound(); });
    
    log("HTTP file server initialized");
    sdInitialized_ = true;
    return true;
}

/**
 * @brief Start the HTTP file server with WiFi connection
 * @return true if server starts successfully, false otherwise
 * 
 * This method performs the complete server startup sequence:
 * 1. Verifies server initialization
 * 2. Connects to WiFi using configuration from SD card
 * 3. Starts the HTTP server on port 80
 * 4. Logs the server URL for client access
 * 
 * Note: This switches the system from ESPNow to WiFi mode.
 */
bool FileServerManager::startFileServer() {
    if (!sdInitialized_) {
        log("Error: File server not initialized");
        return false;
    }
    
    // Connect to WiFi first
    if (!connectToWiFi()) {
        return false;
    }
    
    log("Starting HTTP file server...");
    
    webServer_->begin();
    serverActive_ = true;
    
    String ip = WiFi.localIP().toString();
    log("File server active at: http://" + ip);
    log("Access files from your web browser");
    
    return true;
}

/**
 * @brief Stop the HTTP file server and return to ESPNow mode
 * @return true if server stops successfully, false otherwise
 * 
 * This method performs a clean shutdown of the file server:
 * 1. Stops the HTTP server
 * 2. Disconnects from WiFi
 * 3. Returns the system to ESPNow mode for normal operation
 * 
 * The server can be restarted later by calling startFileServer() again.
 */
bool FileServerManager::stopFileServer() {
    if (!serverActive_) {
        return true; // Already stopped
    }
    
    log("Stopping HTTP file server...");
    
    webServer_->stop();
    serverActive_ = false;
    
    // Disconnect WiFi to return to ESPNow mode
    disconnectWiFi();
    
    log("File server stopped, returning to ESPNow mode");
    return true;
}

/**
 * @brief Handle incoming HTTP client requests
 * 
 * This method must be called regularly in the main loop when the server
 * is active to process incoming HTTP requests and serve responses.
 * It delegates to the underlying WebServer's handleClient() method.
 */
void FileServerManager::handleClient() {
    if (serverActive_ && webServer_) {
        webServer_->handleClient();
    }
}

/**
 * @brief Get the server's current IP address
 * @return IP address as string if connected, "Not connected" otherwise
 * 
 * Returns the local IP address assigned by the WiFi network.
 * This address is used by clients to access the web interface.
 */
String FileServerManager::getServerIP() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "Not connected";
}

/**
 * @brief Handle HTTP requests to the root URL (/)
 * 
 * Serves the main web interface page with:
 * - Responsive HTML design for desktop and mobile
 * - CSS styling for modern appearance
 * - JavaScript for dynamic file listing
 * - File browser interface with navigation
 * 
 * The page provides a complete file management interface accessible
 * from any web browser on the same network.
 */
void FileServerManager::handleRoot() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>M5Stack Core2 - Serveur de fichiers</title>";
    html += "<meta charset='UTF-8'>";
    html += "<style>body{font-family:Arial,sans-serif;margin:20px;}";
    html += "h1{color:#333;}ul{list-style-type:none;}";
    html += "li{margin:5px 0;}a{text-decoration:none;color:#0066cc;}";
    html += "a:hover{text-decoration:underline;}</style>";
    html += "</head><body>";
    html += "<h1>🚢 M5Stack Core2 - Serveur de fichiers GPS</h1>";
    html += "<p>Bienvenue sur le serveur de fichiers de votre M5Stack Core2.</p>";
    html += "<p>Vous pouvez télécharger les fichiers de replay GPS stockés sur la carte SD.</p>";
    html += "<h2>📁 Répertoires disponibles:</h2>";
    html += "<ul>";
    html += "<li>📂 <a href='/list?dir=/replay'>/replay</a> - Fichiers de replay GPS</li>";
    html += "<li>📂 <a href='/list?dir=/'>/</a> - Racine de la carte SD</li>";
    html += "</ul>";
    html += "<hr>";
    html += "<p><em>Généré par M5Stack Core2 - FRA222</em></p>";
    html += "</body></html>";
    
    webServer_->send(200, "text/html", html);
}

/**
 * @brief Handle HTTP requests to /list for directory browsing
 * 
 * Serves a file listing page for the specified directory path.
 * Features include:
 * - Directory navigation with breadcrumb links
 * - File and folder icons
 * - File size display
 * - Download links for files
 * - Parent directory navigation
 * 
 * Query parameter: ?dir=/path/to/directory
 * Defaults to root directory (/) if no path specified.
 */
void FileServerManager::handleFileList() {
    String path = webServer_->arg("dir");
    if (path == "") path = "/";
    
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Fichiers - " + path + "</title>";
    html += "<meta charset='UTF-8'>";
    html += "<style>body{font-family:Arial,sans-serif;margin:20px;}";
    html += "h1{color:#333;}table{border-collapse:collapse;width:100%;}";
    html += "th,td{border:1px solid #ddd;padding:8px;text-align:left;}";
    html += "th{background-color:#f2f2f2;}a{text-decoration:none;color:#0066cc;}";
    html += "a:hover{text-decoration:underline;}</style>";
    html += "</head><body>";
    html += "<h1>📁 Contenu de: " + path + "</h1>";
    html += "<p><a href='/'>🏠 Retour à l'accueil</a></p>";
    
    File dir = SD.open(path);
    if (!dir) {
        html += "<p style='color:red;'>❌ Erreur: Impossible d'ouvrir le répertoire</p>";
    } else if (!dir.isDirectory()) {
        html += "<p style='color:red;'>❌ Erreur: Ce n'est pas un répertoire</p>";
    } else {
        html += "<table>";
        html += "<tr><th>📄 Nom</th><th>📏 Taille</th><th>⬇️ Action</th></tr>";
        
        File file = dir.openNextFile();
        while (file) {
            String fileName = file.name();
            String fullPath = path;
            if (!fullPath.endsWith("/")) fullPath += "/";
            fullPath += fileName;
            
            html += "<tr>";
            html += "<td>";
            if (file.isDirectory()) {
                html += "📂 " + fileName;
            } else {
                html += "📄 " + fileName;
            }
            html += "</td>";
            html += "<td>";
            if (!file.isDirectory()) {
                html += String(file.size()) + " bytes";
            } else {
                html += "-";
            }
            html += "</td>";
            html += "<td>";
            if (file.isDirectory()) {
                html += "<a href='/list?dir=" + fullPath + "'>📂 Ouvrir</a>";
            } else {
                html += "<a href='/download?file=" + fullPath + "'>⬇️ Télécharger</a>";
            }
            html += "</td>";
            html += "</tr>";
            
            file = dir.openNextFile();
        }
        html += "</table>";
    }
    dir.close();
    
    html += "</body></html>";
    webServer_->send(200, "text/html", html);
}

/**
 * @brief Handle HTTP requests to /download for file downloads
 * 
 * Serves individual files from the SD card with proper MIME types.
 * Features include:
 * - MIME type detection based on file extension
 * - Streaming for large files
 * - Error handling for missing or invalid files
 * - Directory protection (prevents downloading directories)
 * 
 * Supported MIME types:
 * - .json → application/json
 * - .txt → text/plain  
 * - .csv → text/csv
 * - others → application/octet-stream
 * 
 * Query parameter: ?file=/path/to/file
 */
void FileServerManager::handleFileDownload() {
    String filename = webServer_->arg("file");
    if (filename == "") {
        webServer_->send(400, "text/plain", "Missing 'file' parameter");
        return;
    }
    
    File file = SD.open(filename);
    if (!file) {
        webServer_->send(404, "text/plain", "File not found");
        return;
    }
    
    if (file.isDirectory()) {
        webServer_->send(400, "text/plain", "Cannot download a directory");
        file.close();
        return;
    }
    
    String contentType = "application/octet-stream";
    if (filename.endsWith(".json")) {
        contentType = "application/json";
    } else if (filename.endsWith(".txt")) {
        contentType = "text/plain";
    } else if (filename.endsWith(".csv")) {
        contentType = "text/csv";
    }
    
    webServer_->streamFile(file, contentType);
    file.close();
    
    log("File downloaded: " + filename);
}

/**
 * @brief Handle HTTP 404 errors for invalid URLs
 * 
 * Serves a styled 404 error page when clients request non-existent URLs.
 * Features include:
 * - User-friendly error message
 * - Navigation link back to home page
 * - Consistent styling with the main interface
 * - UTF-8 character encoding support
 */
void FileServerManager::handleNotFound() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>404 - Page Not Found</title>";
    html += "<meta charset='UTF-8'>";
    html += "</head><body>";
    html += "<h1>❌ 404 - Page Not Found</h1>";
    html += "<p>The requested page does not exist.</p>";
    html += "<p><a href='/'>🏠 Return to Home</a></p>";
    html += "</body></html>";
    
    webServer_->send(404, "text/html", html);
}

/**
 * @brief Load WiFi configuration from SD card
 * @return true if configuration loads successfully, false otherwise
 * 
 * Reads WiFi credentials from /wifi_config.json on the SD card.
 * The JSON file must contain "ssid" and "password" fields.
 * 
 * Expected file format:
 * {
 *   "ssid": "NetworkName",
 *   "password": "NetworkPassword"
 * }
 * 
 * The loaded configuration is stored in the wifiConfig_ member
 * and marked as valid for subsequent connection attempts.
 */
bool FileServerManager::loadWiFiConfig() {
    log("Loading WiFi configuration...");
    
    if (!SD.exists("/wifi_config.json")) {
        log("Error: wifi_config.json file not found on SD card");
        return false;
    }
    
    File configFile = SD.open("/wifi_config.json", FILE_READ);
    if (!configFile) {
        log("Error: Unable to open wifi_config.json");
        return false;
    }
    
    String content = configFile.readString();
    configFile.close();
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, content);
    
    if (error) {
        log("Error: Invalid JSON in wifi_config.json");
        return false;
    }
    
    if (!doc["ssid"].is<String>() || !doc["password"].is<String>()) {
        log("Error: Missing ssid or password keys in wifi_config.json");
        return false;
    }
    
    wifiConfig_.ssid = doc["ssid"].as<String>();
    wifiConfig_.password = doc["password"].as<String>();
    wifiConfig_.isValid = true;
    
    log("WiFi configuration loaded: SSID=" + wifiConfig_.ssid);
    return true;
}

/**
 * @brief Connect to WiFi network using loaded configuration
 * @return true if connection succeeds, false otherwise
 * 
 * Establishes WiFi connection using credentials from wifiConfig_.
 * If configuration is not loaded, attempts to load it first.
 * 
 * Connection process:
 * 1. Load WiFi config if not already loaded
 * 2. Set WiFi mode to Station (WIFI_STA)
 * 3. Begin connection with stored credentials
 * 4. Wait up to 10 seconds for connection (20 × 500ms attempts)
 * 5. Verify and log connection status
 * 
 * Note: This switches from ESPNow to WiFi mode.
 */
bool FileServerManager::connectToWiFi() {
    if (!wifiConfig_.isValid) {
        if (!loadWiFiConfig()) {
            return false;
        }
    }
    
    log("Connecting to WiFi: " + wifiConfig_.ssid);
    
    // Stop ESPNow if active and switch to WiFi mode
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiConfig_.ssid.c_str(), wifiConfig_.password.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected_ = true;
        log("WiFi connected! IP: " + WiFi.localIP().toString());
        return true;
    } else {
        log("Error: Unable to connect to WiFi");
        wifiConnected_ = false;
        return false;
    }
}

/**
 * @brief Disconnect from WiFi and disable WiFi mode
 * 
 * Cleanly disconnects from the current WiFi network and disables
 * WiFi functionality to allow return to ESPNow mode.
 * 
 * Disconnection process:
 * 1. Check if currently connected to WiFi
 * 2. Call WiFi.disconnect() to end network connection
 * 3. Set WiFi mode to OFF to disable radio
 * 4. Update connection status flag
 * 5. Log disconnection status
 * 
 * After this call, the system can return to ESPNow operation.
 */
void FileServerManager::disconnectWiFi() {
    if (wifiConnected_) {
        log("Disconnecting WiFi...");
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        wifiConnected_ = false;
        log("WiFi disconnected");
    }
}