# CHC-104B-M2 Reference Joystick

The CHC-104B-M2 is the first joystick planned for Phase-3 bench characterization.

That makes it the reference test article, not a hard-coded electrical truth table.

## What the firmware records

A completed calibration derived from this stick can carry:

```text
reference_profile = JOYSTICK_CONFIG_REFERENCE_CHC_104B_M2
```

The actual calibration still comes from measured ADC behavior:

```text
X minimum
X center
X maximum
X center noise / deadband
Y minimum
Y center
Y maximum
Y center noise / deadband
```

## What Phase 3 refuses to invent

Until the physical joystick is connected to the real board we do not claim:

- exact endpoint ADC counts;
- exact center ADC counts;
- center-noise amplitude;
- production deadband;
- axis direction/inversion;
- mechanical symmetry;
- return-to-center repeatability;
- temperature drift;
- supply sensitivity;
- open-wiper behavior on the assembled system.

The host tests use synthetic CHC-104B-M2-shaped data only to prove the calibration machinery can handle asymmetry, center noise, endpoint guards, and bad sweeps.

Those synthetic numbers are test vectors. They are not datasheet claims and they are not production calibration defaults.

## Bench plan when hardware arrives

1. Verify joystick supply and ground at the connector.
2. Capture raw center data for both axes without touching the stick.
3. Repeat center capture after deliberate movement and release.
4. Sweep all four cardinal directions repeatedly.
5. Sweep diagonals to make sure mechanical interaction does not reduce one axis unexpectedly.
6. Record stable extrema, not one-sample peaks.
7. Evaluate center noise and return-to-center spread.
8. Inject connector/open-wiper faults and observe the existing RC network behavior.
9. Choose endpoint guard and deadband from measured data.
10. Re-run the same traces through the Phase-3 host model and compare the expected normalized output.

The hardware has opinions. Phase 3 gives us a place to record them instead of pretending they do not exist.
