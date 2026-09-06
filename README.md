IV Drip Monitoring and Control System

An ESP32-C3-based IV drip monitoring and control prototype that
detects individual drops using an infrared optical sensor, calculates
drip rate and estimated flow, displays information on a 128×64 I²C OLED,
and uses a servo-driven external tube clamp to demonstrate automatic
flow adjustment.

> **Important:** This project is intended for educational, laboratory,
> and research prototyping only. It is **not a certified medical
> device** and must not be used to control or monitor an actual
> patient’s IV infusion without appropriate medical-device engineering,
> validation, risk management, electrical safety, sterility
> considerations, and regulatory approval.

✨ Features

• 💧 Optical detection of individual drops
• 🔴 IR LED + phototransistor drop sensor
• 🧠 ESP32-C3 SuperMini controller
• 📟 128×64 I²C OLED display
• ⚙️ Servo-driven external tube clamp for prototype flow control
• 📈 Drops-per-minute (DPM / gtt/min) calculation
• 🧪 Estimated flow-rate calculation in mL/hr
• 🎯 Configurable target drip rate
• ⚠️ Low-flow, high-flow, no-drop, and low-volume alerts
• ⏸️ Pause / resume interface concept
• 🔋 Battery-powered portable architecture
• 🔊 Optional buzzer for audible alerts

🧩 System Overview

```text
              IV FLUID BAG
                   │
                   ▼
            ┌──────────────┐
            │ Drip Chamber │
            └──────┬───────┘
                   │
             💧  💧  💧
                   │
        ┌──────────┴──────────┐
        │   IR DROP SENSOR    │
        │ IR LED → Receiver   │
        └──────────┬──────────┘
                   │
                   │ Drop pulses
                   ▼
          ┌──────────────────┐
          │    ESP32-C3      │
          │    SuperMini     │
          └───────┬──────────┘
                  │
       ┌──────────┼───────────┐
       │          │           │
       ▼          ▼           ▼
   OLED LCD    Servo       Buzzer
   128×64      Clamp       Optional
       │          │
       │          ▼
       │    External tube
       │       clamp
       │
       ▼
 Flow / Rate / Alerts
```

🔬 How It Works

1. An IR LED sends infrared light across the drip chamber.
2. A phototransistor detects the IR beam.
3. When a drop passes through the beam, the received light changes.
4. The ESP32-C3 detects this transition as a pulse.
5. The controller counts the pulses and calculates drops per minute.
6. The IV administration set’s drop factor is used to estimate mL/hr.
7. The measured rate is compared with the configured target.
8. For a laboratory prototype, the ESP32 can adjust a servo-operated
external clamp.
9. The OLED continuously displays the current state and flow
information.
10. Optional alarms indicate abnormal conditions.

📐 Flow-Rate Calculation

The estimated flow rate is calculated from the drip rate and the
administration set’s drop factor:

```text
Flow (mL/hr) = [Drops/min × 60] / [Drop factor (drops/mL)]
```

Example

For:

```text
Drip rate  = 30 drops/min
Drop factor = 20 drops/mL
```

The estimated flow is:

```text
Flow = (30 × 60) / 20
     = 90 mL/hr
```

Drop factor

The drop factor must match the IV administration set being tested.
Typical sets can have different calibration factors, such as:

• 10 drops/mL
• 15 drops/mL
• 20 drops/mL
• 60 drops/mL (microdrip)

Do not assume a single drop factor for every IV set.

🛠️ Hardware

Component                         Purpose

────────

ESP32-C3 SuperMini                Main controller
IR LED                            Infrared transmitter
Phototransistor                   Infrared receiver
220 Ω resistor                    IR LED current limiting
10 kΩ resistor                    Phototransistor pull-up
128×64 I²C OLED (SSD1306)         User display
Servo motor (e.g. SG90/MG90S)     Prototype external tube clamp
Buzzer                            Optional audible alarm
Li-ion battery + protection/BMS   Portable power
5 V regulator/boost converter     Servo supply when required
IV drip chamber/tubing            Test setup
Mechanical clamp                  External tube-flow mechanism

🔌 Suggested ESP32-C3 Pin Assignment

> GPIO availability can vary between ESP32-C3 SuperMini board variants.
> Verify the labels on your specific board before wiring.

Function             ESP32-C3 GPIO Notes

────────

