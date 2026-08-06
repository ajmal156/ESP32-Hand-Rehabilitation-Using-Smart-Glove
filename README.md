# 🧤 ESP32-Hand-Rehabilitation-Using-Smart-Glove

An ESP32-based pneumatic hand rehabilitation system that performs physiotherapy exercises using pneumatic actuators, a 4×4 keypad interface, and a web-based patient management system. The project is designed for rehabilitation research and can be expanded with IoT, cloud storage, and doctor monitoring features.

---

# 📂 Repository Structure

```text
ESP32-Hand-Rehabilitation-Using-Smart-Glove/
│
├── 📁 physioGlove/
│   ├── main.ino                 # Main program
│   ├── keypad.ino               # 4×4 keypad interface
│   ├── glove_hand.ino           # Pneumatic glove control
│   ├── server_wifi.ino          # Wi-Fi communication
│   └── README.md
│
├── 📁 python_flask_server/
│   ├── server.py                # Flask backend server
│   ├── requirements.txt         # Python dependencies
│   ├── patients.csv             # Patient records
│   └── database/
│
├── 📁 web_page/
│   ├── login.html               # Patient login page
│   └── images/
│
├── 📁 DOC/
│   ├── Project_Report.pdf
│   ├── Hardware_Diagram.pdf
│   ├── Circuit_Diagram.pdf
│   ├── Flowchart.pdf
│   ├── User_Manual.pdf
│   ├── Presentation.pptx
│   └── Research_Paper.pdf
│
├── LICENSE
└── README.md
```

---

# 📁 Folder Description

## 📂 physioGlove/

This folder contains all ESP32 firmware responsible for controlling the pneumatic rehabilitation glove.

### Files

| File                | Description                                |
| ------------------- | ------------------------------------------ |
| **main.ino**        | Main application entry point               |
| **keypad.ino**      | Reads the 4×4 keypad and selects exercises |
| **glove_hand.ino**  | Controls pressure and release valves       |
| **server_wifi.ino** | Wi-Fi communication with the Flask server  |

---

## 📂 python_flask_server/

This folder contains the backend responsible for communication between the ESP32 and the web application.

### Files

| File                 | Description                         |
| -------------------- | ----------------------------------- |
| **server.py**        | Flask web server                    |
| **requirements.txt** | Required Python packages            |
| **patients.csv**     | Stores patient information          |
| **database/**        | Future database and patient records |

---

## 📂 web_page/

This folder contains the website used for patient login and monitoring.

### Files

| File               | Description               |
| ------------------ | ------------------------- |
| **login.html**     | Login page                |
| **dashboard.html** | Future patient dashboard  |

---

## 📂 DOC/

Project documentation.

### Documents

* Project Report
* Hardware Diagram
* Circuit Diagram
* Flowchart
* User Manual
* Presentation Slides
* Research Paper

---

# 🚀 System Workflow

```text
Patient
   │
   ▼
Login Web Page
   │
   ▼
Flask Server
   │
   ▼
ESP32 Controller
   │
   ▼
4×4 Keypad
   │
   ▼
Exercise Selection
   │
   ▼
Relay Module
   │
   ▼
Pressure Valve
   │
   ▼
Pneumatic Smart Glove
   │
   ▼
Hand Rehabilitation
```

---

# 📌 Future Development

* Independent five-finger pneumatic control
* Pressure sensor feedback
* Force monitoring
* Mobile application
* Firebase integration
* Doctor dashboard
* Patient history database
* Cloud storage
* AI-assisted rehabilitation analysis

---

# 👨‍💻 Author

**Ajmal Khan**

Department of Robotics and Intelligent Systems

University of Haripur

Pakistan

