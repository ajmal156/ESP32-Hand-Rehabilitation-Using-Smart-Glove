from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
from pathlib import Path
from datetime import datetime
import csv
import threading

BASE = Path(__file__).resolve().parent
WEB = BASE / "web"

PATIENTS_FILE = BASE / "patients.csv"
PATIENTS_LOGIN_FILE = BASE / "patients_login.csv"
PATIENTS_EXERCISE_FOLDER = BASE / "patients_exercise"
EMG_SENSOR_DATA_FOLDER = BASE / "emg_sensor_data"

app = Flask(__name__, static_folder=str(WEB), static_url_path="")
CORS(app)

PATIENT_FIELDS = [
    "patient_id", "name", "age", "doctor", "exercise_plan", "registered"
]
LOGIN_FIELDS = [
    "datetime", "patient_id", "patient_name", "action", "status"
]
EXERCISE_FIELDS = [
    "datetime", "patient_id", "patient_name", "exercise",
    "repetitions", "duration_sec", "pressure_psi", "emg_peak",
    "status"
]
EMG_FIELDS = [
    "datetime", "patient_id", "patient_name", "exercise",
    "emg_adc", "emg_percent", "emg_peak_adc",
    "muscle_fatigue_index", "status"
]

LATEST_EMG_TELEMETRY = {}
_LAST_EMG_FILE_WRITE = {}
CSV_LOCK = threading.Lock()

EMG_MIN_ADC = 720
EMG_MAX_ADC = 2400


def now_string(ms=False):
    if ms:
        return datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")


def read_csv(path):
    if not path.exists() or path.stat().st_size == 0:
        return []
    try:
        with path.open("r", newline="", encoding="utf-8") as f:
            return [row for row in csv.DictReader(f) if row and any(row.values())]
    except (OSError, csv.Error):
        return []


def append_to_csv(file_path, fieldnames, row):
    file_path.parent.mkdir(parents=True, exist_ok=True)
    with CSV_LOCK:
        exists = file_path.exists() and file_path.stat().st_size > 0
        with file_path.open("a", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=fieldnames, extrasaction="ignore")
            if not exists:
                writer.writeheader()
            writer.writerow(row)


def ensure_csv(path, fields):
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.stat().st_size == 0:
        with path.open("w", newline="", encoding="utf-8") as f:
            csv.DictWriter(f, fieldnames=fields).writeheader()


def ensure_environment():
    PATIENTS_EXERCISE_FOLDER.mkdir(parents=True, exist_ok=True)
    EMG_SENSOR_DATA_FOLDER.mkdir(parents=True, exist_ok=True)
    ensure_csv(PATIENTS_FILE, PATIENT_FIELDS)
    ensure_csv(PATIENTS_LOGIN_FILE, LOGIN_FIELDS)


def patient_name_for(patient_id):
    for p in read_csv(PATIENTS_FILE):
        if p.get("patient_id") == patient_id:
            return p.get("name", "Unknown Patient")
    return "Unknown Patient"


def emg_percent(adc):
    try:
        adc = float(adc)
    except (TypeError, ValueError):
        return 0
    pct = (adc - EMG_MIN_ADC) * 100.0 / (EMG_MAX_ADC - EMG_MIN_ADC)
    return round(max(0.0, min(100.0, pct)), 1)


def fatigue_from_percent(percent):
    if percent >= 80:
        return "HIGH"
    if percent >= 55:
        return "MEDIUM"
    return "LOW"


@app.get("/")
def index():
    if (WEB / "dashboard.html").exists():
        return send_from_directory(WEB, "dashboard.html")
    return "<h2>web/dashboard.html not found.</h2>", 404


@app.get("/<path:name>")
def static_files(name):
    return send_from_directory(WEB, name)


@app.get("/api/status")
@app.get("/api/health")
def health():
    return jsonify(
        db_status="ONLINE",
        server="CONNECTED",
        timestamp=now_string()
    )


@app.get("/api/patients")
def get_patients():
    return jsonify(read_csv(PATIENTS_FILE))


@app.post("/api/patients")
def add_patient():
    data = request.get_json(silent=True) or {}
    required = ["patient_id", "name", "age"]

    if any(not str(data.get(k, "")).strip() for k in required):
        return jsonify(error="patient_id, name, and age are required."), 400

    patient_id = str(data["patient_id"]).strip()
    patients = read_csv(PATIENTS_FILE)

    if any(p.get("patient_id") == patient_id for p in patients):
        return jsonify(error="Patient ID already exists."), 409

    row = {
        "patient_id": patient_id,
        "name": str(data["name"]).strip(),
        "age": str(data["age"]).strip(),
        "doctor": str(data.get("doctor", "")).strip(),
        "exercise_plan": str(data.get("exercise_plan", "")).strip(),
        "registered": datetime.now().strftime("%Y-%m-%d")
    }

    append_to_csv(PATIENTS_FILE, PATIENT_FIELDS, row)
    return jsonify(row), 201


@app.post("/api/login")
def log_patient_login():
    data = request.get_json(silent=True) or {}
    patient_id = str(data.get("patient_id", "")).strip()

    if not patient_id:
        return jsonify(error="patient_id is required."), 400

    row = {
        "datetime": now_string(),
        "patient_id": patient_id,
        "patient_name": patient_name_for(patient_id),
        "action": str(data.get("action", "LOGIN")).strip(),
        "status": str(data.get("status", "SUCCESS")).strip()
    }

    append_to_csv(PATIENTS_LOGIN_FILE, LOGIN_FIELDS, row)
    return jsonify(status="logged", record=row)


