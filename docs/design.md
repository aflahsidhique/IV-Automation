# Design

System design reference for the IV Drip Monitoring and Control System. See
the [README](../README.md) for the project overview and safety notice.

## System Overview

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

## How It Works

1. An IR LED sends infrared light across the drip chamber.
2. A phototransistor detects the IR beam.
3. When a drop passes through the beam, the received light changes.
4. The ESP32-C3 detects this transition as a pulse.
5. The controller counts the pulses and calculates drops per minute.
6. The IV administration set's drop factor is used to estimate mL/hr.
7. The measured rate is compared with the configured target.
8. For a laboratory prototype, the ESP32 can adjust a servo-operated
   external clamp.
9. The OLED continuously displays the current state and flow information.
10. Optional alarms indicate abnormal conditions.

## Flow-Rate Calculation

```text
Flow (mL/hr) = [Drops/min × 60] / [Drop factor (drops/mL)]
```

Example, for a drip rate of 30 drops/min and a drop factor of 20 drops/mL:

```text
Flow = (30 × 60) / 20
     = 90 mL/hr
```

The drop factor must match the IV administration set being tested. Typical
sets use different calibration factors — 10, 15, 20, or 60 (microdrip)
drops/mL. Do not assume a single drop factor for every IV set.

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

## Suggested ESP32-C3 Pin Assignment

> GPIO availability can vary between ESP32-C3 SuperMini board variants.
> Verify the labels on your specific board before wiring.

| Function | ESP32-C3 GPIO | Notes |
|---|---|---|
| IR sensor output | GPIO4 | Digital drop pulse input |
| OLED SDA | GPIO20 | I²C data |
| OLED SCL | GPIO21 | I²C clock |
| Servo signal | GPIO3 | PWM |
| Buzzer | GPIO2 | Optional |
| ESP32 supply | 3.3 V | Use regulated supply |
| Servo supply | 5 V | Prefer a separate supply rail |
| Ground | GND | Common ground |

## Sensor Circuit

Phototransistor receiver:

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

The IR LED and phototransistor should be positioned opposite each other so
the falling drop interrupts the optical path. The full Fritzing-style
circuit diagram is in [`diagrams/circuit-diagram.png`](../diagrams/circuit-diagram.png).

## OLED Interface

128×64 monochrome I²C OLED, commonly based on the SSD1306 controller.

Home screen:

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

Alert screens (low flow, high flow, flow halt) follow the same layout with
the relevant condition and reading substituted. See the
[README](../README.md#-oled-interface) for the full set of examples.

## Suggested UI States

1. Home / Monitoring — gtt/min, mL/hr, infused volume, elapsed time
2. Settings — target drip rate, target volume, drop factor
3. Low Flow Alert — rate below configured threshold
4. High Flow Alert — rate above configured threshold
5. Flow Halt — no drops detected for a defined period
6. Low Volume Alert — estimated remaining volume below threshold
7. Pause — pause / resume / stop menu
8. Resume — continue the monitoring session

## Servo Clamp

The servo operates an external, non-sterile mechanical clamp around the
tubing. It must never enter the fluid path or compromise the sterile
tubing.

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

Prototype control algorithm:

```text
Measured flow < target   → Slightly open clamp
Measured flow ≈ target   → Hold position
Measured flow > target   → Slightly close clamp
```

Use small position changes and an averaging period rather than adjusting
the servo for every individual drop.

## Control Logic

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

## Power Architecture

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

The servo can produce electrical noise and current spikes, so give it a
separate regulated supply while keeping the grounds appropriately
referenced. Do not power a servo directly from the ESP32-C3's 3.3 V output.

## Software

- Arduino IDE
- ESP32 Arduino core
- Wire.h
- Adafruit_GFX
- Adafruit_SSD1306
- ESP32Servo or another ESP32-compatible servo library

Arduino IDE board setting: `ESP32C3 Dev Module`. Select the appropriate
USB/serial settings for the particular ESP32-C3 SuperMini board.
