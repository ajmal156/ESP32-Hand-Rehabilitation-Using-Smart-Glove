#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

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

#define VALVE1_PIN 18
#define VALVE2_PIN 19
#define EMG_SIG_PIN 34

// Calibrate these two values for your EMG sensor.
// They are the user's practical signal range, not medical units.
#define EMG_MIN_ADC 720
#define EMG_MAX_ADC 2400
#define EMG_BASELINE_SAMPLES 200
#define EMG_DEADBAND 35

int emgBaseline = 0;
int emgFiltered = 0;

const char* ssid = "Smart-Campus-STD";
const char* password = "pafiast##std2021";

// CHANGE THIS to the PC running Flask.
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
const unsigned long EMG_UPLOAD_INTERVAL = 300;

void setupGlove();
void calibrateEmgBaseline();
void setupLCD();
void setupKeypad();
void setupWiFi();
void readKeypad();
void uploadLiveEMG();
void sendDataLog(String exercise, int durationSec, int emgPeak, int repetitions, String status);
void updateLcd(String line1, String line2);
void setValves(bool inflate, bool deflate);
void runCustomExercise(String name, int inflateS, int deflateS, int repetitions);
void stopSystem();
bool safeDelay(unsigned long ms);
int readSingleEmgSensor();
int calculateEmgPercent(int adc);
String getJsonString(String json, String key);
int getJsonInt(String json, String key, int defaultValue);
void sendCORSHeaders();
void handleCORS();
void handleRoot();
void handleStatus();
void handleEmgLive();
void handleStartTherapy();
void handleStopSystem();

void setup() {
  Serial.begin(115200);
  delay(300);

  Wire.begin(21, 22);

  setupLCD();
  setupGlove();
  calibrateEmgBaseline();
  setupKeypad();
  setupWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());

    updateLcd("System Ready", WiFi.localIP().toString());
  } else {
    updateLcd("WiFi Failed", "Keypad Active");
  }
}

void loop() {
  server.handleClient();
  readKeypad();
  uploadLiveEMG();

  if (pendingRemoteTherapy) {
    pendingRemoteTherapy = false;

    runCustomExercise(
      pendingExName,
      pendingInfSec,
      pendingDefSec,
      pendingRepetitions
    );
  }
}

void setupGlove() {
  pinMode(VALVE1_PIN, OUTPUT);
  pinMode(VALVE2_PIN, OUTPUT);
  pinMode(EMG_SIG_PIN, INPUT);

  analogReadResolution(12);
  setValves(false, false);
}

void calibrateEmgBaseline() {
  // Keep the muscle completely relaxed during startup calibration.
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

int calculateEmgPercent(int adc) {
  // Remove the sensor's resting DC level first.
  int activity = abs(adc - emgBaseline);

  // Ignore small noise around the resting level.
  if (activity <= EMG_DEADBAND) activity = 0;
  else activity -= EMG_DEADBAND;

  // Use the remaining practical signal range for 0-100%.
  int usableRange = max(100, EMG_MAX_ADC - EMG_MIN_ADC);
  long pct = (long)activity * 100L / usableRange;
  return constrain((int)pct, 0, 100);
}

int readSingleEmgSensor() {
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogRead(EMG_SIG_PIN);
    delayMicroseconds(500);
  }
  int raw = constrain(sum / 20, 0, 4095);
  // Light smoothing prevents one noisy ADC sample from jumping to MAX.
  if (emgFiltered == 0) emgFiltered = raw;
  emgFiltered = (emgFiltered * 3 + raw) / 4;
  return emgFiltered;
}

void setValves(bool inflate, bool deflate) {
  digitalWrite(VALVE1_PIN, inflate ? HIGH : LOW);
  digitalWrite(VALVE2_PIN, deflate ? HIGH : LOW);
}

bool safeDelay(unsigned long ms) {
  unsigned long start = millis();

  while (millis() - start < ms) {
    server.handleClient();

    char key = keypad.getKey();

    if (key == '0' || stopFlag) {
      stopSystem();
      return false;
    }

    delay(10);
  }

  return true;
}

