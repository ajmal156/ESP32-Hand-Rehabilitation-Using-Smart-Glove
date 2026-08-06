# 🧤 Pneumatic Physiotherapy Glove

An ESP32-based pneumatic rehabilitation glove designed for hand physiotherapy. The system uses pneumatic pressure and release valves to perform rehabilitation exercises through a simple 4×4 keypad interface.
---

## 📌 Features

- ESP32 based controller
- 4×4 Keypad interface
- Pneumatic hand rehabilitation
- Full hand exercise
- Thumb exercise
- Index finger exercise
- Middle finger exercise
- Ring & Little finger exercise
- Emergency stop
- Modular code structure
- Expandable for IoT monitoring

---

# 📷 Project Overview

```
          ESP32
             │
      4×4 Keypad
             │
             ▼
      Exercise Selection
             │
             ▼
      Relay Module (2CH)
      │             │
      ▼             ▼
 Pressure Valve   Release Valve
      │             │
      └──────┬──────┘
             │
      Pneumatic Glove
             ▲
             │
        5V Air Pump
```

---

# 🔧 Hardware

| Component | Quantity |
|------------|---------:|
| ESP32 DevKit V1 | 1 |
| 4×4 Matrix Keypad | 1 |
| 2 Channel Relay | 1 |
| 12V Pneumatic Solenoid Valve | 2 |
| 5V Air Pump | 1 |
| Pneumatic Glove | 1 |
| External 12V Power Supply | 1 |
| External 5V Power Supply | 1 |
| Air Tubes | As Required |

---

# 📍 Pin Configuration

## Keypad

| Function | GPIO |
|----------|-----:|
| Row1 | 13 |
| Row2 | 12 |
| Row3 | 14 |
| Row4 | 27 |
| Col1 | 18 |
| Col2 | 19 |
| Col3 | 21 |
| Col4 | 22 |

---

## Valves

| Function | GPIO |
|----------|-----:|
| Pressure Valve | 26 |
| Release Valve | 25 |

---

# 🎮 Exercise Modes

| Key | Exercise |
|----:|----------|
| 1 | Full Hand |
| 2 | Thumb |
| 3 | Index Finger |
| 4 | Middle Finger |
| 5 | Ring & Little Finger |
| 0 | Emergency Stop |

---

# ⚙️ Working Principle

1. User selects an exercise using the keypad.
2. ESP32 receives the selected key.
3. ESP32 activates the relay module.
4. The pressure valve inflates the glove.
5. The glove performs the rehabilitation movement.
6. The pressure valve turns OFF.
7. The release valve opens.
8. Air is exhausted from the glove.
9. The glove returns to its original position.

---

# 📂 Project Structure

```
Physio_Glove_Project/
│
├── main.ino
├── keypad.cpp
├── keypad.h
├── glove.cpp
├── glove.h
├── wifi.cpp
├── wifi.h
├── webserver.cpp
├── webserver.h
├── firebase.cpp
├── firebase.h
├── README.md
```

---

# 🚀 Future Improvements

- Independent control of all five fingers
- Pressure sensor feedback
- Mobile application
- Wi-Fi monitoring
- Firebase integration
- Patient history logging
- Exercise scheduling
- Doctor dashboard
- Cloud database
- Real-time monitoring

---

# 👨‍💻 Author

**Ajmal Khan**

Department of Robotics and Intelligent Systems

University of Haripur

Pakistan

---

# 📜 License

This project is released under the MIT License.
