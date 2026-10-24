/**
 * @file Network.cpp
 * @brief Manages Wi-Fi connection, web server endpoint routes, and backend API requests.
 */

#include "Network.h"

const char* ssid = "Smart-Campus-STD";
const char* password = "pafiast##std2021";
String pythonServer = "http://10.1.32.40:5000";

WebServer server(80);

String loggedPatientId = "P101";
volatile bool stopFlag = false;
volatile bool pendingRemoteTherapy = false;
String pendingExName = "";
int pendingInfSec = 5;
int pendingDefSec = 5;
int pendingRepetitions = 1;

unsigned long lastEmgUpload = 0;
const unsigned long EMG_UPLOAD_INTERVAL = 300; // Upload telemetry every 300ms

// Internal helper declarations for lightweight JSON parsing and CORS handling
String getJsonString(String json, String key);
int getJsonInt(String json, String key, int defaultValue);
void sendCORSHeaders();
void handleCORS();
void handleRoot();
void handleStatus();
void handleEmgLive();
void handleStartTherapy();
void handleStopSystem();

/**
 * @brief Connects the ESP32 to the configured Wi-Fi network and registers HTTP server routes.
 */
void setupWiFi() {
    Serial.println("\nConnecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    Serial.println();
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection FAILED");
        return;
    }

    Serial.println("WiFi CONNECTED");
    
    // Register API Endpoints and HTTP methods
    server.on("/", HTTP_GET, handleRoot);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/status", HTTP_OPTIONS, handleCORS);
    server.on("/emg", HTTP_GET, handleEmgLive);
    server.on("/emg", HTTP_OPTIONS, handleCORS);
    server.on("/start_therapy", HTTP_POST, handleStartTherapy);
    server.on("/start_therapy", HTTP_OPTIONS, handleCORS);
    server.on("/stop", HTTP_GET, handleStopSystem);
    server.on("/stop", HTTP_POST, handleStopSystem);
    server.on("/stop", HTTP_OPTIONS, handleCORS);

    server.begin();
    Serial.println("ESP32 WebServer started");
}

/**
 * @brief Periodically uploads live EMG sensor telemetry data to the Python backend server.
 */
void uploadLiveEMG() {
    if (WiFi.status() != WL_CONNECTED) return;
    if (millis() - lastEmgUpload < EMG_UPLOAD_INTERVAL) return;
    lastEmgUpload = millis();

    int adc = readSingleEmgSensor();
    int percent = calculateEmgPercent(adc);

    HTTPClient http;
    http.setTimeout(1500);
    http.begin(pythonServer + "/api/emg");
    http.addHeader("Content-Type", "application/json");

    String json = "{";
    json += "\"patient_id\":\"" + loggedPatientId + "\",";
    json += "\"exercise\":\"LIVE_MONITORING\",";
    json += "\"emg_adc\":" + String(adc) + ",";
    json += "\"emg_percent\":" + String(percent) + ",";
    json += "\"status\":\"LIVE\"";
    json += "}";

    http.POST(json);
    http.end();
}

/**
 * @brief Sends completed exercise summaries, peak muscle telemetry, and execution logs to the backend database.
 */
void sendDataLog(String exercise, int durationSec, int emgPeak, int repetitions, String status) {
    if (WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    http.setTimeout(2500);
    http.begin(pythonServer + "/api/logs");
    http.addHeader("Content-Type", "application/json");

    String json = "{";
    json += "\"patient_id\":\"" + loggedPatientId + "\",";
    json += "\"exercise\":\"" + exercise + "\",";
    json += "\"status\":\"" + status + "\",";
    json += "\"duration_sec\":\"" + String(durationSec) + "\",";
    json += "\"repetitions\":\"" + String(repetitions) + "\",";
    json += "\"pressure_psi\":\"15\",";
    json += "\"emg_peak\":\"" + String(emgPeak) + "\"";
    json += "}";

    http.POST(json);
    http.end();
}

// --- Lightweight JSON Helper Functions ---
String getJsonString(String json, String key) {
    int keyIdx = json.indexOf("\"" + key + "\"");
    if (keyIdx == -1) return "";
    int colonIdx = json.indexOf(":", keyIdx);
    if (colonIdx == -1) return "";
    int startQuote = json.indexOf("\"", colonIdx);
    if (startQuote == -1) return "";
    int endQuote = json.indexOf("\"", startQuote + 1);
    if (endQuote == -1) return "";
    return json.substring(startQuote + 1, endQuote);
}

int getJsonInt(String json, String key, int defaultValue) {
    int keyIdx = json.indexOf("\"" + key + "\"");
    if (keyIdx == -1) return defaultValue;
    int colonIdx = json.indexOf(":", keyIdx);
    if (colonIdx == -1) return defaultValue;
    String sub = json.substring(colonIdx + 1);
    sub.trim();
    int val = sub.toInt();
    return val > 0 ? val : defaultValue;
}

// --- Web Server Endpoints & CORS Handlers ---
void sendCORSHeaders() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleCORS() {
    sendCORSHeaders();
    server.send(204);
}

void handleRoot() {
    sendCORSHeaders();
    server.send(200, "text/html", "<h2>ESP32 Physiotherapy Glove ONLINE</h2>");
}

void handleStatus() {
    sendCORSHeaders();
    String json = "{\"status\":\"ONLINE\",\"patient_id\":\"" + loggedPatientId + "\",\"ip\":\"" + WiFi.localIP().toString() + "\"}";
    server.send(200, "application/json", json);
}

void handleEmgLive() {
    sendCORSHeaders();
    int adc = readSingleEmgSensor();
    int percent = calculateEmgPercent(adc);
    String json = "{\"adc\":" + String(adc) + ",\"percent\":" + String(percent) + "}";
    server.send(200, "application/json", json);
}

void handleStartTherapy() {
    sendCORSHeaders();
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Body missing");
        return;
    }

    String body = server.arg("plain");
    String exName = getJsonString(body, "exercise");
    if (exName == "") exName = "Remote Exercise";

    String patId = getJsonString(body, "patient_id");
    if (patId != "") loggedPatientId = patId;

    pendingExName = exName;
    pendingInfSec = getJsonInt(body, "inflate_sec", 5);
    pendingDefSec = getJsonInt(body, "deflate_sec", 5);
    pendingRepetitions = getJsonInt(body, "repetitions", 1);
    pendingRemoteTherapy = true;

    server.send(200, "application/json", "{\"status\":\"STARTED\",\"exercise\":\"" + exName + "\"}");
}

void handleStopSystem() {
    sendCORSHeaders();
    stopSystem();
    server.send(200, "text/plain", "STOPPED");
}