const API = "/api";
let patients = [], logs = [];

document.querySelectorAll(".nav-btn").forEach(btn => {
  btn.addEventListener("click", () => {
    document.querySelectorAll(".nav-btn").forEach(b => b.classList.remove("active"));
    document.querySelectorAll(".section").forEach(s => s.classList.remove("active"));
    btn.classList.add("active");
    document.getElementById(btn.dataset.section).classList.add("active");
    document.getElementById("pageTitle").textContent =
      btn.dataset.section === "dashboard" ? "Dashboard" :
      btn.dataset.section === "patients" ? "Patients" : "Data Logging";
  });
});

async function getJSON(url, options={}) {
  const r = await fetch(url, options);
  if (!r.ok) throw new Error(await r.text());
  return r.json();
}

async function loadAll() {
  try {
    [patients, logs] = await Promise.all([getJSON(`${API}/patients`), getJSON(`${API}/logs`)]);
    document.getElementById("apiStatus").textContent = "Server connected";
    document.querySelector(".connection").classList.add("online");
    renderPatients(); renderLogs(); renderDashboard(); fillFilter();
  } catch(e) {
    document.getElementById("apiStatus").textContent = "Server unavailable";
    document.querySelector(".connection").classList.remove("online");
    console.error(e);
  }
}

function renderDashboard() {
  document.getElementById("totalPatients").textContent = patients.length;
  document.getElementById("totalSessions").textContent = logs.length;
  document.getElementById("completedSessions").textContent = logs.filter(x => x.status === "Completed").length;
  document.getElementById("latestExercise").textContent = logs[0]?.exercise || "—";
  document.getElementById("recentLogs").innerHTML = logs.slice(0,10).map(x => `
    <tr><td>${esc(x.datetime)}</td><td>${esc(x.patient_name || x.patient_id)}</td>
    <td>${esc(x.exercise)}</td><td>${esc(x.repetitions)}</td><td>${esc(x.duration_sec)} sec</td>
    <td>${esc(x.pressure_psi)} PSI</td><td class="status ${x.status.toLowerCase()}">${esc(x.status)}</td></tr>`).join("") || emptyRow(7);
}

function renderPatients() {
  document.getElementById("patientsTable").innerHTML = patients.map(p => `
    <tr><td>${esc(p.patient_id)}</td><td>${esc(p.name)}</td><td>${esc(p.age)}</td>
    <td>${esc(p.doctor)}</td><td>${esc(p.exercise_plan)}</td><td>${esc(p.registered)}</td></tr>`).join("") || emptyRow(6);
}

function renderLogs() {
  const f = document.getElementById("patientFilter").value;
  const data = f ? logs.filter(x => x.patient_id === f) : logs;
  document.getElementById("logsTable").innerHTML = data.map(x => `
    <tr><td>${esc(x.datetime)}</td><td>${esc(x.patient_id)}</td><td>${esc(x.exercise)}</td>
    <td>${esc(x.repetitions)}</td><td>${esc(x.duration_sec)} sec</td><td>${esc(x.pressure_psi)} PSI</td>
    <td>${esc(x.valve1)}</td><td>${esc(x.valve2)}</td><td class="status ${x.status.toLowerCase()}">${esc(x.status)}</td></tr>`).join("") || emptyRow(9);
}

function fillFilter() {
  const s = document.getElementById("patientFilter");
  const current = s.value;
  s.innerHTML = '<option value="">All Patients</option>' +
    patients.map(p => `<option value="${esc(p.patient_id)}">${esc(p.patient_id)} - ${esc(p.name)}</option>`).join("");
  s.value = current;
}

function openPatientForm(){document.getElementById("patientModal").classList.add("show")}
function closePatientForm(){document.getElementById("patientModal").classList.remove("show")}

document.getElementById("patientForm").addEventListener("submit", async e => {
  e.preventDefault();
  const patient = {
    patient_id: p_id.value.trim(), name: p_name.value.trim(),
    age: Number(p_age.value), doctor: p_doctor.value.trim(),
    exercise_plan: p_plan.value.trim()
  };
  try {
    await getJSON(`${API}/patients`, {method:"POST", headers:{"Content-Type":"application/json"}, body:JSON.stringify(patient)});
    e.target.reset(); closePatientForm(); await loadAll();
  } catch(err) { alert("Could not save patient: " + err.message); }
});

function emptyRow(n){return `<tr><td colspan="${n}" style="text-align:center;color:#8793a3">No records found</td></tr>`}
function esc(v){return String(v ?? "").replace(/[&<>"']/g,m=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#039;"}[m]))}
loadAll();
setInterval(loadAll, 10000);
