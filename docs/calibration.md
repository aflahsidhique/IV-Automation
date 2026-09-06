# Calibration

Calibration is critical because the optical sensor measures drops, not
fluid volume directly. See the [README](../README.md) for the flow-rate
formula and overall system design.

## Drop Factor

The drop factor must match the IV administration set being tested. Typical
sets can have different calibration factors, such as:

- 10 drops/mL
- 15 drops/mL
- 20 drops/mL
- 60 drops/mL (microdrip)

Do not assume a single drop factor for every IV set.

## Procedure

1. Select the IV administration set used for testing.
2. Determine its specified drop factor.
3. Run a known quantity of test fluid through the system.
4. Count the detected drops.
5. Compare the electronic count with a manual/reference count.
6. Adjust sensor alignment and filtering.
7. Repeat at several drip rates.
8. Record false positives and missed drops.
9. Verify the calculated mL/hr against a reference measurement.

## Environment

The sensor should be shielded from strong ambient light, and the drip
chamber should be held rigidly so every drop crosses the optical beam.