IR sensor output             GPIO4 Digital drop pulse input
OLED SDA                    GPIO20 I²C data
OLED SCL                    GPIO21 I²C clock
Servo signal                 GPIO3 PWM
Buzzer                       GPIO2 Optional
ESP32 supply                 3.3 V Use regulated supply
Servo supply                   5 V Prefer a separate supply rail
Ground                         GND Common ground

⚡ Sensor Circuit

A simple phototransistor receiver can be arranged as:

```text
              3.3 V
                │
               10kΩ
                │
                ├────────── GPIO4
                │
          Collector
         ┌──────────┐
IR ─────►│Phototran.│
receiver └──────────┘
          Emitter
                │
               GND
```

IR transmitter:

```text
ESP32 / 3.3 V
     │
    220Ω
     │
   IR LED
     │
    GND
```

The IR LED and phototransistor should be positioned opposite each other
so the falling drop interrupts the optical path.
## 🔌 Circuit Diagram

The complete Fritzing-style circuit diagram is available in the repository:

![IV Drip Monitoring Circuit Diagram](https://raw.githubusercontent.com/aflahsidhique/IV-Automation/main/diagram.png)

**[View the full-size circuit diagram on GitHub](https://github.com/aflahsidhique/IV-Automation/blob/main/diagram.png)**

📟 OLED Interface

The recommended display is a 128×64 monochrome I²C OLED, commonly
based on the SSD1306 controller.

Example main screen:

```text
┌────────────────────────┐
│ IV FLOW           🔋95%│
│                        │
│   30 gtt/min  90 mL/hr │
│                        │
│ Infused: 250 mL        │
│ Time:   00:10:05       │
│ ▶ RUNNING              │
└────────────────────────┘
```

Possible alert screens:

```text
┌────────────────────────┐
│       ⚠ LOW FLOW       │
│                        │
│       15 gtt/min       │
│                        │
│     CHECK IV LINE      │
└────────────────────────┘
```

```text
┌────────────────────────┐
│       ⚠ HIGH FLOW      │
│                        │
│       50 gtt/min       │
│                        │
│     CHECK CLAMP        │
└────────────────────────┘
```

```text
┌────────────────────────┐
│       ⚠ FLOW HALT      │
│                        │
│       0 gtt/min        │
│                        │
│   NO DROPS DETECTED    │
└────────────────────────┘
```

⚙️ Servo Clamp

The servo is intended to operate an external, non-sterile mechanical
clamp around the tubing.

```text
             Servo
               │
               ▼
        ┌─────────────┐
        │ Servo Arm   │
        └──────┬──────┘
               │
               ▼
        ┌─────────────┐
        │    TUBE     │
        │  ─────────  │
        └─────────────┘
               │
        Flow changes as
        clamp position changes
```

The servo should not enter the fluid path and should not damage or
compromise the sterile tubing.

For the prototype control algorithm:

```text
Measured flow < target
        ↓
Slightly open clamp

Measured flow ≈ target
        ↓
Hold position

Measured flow > target
        ↓
Slightly close clamp
```

Use small position changes and an averaging period rather than adjusting
the servo for every individual drop.

🧠 Control Logic

A high-level control loop can be implemented as:

```text
START
  │
  ▼
Initialize OLED, sensor, servo
  │
  ▼
Detect drop
  │
  ▼
Increment drop counter
  │
  ▼
Calculate DPM
  │
  ▼
Calculate estimated mL/hr
  │
  ▼
Update OLED
  │
  ├── Flow too low? ──► Alert + prototype servo adjustment
  │
  ├── Flow too high? ─► Alert + prototype servo adjustment
  │
  ├── No drops? ──────► Flow halt alert
  │
  └── Normal? ────────► Maintain / hold
  │
  ▼
Repeat
```

🖥️ Suggested UI States

The OLED interface can contain:

1. Home / Monitoring
  • Current gtt/min
  • Estimated mL/hr
  • Infused volume
  • Elapsed time
2. Settings
  • Target drip rate
  • Target volume
  • Drop factor
3. Low Flow Alert
  • Rate below configured threshold
4. High Flow Alert
  • Rate above configured threshold
5. Flow Halt
  • No drops detected for a defined period
6. Low Volume Alert
  • Estimated remaining volume below threshold
7. Pause
  • Pause / resume / stop menu
8. Resume
  • Continue the monitoring session

📦 Software

The firmware can be developed using:

• Arduino IDE
• ESP32 Arduino core
• Wire.h
• Adafruit_GFX
• Adafruit_SSD1306
• ESP32Servo or another ESP32-compatible servo library

Typical Arduino IDE configuration:

```text
Board:
ESP32C3 Dev Module
```

Select the appropriate USB/serial settings for the particular ESP32-C3
SuperMini board.
## 🎥 Working Video

A working demonstration of the prototype is available here:

[**▶️ Watch the working video on YouTube**](https://youtube.com/shorts/cJuZJ1mvMUA?si=qyw3it6B-IV5D7mZ)
📁 Suggested Repository Structure

```text
iv-drip-monitor/
│
├── README.md
├── LICENSE
│
├── firmware/
│   └── iv_drip_monitor.ino
│
├── hardware/
│   ├── circuit/
│   ├── pcb/
│   └── mechanical/
│
├── diagrams/
│   ├── system-block-diagram.png
│   ├── circuit-diagram.png
│   └── mechanical-clamp.png
│
├── docs/
│   ├── design.md
│   ├── calibration.md
│   └── testing.md
│
└── images/
    └── prototype.jpg
```

🧪 Calibration

Calibration is critical because the optical sensor measures drops,
not fluid volume directly.

A basic laboratory calibration procedure:

1. Select the IV administration set used for testing.
2. Determine its specified drop factor.
3. Run a known quantity of test fluid through the system.
4. Count the detected drops.
5. Compare the electronic count with a manual/reference count.
6. Adjust sensor alignment and filtering.
7. Repeat at several drip rates.
8. Record false positives and missed drops.
9. Verify the calculated mL/hr against a reference measurement.

The sensor should be shielded from strong ambient light, and the drip
chamber should be held rigidly so every drop crosses the optical beam.

🚨 Fault Detection

Potential conditions to detect:

Condition                 Example response

────────

Normal flow               Display rate
Low flow                  OLED alert + optional buzzer
High flow                 OLED alert + optional buzzer
No drops                  Flow-halt alert
Sensor fault              Sensor error
Battery low               Battery warning
Target reached            Infusion-complete notification
Excess estimated volume   Overflow/limit alert

🔋 Power Architecture

For a portable prototype:

```text
Li-ion Battery
      │
      ▼
Battery Protection / BMS
      │
      ├──────────────► 3.3 V regulator ──► ESP32-C3
      │
      └──────────────► 5 V converter ─────► Servo
```

The servo can produce electrical noise and current spikes, so it is
preferable to give it a suitable separate regulated supply while keeping
the grounds appropriately referenced.

Do not power a servo directly from the ESP32-C3’s 3.3 V output.

⚠️ Safety

This repository describes a prototype engineering project.

• Do not connect the electronics to the patient’s body.
• Do not place homemade electronics or sensors inside the sterile
fluid path.
• The servo should operate an external clamp only.
• Do not rely on the prototype as the sole means of determining or
controlling a patient’s infusion.
• Use appropriate electrical isolation and battery protection.
• Test using non-clinical fluids and a laboratory setup.
• Validate the optical sensor across the expected range of drip rates.
• Implement fail-safe behavior so loss of power or controller failure
does not create an unsafe mechanical state.
• Any clinical deployment requires formal verification, validation,
risk management, usability assessment, electrical safety testing,
and applicable regulatory approval.

🚧 Current Project Status

Prototype / Development

Planned development stages:

☑ System concept
☑ IR drop detection concept
☑ ESP32-C3 architecture
☑ OLED interface concept
☑ Servo clamp concept
☐ Final sensor circuit
☐ Firmware implementation
☐ Calibration
☐ Mechanical enclosure
☐ Bench testing
☐ Flow-rate accuracy testing
☐ Fault-condition testing
☐ Power/battery testing

🤝 Contributing

Contributions are welcome for educational and research development.

Useful contributions include:

• Improved optical sensing
• Better drop filtering/debouncing
• OLED UI improvements
• Calibration algorithms
• Servo control algorithms
• Mechanical clamp designs
• Battery monitoring
• Fault detection
• Test procedures
• Documentation

📄 License

Choose an appropriate open-source license before publishing the
repository. MIT is a common choice for educational hardware/software
projects, but confirm that it fits your intended use.

────────

Project Summary

IV Drip Monitoring and Control System combines an IR optical drop
sensor, ESP32-C3 SuperMini, 128×64 I²C OLED, and servo-driven external
tube clamp to create a compact prototype capable of measuring drip rate,
estimating flow, displaying infusion information, generating alerts, and
demonstrating closed-loop flow control.

Designed for engineering education and prototyping — not for direct
clinical use.
