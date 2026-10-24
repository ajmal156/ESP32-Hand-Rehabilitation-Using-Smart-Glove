/**
 * @file Hardware.cpp
 * @brief Manages physical user inputs (Keypad), visual outputs (LCD), and pneumatic glove actuators.
 */

#include "Hardware.h"

// Initialize I2C LCD Display (Address: 0x27, Columns: 16, Rows: 2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Configure 4x4 Matrix Keypad Layout and GPIO Pin Assignments
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 32, 33};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/**
 * @brief Configures GPIO pin directions, ADC resolution, and initializes the LCD display.
 */
void setupHardware() {
    pinMode(VALVE1_PIN, OUTPUT);
    pinMode(VALVE2_PIN, OUTPUT);
    pinMode(EMG_SIG_PIN, INPUT);

    analogReadResolution(12); // Set ESP32 ADC resolution to 12-bit (0 - 4095)
    setValves(false, false);  // Ensure valves start in a closed/safe state

    lcd.init();
    lcd.backlight();
    lcd.clear();
    updateLcd("Physio Glove", "Initializing...");
}

/**
 * @brief Safely updates both lines of the 16x2 LCD display, trimming long strings.
 */
void updateLcd(String line1, String line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1.substring(0, 16));
    lcd.setCursor(0, 1);
    lcd.print(line2.substring(0, 16));
}

/**
 * @brief Controls the pneumatic solenoid valves to inflate or deflate the glove chambers.
 */
void setValves(bool inflate, bool deflate) {
    digitalWrite(VALVE1_PIN, inflate ? HIGH : LOW);
    digitalWrite(VALVE2_PIN, deflate ? HIGH : LOW);
}

/**
 * @brief Implements a blocking delay that remains responsive to emergency stops and web client requests.
 */
bool safeDelay(unsigned long ms) {
    unsigned long start = millis();
    while (millis() - start < ms) {
        server.handleClient(); // Keeps the web server responsive during execution blocks
        char key = keypad.getKey();
        
        // If emergency stop key '0' or stop flag is triggered, abort execution immediately
        if (key == '0' || stopFlag) {
            stopSystem();
            return false;
        }
        delay(10);
    }
    return true;
}

/**
 * @brief Scans the physical keypad to handle manual local therapy routine selection.
 */
void readKeypad() {
    char key = keypad.getKey();
    if (!key) return;

    switch (key) {
        case '1': runCustomExercise("Full Hand", 9, 9, 1); break;
        case '2': runCustomExercise("Thumb Ex", 5, 5, 1); break;
        case '3': runCustomExercise("Index Finger", 4, 4, 1); break;
        case '4': runCustomExercise("Middle Finger", 4, 4, 1); break;
        case '5': runCustomExercise("Ring & Little", 4, 4, 1); break;
        case '0': stopSystem(); break;
        default: break;
    }
}

/**
 * @brief Executes a complete physiotherapy exercise cycle containing inflation, holding, and deflation phases.
 */
void runCustomExercise(String name, int inflateS, int deflateS, int repetitions) {
    stopFlag = false;
    int emgStart = readSingleEmgSensor();
    sendDataLog(name, 0, emgStart, repetitions, "STARTED");

    for (int rep = 1; rep <= repetitions; rep++) {
        // Inflation Phase
        updateLcd(name, "Inflate R" + String(rep));
        setValves(true, false);
        if (!safeDelay((unsigned long)inflateS * 1000UL)) return;

        // Pause between cycles
        setValves(false, false);
        if (!safeDelay(500)) return;

        // Deflation Phase
        updateLcd(name, "Deflate R" + String(rep));
        setValves(false, true);
        if (!safeDelay((unsigned long)deflateS * 1000UL)) return;

        setValves(false, false);
        if (rep < repetitions && !safeDelay(500)) return;
    }

    int emgEnd = readSingleEmgSensor();
    int peak = max(emgStart, emgEnd);
    updateLcd("Complete", name);
    sendDataLog(name, (inflateS + deflateS) * repetitions, peak, repetitions, "COMPLETED");
}

/**
 * @brief Instantly halts all pneumatic operations and logs an emergency stop event.
 */
void stopSystem() {
    stopFlag = true;
    pendingRemoteTherapy = false;
    setValves(false, false); // Turn off all valves
    int emg = readSingleEmgSensor();
    updateLcd("EMERGENCY STOP", "Glove OFF");
    sendDataLog("EMERGENCY_STOP", 0, emg, 0, "STOPPED");
}