void runCustomExercise(
  String name,
  int inflateS,
  int deflateS,
  int repetitions
) {
  stopFlag = false;

  int emgStart = readSingleEmgSensor();

  sendDataLog(
    name,
    0,
    emgStart,
    repetitions,
    "STARTED"
  );

  for (int rep = 1; rep <= repetitions; rep++) {

    updateLcd(
      name,
      "Inflate R" + String(rep)
    );

    setValves(true, false);

    if (!safeDelay((unsigned long)inflateS * 1000UL)) {
      return;
    }

    setValves(false, false);

    if (!safeDelay(500)) {
      return;
    }

    updateLcd(
      name,
      "Deflate R" + String(rep)
    );

    setValves(false, true);

    if (!safeDelay((unsigned long)deflateS * 1000UL)) {
      return;
    }

    setValves(false, false);

    if (rep < repetitions) {
      if (!safeDelay(500)) {
        return;
      }
    }
  }

  int emgEnd = readSingleEmgSensor();
  int peak = max(emgStart, emgEnd);

  updateLcd("Complete", name);

  sendDataLog(
    name,
    (inflateS + deflateS) * repetitions,
    peak,
    repetitions,
    "COMPLETED"
  );
}

void stopSystem() {
  stopFlag = true;
  pendingRemoteTherapy = false;

  setValves(false, false);

  int emg = readSingleEmgSensor();

  updateLcd("EMERGENCY STOP", "Glove OFF");

  sendDataLog(
    "EMERGENCY_STOP",
    0,
    emg,
    0,
    "STOPPED"
  );
}

void setupLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  updateLcd(
    "Physio Glove",
    "Initializing..."
  );
}

void updateLcd(String line1, String line2) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(line1.substring(0, 16));

  lcd.setCursor(0, 1);
  lcd.print(line2.substring(0, 16));
}

void setupKeypad() {
  Serial.println("Keypad Active");
}

void readKeypad() {
  char key = keypad.getKey();

  if (!key) {
    return;
  }

  switch (key) {

    case '1':
      runCustomExercise("Full Hand", 9, 9, 1);
      break;

    case '2':
      runCustomExercise("Thumb Ex", 5, 5, 1);
      break;

    case '3':
      runCustomExercise("Index Finger", 4, 4, 1);
      break;

    case '4':
      runCustomExercise("Middle Finger", 4, 4, 1);
      break;

    case '5':
      runCustomExercise("Ring & Little", 4, 4, 1);
      break;

    case '0':
      stopSystem();
      break;

    default:
      break;
  }
}

String getJsonString(String json, String key) {
  int keyIdx = json.indexOf("\"" + key + "\"");

  if (keyIdx == -1) return "";

  int colonIdx = json.indexOf(":", keyIdx);

  if (colonIdx == -1) return "";

  int startQuote = json.indexOf("\"", colonIdx);

  if (startQuote == -1) return "";

  int endQuote = json.indexOf("\"", startQuote + 1);

  if (endQuote == -1) return "";

  return json.substring(
    startQuote + 1,
    endQuote
  );
}

int getJsonInt(
  String json,
  String key,
  int defaultValue
) {
  int keyIdx = json.indexOf("\"" + key + "\"");

  if (keyIdx == -1) return defaultValue;

  int colonIdx = json.indexOf(":", keyIdx);

  if (colonIdx == -1) return defaultValue;

  String sub = json.substring(colonIdx + 1);

  sub.trim();

  int val = sub.toInt();

  return val > 0 ? val : defaultValue;
}

void sendCORSHeaders() {
  server.sendHeader(
    "Access-Control-Allow-Origin",
    "*"
  );

  server.sendHeader(
    "Access-Control-Allow-Methods",
    "GET, POST, OPTIONS"
  );

  server.sendHeader(
    "Access-Control-Allow-Headers",
    "Content-Type"
  );
}

void handleCORS() {
  sendCORSHeaders();
  server.send(204);
}

void handleRoot() {
  sendCORSHeaders();

  server.send(
    200,
    "text/html",
    "<h2>ESP32 Physiotherapy Glove ONLINE</h2>"
  );
}

void handleStatus() {
  sendCORSHeaders();

  String json = "{";
  json += "\"status\":\"ONLINE\",";
  json += "\"patient_id\":\"" + loggedPatientId + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += "}";

  server.send(
    200,
    "application/json",
    json
  );
}

