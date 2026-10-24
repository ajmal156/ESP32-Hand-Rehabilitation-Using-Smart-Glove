# Physio Glove Project - Corrected Version

## Components
- ESP32
- One analog EMG sensor connected to GPIO 34
- Two pneumatic valve control outputs
- 4x4 keypad
- 16x2 I2C LCD
- Flask dashboard

## Folder structure

Physio_Glove_Project_Corrected/
  server.py
  web/
    dashboard.html
    style.css
    script.js
  esp32/
    PhysioGlove.ino

Runtime-created:
  patients.csv
  patients_login.csv
  patients_exercise/
  emg_sensor_data/

## Before uploading ESP32 code

Open `esp32/PhysioGlove.ino` and change:

String pythonServer = "http://10.1.32.40:5000";

to the IPv4 address of the PC running Flask.

Also verify the Wi-Fi SSID/password.

## Start Flask

From the project directory:

python server.py

Open:

http://127.0.0.1:5000

## ESP32 test

After upload, open Serial Monitor at 115200 baud.

Find:

ESP32 IP: x.x.x.x

Then open:

http://ESP32_IP/status

Expected:

{"status":"ONLINE", ...}

## EMG

Current project uses the raw ESP32 ADC range 0-4095.

The practical signal percentage is calculated from:

EMG_MIN_ADC = 720
EMG_MAX_ADC = 2400

This percentage is a project signal-strength indicator and is not a clinically validated medical measurement.

## Important

The current hardware has ONE EMG analog input on GPIO 34. Therefore the dashboard shows one real EMG channel. It does not create a fake second EMG channel.
