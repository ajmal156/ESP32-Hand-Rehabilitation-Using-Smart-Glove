let allPatients = [];
let allLogs = [];
let emgChart = null;
let maxPeakRecorded = 0;
const MAX_CHART_POINTS = 40;

document.addEventListener("DOMContentLoaded", () => {
  initNavigation();
  initEMGChart();
  loadAll();

  setInterval(loadAll, 5000);
  setInterval(pollLiveEMG, 500);

  document.getElementById("patientForm")?.addEventListener("submit", handleAddPatient);
  document.getElementById("patientFilterSelector")?.addEventListener("change", fetchExerciseLogs);
});

function initNavigation() {
  document.querySelectorAll(".nav-btn").forEach(btn => {
    btn.addEventListener("click", () => {
      document.querySelectorAll(".nav-btn").forEach(b => b.classList.remove("active"));
      document.querySelectorAll(".section").forEach(s => s.classList.remove("active"));
      btn.classList.add("active");
      document.getElementById(btn.dataset.section)?.classList.add("active");
      document.getElementById("pageTitle").innerText = btn.innerText.trim();
    });
  });
}

function initEMGChart() {
  const ctx = document.getElementById("emgChartCanvas");
  if (!ctx || typeof Chart === "undefined") return;

  emgChart = new Chart(ctx, {
    type: "line",
    data: {
      labels: Array(MAX_CHART_POINTS).fill(""),
      datasets: [{
        label: "EMG Signal (ADC)",
        data: Array(MAX_CHART_POINTS).fill(0),
        borderColor: "#007bff",
        backgroundColor: "rgba(0,123,255,.12)",
        borderWidth: 2,
        fill: true,
        tension: .25,
        pointRadius: 0
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: false,
      scales: {
        x: { display: false },
        y: {
          beginAtZero: true,
          min: 0,
          max: 4095,
          title: { display: true, text: "ADC Signal" }
        }
      }
    }
  });
}

async function loadAll() {
  await fetchPatients();
  await fetchExerciseLogs();
  updateDashboardCards();
}

async function fetchPatients() {
  try {
    const res = await fetch("/api/patients");
    if (!res.ok) throw new Error("Database error");
    allPatients = await res.json();
    updateAPIStatus(true, "DB Connected");
    renderPatientsTable();
    populatePatientSelectors();
  } catch (err) {
    updateAPIStatus(false, "DB Offline");
  }
}

async function fetchExerciseLogs() {
  const patient = document.getElementById("patientFilterSelector")?.value;
  if (!patient) {
    allLogs = [];
    renderRecentLogs();
    renderLogsTable();
    return;
  }

  try {
    const res = await fetch(`/api/exercise/logs?patient_id=${encodeURIComponent(patient)}`);
    if (!res.ok) throw new Error("Exercise logs unavailable");
    allLogs = await res.json();
    renderRecentLogs();
    renderLogsTable();
  } catch (err) {
    console.error(err);
  }
}

async function pollLiveEMG() {
  const patient = document.getElementById("sessionPatientSelector")?.value;
  if (!patient) return;

  try {
    const res = await fetch(`/api/emg/latest?patient_id=${encodeURIComponent(patient)}`);
    if (!res.ok) return;

    const data = await res.json();
    if (data && data.emg_adc !== undefined) updateEMGUI(data);
  } catch (_) {}
}

function updateEMGUI(data) {
  const adc = Number(data.emg_adc || 0);
  const pct = Number(data.emg_percent || 0);
  const peak = Number(data.emg_peak_adc || adc);

  if (peak > maxPeakRecorded) maxPeakRecorded = peak;

  document.getElementById("emgCh1Val").innerHTML = `${Math.round(adc)} <span>ADC</span>`;
  document.getElementById("emgPercentVal").innerText = `${Math.round(pct)}%`;
  document.getElementById("emgPeakVal").innerText = `${Math.round(maxPeakRecorded)} ADC`;

  const fatigue = document.getElementById("emgFatigueVal");
  fatigue.innerText = data.muscle_fatigue_index || "LOW";
  fatigue.style.color =
    fatigue.innerText === "HIGH" ? "#dc3545" :
    fatigue.innerText === "MEDIUM" ? "#f59e0b" : "#28a745";

  document.getElementById("emgCh1Bar").style.width = `${Math.min(100, pct)}%`;

  if (emgChart) {
    emgChart.data.datasets[0].data.push(adc);
    emgChart.data.datasets[0].data.shift();
    emgChart.update("none");
  }
}

function renderRecentLogs() {
  const tbody = document.getElementById("recentLogsTable");
  if (!tbody) return;

  const recent = [...allLogs].slice(-6).reverse();
  tbody.innerHTML = recent.length ? recent.map(log => `
    <tr>
      <td>${escapeHtml(log.datetime || "—")}</td>
      <td>${escapeHtml(log.patient_name || log.patient_id || "Unknown")}</td>
      <td>${escapeHtml(log.exercise || "—")}</td>
      <td>${escapeHtml(log.repetitions || "0")}</td>
      <td>${escapeHtml(log.duration_sec || "0")}s</td>
      <td>${escapeHtml(log.pressure_psi || "0")} PSI</td>
      <td>${escapeHtml(log.emg_peak || "0")} ADC</td>
      <td><span class="status-tag">${escapeHtml(log.status || "—")}</span></td>
    </tr>`).join("") :
    `<tr><td colspan="8" style="text-align:center">Select a patient to view logs.</td></tr>`;
}

function renderLogsTable() {
  const tbody = document.getElementById("dataLogsTable");
  if (!tbody) return;

  tbody.innerHTML = allLogs.length ? [...allLogs].reverse().map(log => `
    <tr>
      <td>${escapeHtml(log.datetime || "—")}</td>
      <td>${escapeHtml(log.patient_id || "—")}</td>
      <td>${escapeHtml(log.exercise || "—")}</td>
      <td>${escapeHtml(log.repetitions || "0")}</td>
      <td>${escapeHtml(log.duration_sec || "0")}s</td>
      <td>${escapeHtml(log.pressure_psi || "0")} PSI</td>
      <td>${escapeHtml(log.emg_peak || "0")} ADC</td>
      <td><span class="status-tag">${escapeHtml(log.status || "—")}</span></td>
    </tr>`).join("") :
    `<tr><td colspan="8" style="text-align:center">No exercise logs found.</td></tr>`;
}

function renderPatientsTable() {
  const tbody = document.getElementById("patientsRecordsTable");
  if (!tbody) return;

  tbody.innerHTML = allPatients.length ? allPatients.map(p => `
    <tr>
      <td><strong>${escapeHtml(p.patient_id)}</strong></td>
      <td>${escapeHtml(p.name)}</td>
      <td>${escapeHtml(p.age)}</td>
      <td>${escapeHtml(p.doctor || "Unassigned")}</td>
      <td>${escapeHtml(p.exercise_plan || "General Therapy")}</td>
      <td>${escapeHtml(p.registered || "—")}</td>
    </tr>`).join("") :
    `<tr><td colspan="6" style="text-align:center">No patients registered.</td></tr>`;
}

function populatePatientSelectors() {
  const options = allPatients.map(p =>
    `<option value="${escapeAttr(p.patient_id)}">${escapeHtml(p.name)} (${escapeHtml(p.patient_id)})</option>`
  ).join("");

  const session = document.getElementById("sessionPatientSelector");
  const filter = document.getElementById("patientFilterSelector");

  if (session) {
    const old = session.value;
    session.innerHTML = `<option value="">Select Active Patient...</option>${options}`;
    if (allPatients.some(p => p.patient_id === old)) session.value = old;
  }

  if (filter) {
    const old = filter.value;
    filter.innerHTML = `<option value="">Select Patient...</option>${options}`;
    if (allPatients.some(p => p.patient_id === old)) filter.value = old;
  }
}

function updateDashboardCards() {
  document.getElementById("totalPatientsCard").innerText = allPatients.length;
  document.getElementById("totalSessionsCard").innerText = allLogs.length;
  document.getElementById("completedSessionsCard").innerText =
    allLogs.filter(x => x.status === "COMPLETED").length;
  document.getElementById("latestExerciseCard").innerText =
    allLogs.length ? allLogs[allLogs.length - 1].exercise : "None";
}

function updateAPIStatus(connected, message) {
  document.getElementById("dot").style.backgroundColor =
    connected ? "#28a745" : "#dc3545";
  document.getElementById("apiStatus").innerText = message;
}

function openPatientForm() {
  document.getElementById("patientAddModal").style.display = "flex";
}
function closePatientForm() {
  document.getElementById("patientAddModal").style.display = "none";
}

async function handleAddPatient(e) {
  e.preventDefault();

  const payload = {
    patient_id: document.getElementById("p_id").value.trim(),
    name: document.getElementById("p_name").value.trim(),
    age: document.getElementById("p_age").value.trim(),
    doctor: document.getElementById("p_doctor").value.trim(),
    exercise_plan: document.getElementById("p_plan").value.trim()
  };

  try {
    const res = await fetch("/api/patients", {
      method: "POST",
      headers: {"Content-Type":"application/json"},
      body: JSON.stringify(payload)
    });

    const data = await res.json();

    if (!res.ok) {
      alert(data.error || "Failed to add patient.");
      return;
    }

    closePatientForm();
    document.getElementById("patientForm").reset();
    await loadAll();
  } catch (_) {
    alert("Server communication error.");
  }
}

async function checkGloveConnection() {
  const ip = document.getElementById("gloveIpInput").value.trim();
  const text = document.getElementById("gloveStatusText");
  const dot = document.getElementById("gloveStatusDot");

  if (!ip) {
    alert("Enter the ESP32 IP address first.");
    return;
  }

  text.innerText = "Connecting...";
  dot.style.backgroundColor = "#f59e0b";

  try {
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), 3000);

    const res = await fetch(`http://${ip}/status`, {
      method: "GET",
      signal: controller.signal
    });

    clearTimeout(timer);

    if (!res.ok) throw new Error("ESP32 not responding");

    const data = await res.json();

    if (data.status !== "ONLINE") throw new Error("Invalid status");

    text.innerText = `Connected (${ip})`;
    dot.style.backgroundColor = "#28a745";
    document.getElementById("activeSessionDisplay").innerText = `Active Link: ${ip}`;
  } catch (_) {
    text.innerText = "Disconnected";
    dot.style.backgroundColor = "#dc3545";
    alert("ESP32 connection failed. Check IP, Wi-Fi and firewall.");
  }
}

