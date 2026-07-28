
#include <WebServer.h>

void setupGlove();

void setupKeypad();

void setupWiFi();

void readKeypad();


extern WebServer server;

void setup()
{
    Serial.begin(115200);


    Serial.println();
    Serial.println("====================================");
    Serial.println(" Physiotherapy Glove System");
    Serial.println(" ESP32 Starting...");
    Serial.println("====================================");

    // Initialize Glove Hardware
    setupGlove();

    // Initialize Keypad
    setupKeypad();

    // Connect WiFi
    setupWiFi();


    Serial.println();
    Serial.println("System Ready");
    Serial.println("Waiting for Patient Login...");
}

// -------- Loop --------

void loop()
{

    // Handle ESP32 Web Server
    server.handleClient();   

    // Read Keypad Input
    readKeypad();

}