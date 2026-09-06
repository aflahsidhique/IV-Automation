# IV Drip Monitoring and Control System

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform: ESP32-C3](https://img.shields.io/badge/Platform-ESP32--C3-informational.svg)](#hardware)
[![Status: Prototype](https://img.shields.io/badge/Status-Prototype-orange.svg)](#project-status)

An ESP32-C3-based IV drip monitoring and control prototype that detects
individual drops using an infrared optical sensor, calculates drip rate
and estimated flow, displays information on a 128×64 I²C OLED, and uses a
servo-driven external tube clamp to demonstrate automatic flow adjustment.

> **Important**: This project is intended for educational, laboratory, and
> research prototyping only. It is **not a certified medical device** and
> must not be used to control or monitor an actual patient's IV infusion
> without appropriate medical-device engineering, validation, risk
> management, electrical safety, sterility considerations, and regulatory
> approval. See [Safety](#safety).

## Contents

- [Features](#features)
- [System Architecture](#system-architecture)
- [How It Works](#how-it-works)
- [Flow-Rate Calculation](#flow-rate-calculation)
- [Hardware](#hardware)
- [Repository Structure](#repository-structure)
- [Software](#software)
- [Calibration](#calibration)
- [Fault Detection](#fault-detection)
- [Safety](#safety)
- [Project Status](#project-status)
- [Contributing](#contributing)
- [License](#license)

## Features

- Optical detection of individual drops via an IR LED/phototransistor pair
- ESP32-C3 SuperMini controller
- 128×64 I²C OLED display
- Servo-driven external tube clamp for prototype flow control
- Drops-per-minute (DPM / gtt/min) and estimated mL/hr calculation
- Configurable target drip rate
- Low-flow, high-flow, no-drop, and low-volume alerts
- Pause/resume interface concept
- Battery-powered, portable architecture with optional buzzer alerts

## System Architecture

```text
              IV FLUID BAG
                   │
                   ▼
            ┌──────────────┐
            │ Drip Chamber │
            └──────┬───────┘
                   │
                  drops
                   │
        ┌──────────┴──────────┐
        │   IR DROP SENSOR    │
        │ IR LED → Receiver   │
        └──────────┬──────────┘
                   │ drop pulses
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
   128×64      Clamp       (optional)
       │          │
       │          ▼
       │    External tube clamp
       ▼
 Flow / Rate / Alerts
```

Full design detail — pin assignment, sensor circuit, OLED interface, servo
control, and power architecture — is in [docs/design.md](docs/design.md).

## How It Works

1. An IR LED sends infrared light across the drip chamber.
2. A phototransistor detects the IR beam.
3. When a drop passes through the beam, the received light changes.
4. The ESP32-C3 detects this transition as a pulse.
5. The controller counts the pulses and calculates drops per minute.
6. The IV administration set's drop factor is used to estimate mL/hr.
7. The measured rate is compared with the configured target.
8. As a laboratory prototype, the ESP32 can adjust a servo-operated
   external clamp to correct the flow.
9. The OLED continuously displays the current state and flow information.
10. Optional alarms indicate abnormal conditions.

## Flow-Rate Calculation

```text
Flow (mL/hr) = (Drops/min × 60) / Drop factor (drops/mL)
```

**Example** — 30 drops/min at a drop factor of 20 drops/mL:

```text
Flow = (30 × 60) / 20 = 90 mL/hr
```

The drop factor must match the IV administration set being tested (common
values: 10, 15, 20, or 60 drops/mL for microdrip sets). Do not assume a
single drop factor for every IV set — see [Calibration](#calibration).

## Hardware

| Component | Purpose |
|---|---|
| ESP32-C3 SuperMini | Main controller |
| IR LED | Infrared transmitter |
| Phototransistor | Infrared receiver |
| 220 Ω resistor | IR LED current limiting |
| 10 kΩ resistor | Phototransistor pull-up |
| 128×64 I²C OLED (SSD1306) | User display |
| Servo motor (e.g. SG90/MG90S) | Prototype external tube clamp |
| Buzzer | Optional audible alarm |
| Li-ion battery + protection/BMS | Portable power |
| 5 V regulator/boost converter | Servo supply when required |
| IV drip chamber/tubing | Test setup |
| Mechanical clamp | External tube-flow mechanism |

Pin assignment, sensor circuit schematics, and PCB/mechanical notes are in
[docs/design.md](docs/design.md) and [hardware/](hardware/). The full
circuit diagram is at [diagrams/circuit-diagram.png](diagrams/circuit-diagram.png).

## Repository Structure

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

## Software

- Arduino IDE with the ESP32 Arduino core
- `Wire.h`, `Adafruit_GFX`, `Adafruit_SSD1306`
- `ESP32Servo` or another ESP32-compatible servo library

Arduino IDE board setting: `ESP32C3 Dev Module`. Select the appropriate
USB/serial settings for your specific ESP32-C3 SuperMini board.

## Calibration

The optical sensor measures drops, not fluid volume directly, so
calibration against the specific IV administration set in use is
required before trusting any reading. See
[docs/calibration.md](docs/calibration.md) for the full procedure.

## Fault Detection

| Condition | Response |
|---|---|
| Normal flow | Display rate |
| Low flow | OLED alert + optional buzzer |
| High flow | OLED alert + optional buzzer |
| No drops | Flow-halt alert |
| Sensor fault | Sensor error |
| Battery low | Battery warning |
| Target reached | Infusion-complete notification |
| Excess estimated volume | Overflow/limit alert |

See [docs/testing.md](docs/testing.md) for the full test plan and bench
checklist.

## Safety

This repository describes a prototype engineering project.

- Do not connect the electronics to the patient's body.
- Do not place homemade electronics or sensors inside the sterile fluid
  path.
- The servo must operate an external clamp only.
- Do not rely on the prototype as the sole means of determining or
  controlling a patient's infusion.
- Use appropriate electrical isolation and battery protection.
- Test using non-clinical fluids and a laboratory setup.
- Validate the optical sensor across the expected range of drip rates.
- Implement fail-safe behavior so loss of power or controller failure does
  not create an unsafe mechanical state.
- Any clinical deployment requires formal verification, validation, risk
  management, usability assessment, electrical safety testing, and
  applicable regulatory approval.

## Project Status

Prototype / development.

A working demonstration of the current prototype is available on
[YouTube](https://youtube.com/shorts/cJuZJ1mvMUA?si=qyw3it6B-IV5D7mZ).

## Contributing

Contributions are welcome for educational and research development,
particularly:

- Improved optical sensing and drop filtering/debouncing
- OLED UI improvements
- Calibration and servo control algorithms
- Mechanical clamp designs
- Battery monitoring and fault detection
- Test procedures and documentation

## License

Released under the [MIT License](LICENSE).

---

IV Drip Monitoring and Control System combines an IR optical drop sensor,
ESP32-C3 SuperMini, 128×64 I²C OLED, and a servo-driven external tube
clamp into a compact prototype for measuring drip rate, estimating flow,
displaying infusion information, generating alerts, and demonstrating
closed-loop flow control. Designed for engineering education and
prototyping — not for direct clinical use.
