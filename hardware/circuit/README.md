# Circuit

Schematic source files (e.g. Fritzing/KiCad) for the IR drop-sensor
circuit and OLED/servo/buzzer wiring described in
[docs/design.md](../../docs/design.md#sensor-circuit).

The rendered circuit diagram lives at
[`diagrams/circuit-diagram.png`](../../diagrams/circuit-diagram.png).

Quick reference — phototransistor receiver:

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
