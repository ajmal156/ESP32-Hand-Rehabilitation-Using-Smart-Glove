from flask import Flask, request, jsonify, send_from_directory
from pathlib import Path
import csv
from datetime import datetime

BASE = Path(__file__).resolve().parent
WEB = BASE.parent / "web"
PATIENTS = BASE / "patients.csv"
LOGS = BASE / "exercise_logs.csv"

app = Flask(__name__, static_folder=str(WEB), static_url_path="")

PATIENT_FIELDS = ["patient_id","name","age","doctor","exercise_plan","registered"]
LOG_FIELDS = ["datetime","patient_id","patient_name","exercise","repetitions",
              "duration_sec","pressure_psi","valve1","valve2","status"]

def ensure_files():
    if not PATIENTS.exists():
        with PATIENTS.open("w", newline="", encoding="utf-8") as f:
            csv.DictWriter(f, fieldnames=PATIENT_FIELDS).writeheader()
    if not LOGS.exists():
        with LOGS.open("w", newline="", encoding="utf-8") as f:
            csv.DictWriter(f, fieldnames=LOG_FIELDS).writeheader()

def read_csv(path):
    with path.open("r", newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))

@app.get("/")
def index():
    return send_from_directory(WEB, "dashboard.html")

@app.get("/<path:name>")
def static_files(name):
    return send_from_directory(WEB, name)

@app.get("/api/patients")
def get_patients():
    return jsonify(read_csv(PATIENTS))

@app.post("/api/patients")
def add_patient():
    data = request.get_json(silent=True) or {}
    required = ["patient_id","name","age"]
    if any(not str(data.get(k,"")).strip() for k in required):
        return jsonify(error="patient_id, name and age are required"), 400

    patients = read_csv(PATIENTS)
    if any(p["patient_id"] == str(data["patient_id"]).strip() for p in patients):
        return jsonify(error="Patient ID already exists"), 409

    row = {
        "patient_id": str(data["patient_id"]).strip(),
        "name": str(data["name"]).strip(),
        "age": str(data["age"]).strip(),
        "doctor": str(data.get("doctor","")).strip(),
        "exercise_plan": str(data.get("exercise_plan","")).strip(),
        "registered": datetime.now().strftime("%Y-%m-%d")
    }
    with PATIENTS.open("a", newline="", encoding="utf-8") as f:
        csv.DictWriter(f, fieldnames=PATIENT_FIELDS).writerow(row)
    return jsonify(row), 201

@app.get("/api/logs")
def get_logs():
    logs = read_csv(LOGS)
    logs.reverse()
    patients = {p["patient_id"]: p["name"] for p in read_csv(PATIENTS)}
    for row in logs:
        row["patient_name"] = patients.get(row["patient_id"], row.get("patient_name",""))
    return jsonify(logs)

@app.post("/api/logs")
def add_log():
    data = request.get_json(silent=True) or {}
    row = {field: str(data.get(field, "")).strip() for field in LOG_FIELDS}
    row["datetime"] = row["datetime"] or datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    if not row["patient_id"] or not row["exercise"]:
        return jsonify(error="patient_id and exercise are required"), 400

    with LOGS.open("a", newline="", encoding="utf-8") as f:
        csv.DictWriter(f, fieldnames=LOG_FIELDS).writerow(row)
    return jsonify(row), 201

@app.get("/api/health")
def health():
    return jsonify(status="ok")

if __name__ == "__main__":
    ensure_files()
    app.run(host="0.0.0.0", port=5000, debug=True)