void handleEmgLive() {
  sendCORSHeaders();

  int adc = readSingleEmgSensor();
  int percent = calculateEmgPercent(adc);

  String json = "{";
  json += "\"adc\":" + String(adc) + ",";
  json += "\"percent\":" + String(percent);
  json += "}";

  server.send(
    200,
    "application/json",
    json
  );
}

void handleStartTherapy() {
  sendCORSHeaders();

  if (!server.hasArg("plain")) {
    server.send(
      400,
      "text/plain",
      "Body missing"
    );
    return;
  }

  String body = server.arg("plain");

  String exName =
    getJsonString(body, "exercise");

  if (exName == "") {
    exName = "Remote Exercise";
  }

  String patId =
    getJsonString(body, "patient_id");

  if (patId != "") {
    loggedPatientId = patId;
  }

  pendingExName = exName;

  pendingInfSec =
    getJsonInt(body, "inflate_sec", 5);

  pendingDefSec =
    getJsonInt(body, "deflate_sec", 5);

  pendingRepetitions =
    getJsonInt(body, "repetitions", 1);

  pendingRemoteTherapy = true;

  String json = "{";
  json += "\"status\":\"STARTED\",";
  json += "\"exercise\":\"" + exName + "\"";
  json += "}";

  server.send(
    200,
    "application/json",
    json
  );
}

void handleStopSystem() {
  sendCORSHeaders();

  stopSystem();

  server.send(
    200,
    "text/plain",
    "STOPPED"
  );
}

void setupWiFi() {
  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;

  while (
    WiFi.status() != WL_CONNECTED &&
    attempts < 30
  ) {
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
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );

  server.on(
    "/status",
    HTTP_GET,
    handleStatus
  );

  server.on(
    "/status",
    HTTP_OPTIONS,
    handleCORS
  );

  server.on(
    "/emg",
    HTTP_GET,
    handleEmgLive
  );

  server.on(
    "/emg",
    HTTP_OPTIONS,
    handleCORS
  );

  server.on(
    "/start_therapy",
    HTTP_POST,
    handleStartTherapy
  );

  server.on(
    "/start_therapy",
    HTTP_OPTIONS,
    handleCORS
  );

  server.on(
    "/stop",
    HTTP_GET,
    handleStopSystem
  );

  server.on(
    "/stop",
    HTTP_POST,
    handleStopSystem
  );

  server.on(
    "/stop",
    HTTP_OPTIONS,
    handleCORS
  );

  server.begin();

  Serial.println("ESP32 WebServer started");
}

void uploadLiveEMG() {

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (
    millis() - lastEmgUpload <
    EMG_UPLOAD_INTERVAL
  ) {
    return;
  }

  lastEmgUpload = millis();

  int adc = readSingleEmgSensor();
  int percent = calculateEmgPercent(adc);

  HTTPClient http;

  http.setTimeout(1500);

  http.begin(
    pythonServer + "/api/emg"
  );

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  String json = "{";
  json += "\"patient_id\":\"" + loggedPatientId + "\",";
  json += "\"exercise\":\"LIVE_MONITORING\",";
  json += "\"emg_adc\":" + String(adc) + ",";
  json += "\"emg_percent\":" + String(percent) + ",";
  json += "\"status\":\"LIVE\"";
  json += "}";

  int code = http.POST(json);

  if (code < 0) {
    Serial.print("EMG upload failed: ");
    Serial.println(code);
  }

  http.end();
}

void sendDataLog(
  String exercise,
  int durationSec,
  int emgPeak,
  int repetitions,
  String status
) {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  HTTPClient http;

  http.setTimeout(2500);

  http.begin(
    pythonServer + "/api/logs"
  );

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  String json = "{";

  json += "\"patient_id\":\"" +
          loggedPatientId + "\",";

  json += "\"exercise\":\"" +
          exercise + "\",";

  json += "\"status\":\"" +
          status + "\",";

  json += "\"duration_sec\":\"" +
          String(durationSec) + "\",";

  json += "\"repetitions\":\"" +
          String(repetitions) + "\",";

  json += "\"pressure_psi\":\"15\",";

  json += "\"emg_peak\":\"" +
          String(emgPeak) + "\"";

  json += "}";

  int code = http.POST(json);

  Serial.print("Exercise log response: ");
  Serial.println(code);

  http.end();
}
