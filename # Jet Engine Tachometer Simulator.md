# Jet Engine Tachometer Simulator
**Designed By:** Larry Cameron, Ph.D., MSW  
**Version:** 1.2  
**Language:** C++17  
**Project Type:** Avionics Simulation – Jet Engine RPM & Power-Band Modeling

---

## 📘 Overview
This simulator models the behavior of a **jet engine tachometer**, converting angular velocity (rad/sec) into RPM, filtering the signal, determining engine power-bands, and generating diagnostic outputs. The program also logs simulation results to a CSV file for later analysis.

This project is designed using **Object-Oriented Programming (OOP)** principles and structured for clarity, modularity, and avionics-style simulation logic.

---

## 🚀 Features
- Real-time RPM conversion from rad/sec → RPM  
- Filtered tachometer signal (rounded to nearest whole RPM)
- Power-band classification using `enum class EnginePowerBand`
- Built-in RPM thresholds:
  - PowerOff  
  - Idle  
  - Climb  
  - Cruise  
  - Caution  
  - RedLine  
  - OverLimit  
- System self-check and diagnostics
- Flight log auto-generated as `flight_log.csv`
- Randomized simulation engine for testing RPM fluctuations
- Clean, modular class design (EnginePowerModel)

---

## 🛠 Engine Power Bands
The simulator uses **well-defined RPM thresholds** to classify engine state:

| Power Band | Description | Typical Range (RPM) |
|-----------|-------------|---------------------|
| PowerOff | Engine not turning | 0 |
| Idle | Minimum stable operating RPM | 1000–3500 |
| Climb | Increased thrust for ascent | 3501–6000 |
| Cruise | Efficient power for level flight | 6001–8500 |
| Caution | High RPM requiring monitoring | 8501–9500 |
| RedLine | Maximum allowable limit | 9501–10000 |
| OverLimit | Dangerous, possible engine damage | >10000 |

---

## 🧩 Class Structure

### **`enum class EnginePowerBand`**
Represents the engine’s power state.  
Each enumerator corresponds to a specific operational band.

### **`class EnginePowerModel`**
Handles:
- RPM measurement input  
- Filtering  
- Power band evaluation  
- Diagnostic reporting  
- Logging  
- Simulation control  

**Key Private Variables**
- `m_raw_rpm` – raw computed RPM from rad/sec  
- `m_filtered_rpm` – rounded RPM   
- `m_powerBand` – current engine band   

**Key Public Methods**
- `update_from_rpm(double angular_speed_rad_per_sec)`  
- `determine_power_band()`  
- `run_diagnostics()`  
- `log_to_csv()`  

---

## 📂 File Structure
/JetEngineTachometer/
│
├── Tachometer_Simulator1.2.cpp
├── EnginePowerModel.hpp
├── EnginePowerModel.cpp
├── flight_log.csv
└── README.md