@app.post("/api/exercise")
def record_exercise():
    # Kept for compatibility with existing clients.
    data = request.get_json(silent=True) or {}
    patient_id = str(data.get("patient_id", "")).strip()
    if not patient_id:
        return jsonify(error="patient_id is required."), 400

    row = {
        "datetime": now_string(),
        "patient_id": patient_id,
        "patient_name": patient_name_for(patient_id),
        "exercise": str(data.get("exercise", "General Exercise")).strip(),
        "repetitions": str(data.get("repetitions", "1")).strip(),
        "duration_sec": str(data.get("duration_sec", "0")).strip(),
        "pressure_psi": str(data.get("pressure_psi", "0")).strip(),
        "emg_peak": str(data.get("emg_peak", "0")).strip(),
        "status": str(data.get("status", "COMPLETED")).strip()
    }

    append_to_csv(
        PATIENTS_EXERCISE_FOLDER / f"{patient_id}.csv",
        EXERCISE_FIELDS,
        row
    )
    return jsonify(status="recorded", record=row)


@app.post("/api/emg")
def record_emg_stream():
    data = request.get_json(silent=True) or {}
    patient_id = str(data.get("patient_id", "")).strip()

    if not patient_id:
        return jsonify(error="patient_id is required for EMG recording."), 400

    try:
        adc = float(data.get("emg_adc", data.get("emg_ch1", 0)))
    except (TypeError, ValueError):
        adc = 0.0

    percent = emg_percent(adc)
    fatigue = fatigue_from_percent(percent)
    patient_name = patient_name_for(patient_id)

    row = {
        "datetime": now_string(ms=True),
        "patient_id": patient_id,
        "patient_name": patient_name,
        "exercise": str(data.get("exercise", "LIVE_MONITORING")).strip(),
        "emg_adc": str(round(adc, 1)),
        "emg_percent": str(percent),
        "emg_peak_adc": str(round(adc, 1)),
        "muscle_fatigue_index": fatigue,
        "status": str(data.get("status", "LIVE")).strip()
    }

    LATEST_EMG_TELEMETRY[patient_id] = row

    # Do not write 3+ CSV rows per second forever.
    # Save approximately one live EMG sample per second.
    current_ms = datetime.now().timestamp() * 1000
    previous_ms = _LAST_EMG_FILE_WRITE.get(patient_id, 0)

    if current_ms - previous_ms >= 1000:
        append_to_csv(
            EMG_SENSOR_DATA_FOLDER / f"{patient_id}.csv",
            EMG_FIELDS,
            row
        )
        _LAST_EMG_FILE_WRITE[patient_id] = current_ms

    return jsonify(status="recorded", data=row)


@app.post("/api/logs")
def receive_esp32_logs():
    data = request.get_json(silent=True) or {}
    patient_id = str(data.get("patient_id", "P101")).strip()
    patient_name = patient_name_for(patient_id)

    exercise = str(data.get("exercise", "General Exercise")).strip()
    repetitions = str(data.get("repetitions", "1")).strip()
    duration = str(data.get("duration_sec", "0")).strip()
    pressure = str(data.get("pressure_psi", "15")).strip()
    status = str(data.get("status", "COMPLETED")).strip()

    try:
        emg = float(data.get("emg_peak", data.get("emg_ch1", 0)))
    except (TypeError, ValueError):
        emg = 0

    exercise_row = {
        "datetime": now_string(),
        "patient_id": patient_id,
        "patient_name": patient_name,
        "exercise": exercise,
        "repetitions": repetitions,
        "duration_sec": duration,
        "pressure_psi": pressure,
        "emg_peak": str(round(emg, 1)),
        "status": status
    }

    append_to_csv(
        PATIENTS_EXERCISE_FOLDER / f"{patient_id}.csv",
        EXERCISE_FIELDS,
        exercise_row
    )

    percent = emg_percent(emg)

    emg_row = {
        "datetime": now_string(ms=True),
        "patient_id": patient_id,
        "patient_name": patient_name,
        "exercise": exercise,
        "emg_adc": str(round(emg, 1)),
        "emg_percent": str(percent),
        "emg_peak_adc": str(round(emg, 1)),
        "muscle_fatigue_index": fatigue_from_percent(percent),
        "status": status
    }

    append_to_csv(
        EMG_SENSOR_DATA_FOLDER / f"{patient_id}.csv",
        EMG_FIELDS,
        emg_row
    )

    LATEST_EMG_TELEMETRY[patient_id] = emg_row

    return jsonify(status="recorded", record=exercise_row)


@app.get("/api/emg/latest")
def get_latest_emg():
    patient_id = request.args.get("patient_id", "").strip()
    if patient_id:
        return jsonify(LATEST_EMG_TELEMETRY.get(patient_id, {}))
    return jsonify(LATEST_EMG_TELEMETRY)


@app.get("/api/exercise/logs")
def get_patient_exercise_logs():
    patient_id = request.args.get("patient_id", "").strip()
    if not patient_id:
        return jsonify(error="patient_id parameter missing."), 400

    return jsonify(
        read_csv(PATIENTS_EXERCISE_FOLDER / f"{patient_id}.csv")
    )


@app.get("/api/emg/logs")
def get_patient_emg_logs():
    patient_id = request.args.get("patient_id", "").strip()
    if not patient_id:
        return jsonify(error="patient_id parameter missing."), 400

    return jsonify(
        read_csv(EMG_SENSOR_DATA_FOLDER / f"{patient_id}.csv")
    )


if __name__ == "__main__":
    ensure_environment()
    app.run(host="0.0.0.0", port=5000, debug=True)

