/**
 * @file Config.h
 * @brief Global configuration, pin definitions, and shared system variables.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// --- HARDWARE PIN DEFINITIONS ---
#define VALVE1_PIN 18         // Control pin for Inflation Valve
#define VALVE2_PIN 19         // Control pin for Deflation Valve
#define EMG_SIG_PIN 34        // Analog input pin for the EMG Muscle Sensor

// EMG CALIBRATION & FILTERING CONFIGURATION 
#define EMG_MIN_ADC 720       // Minimum ADC threshold representing resting/low activity
#define EMG_MAX_ADC 2400      // Maximum ADC threshold representing full muscle contraction
#define EMG_BASELINE_SAMPLES 200 // Number of samples averaged during startup calibration
#define EMG_DEADBAND 35       // Noise filter deadband range around baseline

// --- NETWORK CREDENTIALS ---
extern const char* ssid;
extern const char* password;
extern String pythonServer;

// --- GLOBAL SYSTEM SYNCHRONIZATION FLAGS ---
extern String loggedPatientId;
extern volatile bool stopFlag;
extern volatile bool pendingRemoteTherapy;
extern String pendingExName;
extern int pendingInfSec;
extern int pendingDefSec;
extern int pendingRepetitions;

// Core Function Declarations shared across modules
void runCustomExercise(String name, int inflateS, int deflateS, int repetitions);
void stopSystem();
void updateLcd(String line1, String line2);
int readSingleEmgSensor();
int calculateEmgPercent(int adc);
void sendDataLog(String exercise, int durationSec, int emgPeak, int repetitions, String status);

#endif