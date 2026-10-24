/**
 * @file EMG_Sensor.cpp
 * @brief Handles analog EMG signal reading, noise baseline calibration, filtering, and percentage mapping.
 */

#include "EMG_Sensor.h"

int emgBaseline = 0;
int emgFiltered = 0;

/**
 * @brief Measures the resting muscle voltage baseline during device startup.
 * @note The user must keep their muscle completely relaxed while this runs.
 */
void calibrateEmgBaseline() {
    long sum = 0;
    for (int i = 0; i < EMG_BASELINE_SAMPLES; i++) {
        sum += analogRead(EMG_SIG_PIN);
        delay(10);
    }
    emgBaseline = constrain(sum / EMG_BASELINE_SAMPLES, 0, 4095);
    emgFiltered = emgBaseline;

    Serial.print("EMG baseline: ");
    Serial.println(emgBaseline);
    updateLcd("EMG Calibrated", "Baseline: " + String(emgBaseline));
    delay(700);
}

/**
 * @brief Converts raw ADC muscle signals into a normalized 0% to 100% contraction scale.
 */
int calculateEmgPercent(int adc) {
    int activity = abs(adc - emgBaseline);
    
    // Apply deadband filter to eliminate minor background noise around resting levels
    if (activity <= EMG_DEADBAND) {
        activity = 0;
    } else {
        activity -= EMG_DEADBAND;
    }

    int usableRange = max(100, EMG_MAX_ADC - EMG_MIN_ADC);
    long pct = (long)activity * 100L / usableRange;
    return constrain((int)pct, 0, 100);
}

/**
 * @brief Takes oversampling measurements and applies smoothing filters to steady the EMG sensor data.
 */
int readSingleEmgSensor() {
    long sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += analogRead(EMG_SIG_PIN);
        delayMicroseconds(500);
    }
    int raw = constrain(sum / 20, 0, 4095);
    
    // Apply exponential/moving average smoothing filter to prevent sudden ADC spikes
    if (emgFiltered == 0) emgFiltered = raw;
    emgFiltered = (emgFiltered * 3 + raw) / 4; 
    return emgFiltered;
}