async function remoteControlExercise(exerciseName, duration, repetitions) {
  const ip = document.getElementById("gloveIpInput").value.trim();
  const patientId = document.getElementById("sessionPatientSelector").value;
  const msg = document.getElementById("gloveControlMessage");

  if (!ip) {
    alert("Connect the glove first.");
    return;
  }
  if (!patientId) {
    alert("Assign a patient first.");
    return;
  }

  msg.innerText = `Sending ${exerciseName}...`;

  try {
    const res = await fetch(`http://${ip}/start_therapy`, {
      method: "POST",
      headers: {"Content-Type":"application/json"},
      body: JSON.stringify({
        patient_id: patientId,
        exercise: exerciseName,
        duration_sec: duration,
        repetitions: repetitions,
        inflate_sec: Math.floor(duration / 2),
        deflate_sec: Math.ceil(duration / 2)
      })
    });

    if (!res.ok) throw new Error("ESP32 rejected command");

    msg.innerText = `${exerciseName} started.`;
  } catch (_) {
    msg.innerText = `Failed to reach glove at ${ip}.`;
    alert("Therapy command could not reach ESP32.");
  }
}

async function remoteStopSystem() {
  const ip = document.getElementById("gloveIpInput").value.trim();
  if (!ip) {
    alert("Enter ESP32 IP first.");
    return;
  }

  try {
    await fetch(`http://${ip}/stop`, {method:"GET"});
    document.getElementById("gloveControlMessage").innerText = "Emergency stop sent.";
  } catch (_) {
    alert("Could not reach ESP32 for emergency stop.");
  }
}

function escapeHtml(value) {
  return String(value ?? "").replace(/[&<>"']/g, c => ({
    "&":"&amp;","<":"&lt;",">":"&gt;","\"":"&quot;","'":"&#039;"
  }[c]));
}
function escapeAttr(value) {
  return escapeHtml(value);
}
