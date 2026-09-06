# Testing

Testing reference for the IV Drip Monitoring and Control System prototype.
Also see [calibration.md](calibration.md) for sensor-accuracy procedures.

## Fault Conditions to Verify

| Condition | Expected response |
|---|---|
| Normal flow | Display rate |
| Low flow | OLED alert + optional buzzer |
| High flow | OLED alert + optional buzzer |
| No drops | Flow-halt alert |
| Sensor fault | Sensor error |
| Battery low | Battery warning |
| Target reached | Infusion-complete notification |
| Excess estimated volume | Overflow/limit alert |

## Test Stages

- [ ] Final sensor circuit
- [ ] Firmware implementation
- [ ] Calibration (see [calibration.md](calibration.md))
- [ ] Mechanical enclosure
- [ ] Bench testing
- [ ] Flow-rate accuracy testing
- [ ] Fault-condition testing
- [ ] Power/battery testing

## Bench Testing Notes

- Use non-clinical fluids and a laboratory setup only.
- Validate the optical sensor across the expected range of drip rates.
- Verify the servo clamp responds correctly to low/high/target flow states
  without entering the fluid path or damaging the tubing.
- Confirm fail-safe behavior: loss of power or controller failure must not
  create an unsafe mechanical state (e.g. an unintended fully-closed or
  fully-open clamp).
- Re-run calibration whenever the IV administration set or drop factor
  changes.

See the [README safety notice](../README.md#-safety) before connecting any
hardware to fluid or tubing.
