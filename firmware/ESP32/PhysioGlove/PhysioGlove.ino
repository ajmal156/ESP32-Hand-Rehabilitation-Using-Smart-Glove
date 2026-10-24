/**
 * @file PhysioGlove.ino
 * @brief Main entry point for the ESP32 Physiotherapy Glove System.
 * @details Initializes system peripherals, connects to Wi-Fi, handles web server requests, 
 *          scans the keypad, and streams live EMG telemetry.
 */

#include "Config.h"
#include "Hardware.h"
#include "EMG_Sensor.h"
#include "Network.h"

void setup() {
    // Initialize serial communication for debugging output
    Serial.begin(115200);
    delay(300);

    // Initialize I2C communication for LCD and expanders (SDA: 21, SCL: 22)
    Wire.begin(21, 22);

    // Initialize hardware components (Valves, LCD, Keypad)
    setupHardware();
    
    // Calibrate EMG baseline noise floor from relaxed muscles
    calibrateEmgBaseline();
    
    // Connect to Wi-Fi network and start web server
    setupWiFi();

    // Update LCD based on Wi-Fi connection status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("ESP32 IP: ");
        Serial.println(WiFi.localIP());
        updateLcd("System Ready", WiFi.localIP().toString());
    } else {
        updateLcd("WiFi Failed", "Keypad Active");
    }
}

void loop() {
    // Handle incoming HTTP client web requests
    server.handleClient();
    
    // Continuously poll the physical keypad for manual commands
    readKeypad();
    
    // Periodically upload real-time EMG readings to the Python/Flask backend server
    uploadLiveEMG();

    // Execute remote therapy commands triggered via the Web API
    if (pendingRemoteTherapy) {
        pendingRemoteTherapy = false;
        runCustomExercise(pendingExName, pendingInfSec, pendingDefSec, pendingRepetitions);
    }
}
