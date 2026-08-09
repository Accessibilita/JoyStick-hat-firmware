# Phase 3 Architecture — Calibration, Configuration, and Command Shaping

Phase 1 built the safety foundation. Phase 2 built the motor-link contract. Phase 3 finally gives the joystick data a controlled path from raw ADC counts into a requested drive command.

Raw joystick data is still not a motor command.

Phase 3 makes that statement more concrete by putting three explicit transformations between the ADC and the Phase-2 authorization boundary:

```text
raw ADC counts
    ↓
electrical diagnostics
    ↓
calibration
    ↓
normalized signed Q15 axes
    ↓
deadband + response shaping + speed limit
    ↓
RequestedDriveCommand
    ↓
Phase-2 logical authorization
    ↓
physical drive lock
```

The physical drive lock remains in place. Phase 3 is software development ahead of hardware availability, not permission to move a chair.

## Reference joystick

The initial bench reference is the CHC-104B-M2.

The firmware does not hard-code assumed CHC-104B-M2 endpoints, center voltage, center noise, or required deadband. Those are exactly the parameters we need the real part on the bench to measure.

The reference profile is therefore an identifier, not a magic table of fake measurements.

```text
JOYSTICK_CONFIG_REFERENCE_CHC_104B_M2
```

A completed calibration record carries the reference profile so test records and future field data can say what hardware they were derived from.

## Calibration model

Each axis gets four values and one direction bit:

```text
minimum_counts
center_counts
maximum_counts
deadband_counts
inverted
```

Minimum and maximum are *usable* endpoints after an endpoint guard is applied. We do not normalize against one lucky peak sample collected while somebody was slamming the stick into a mechanical stop.

Center capture is a bounded sample set. The software records:

- center average;
- center minimum;
- center maximum;
- observed center noise span.

Sweep capture is another bounded operation. It records the reached minimum and maximum for both axes.

Finalization rejects a calibration if either side of center does not have enough usable travel.

The software-default calibration policy is intentionally provisional. It exists so the algorithm can be tested before hardware arrives. Real CHC-104B-M2 measurements get to overrule it.

## Asymmetric normalization

Center does not have to sit halfway between the endpoints.

For an axis like:

```text
min = 500
center = 1800
max = 3900
```

the negative and positive sides are scaled independently. That prevents one short side of a real potentiometer from distorting the other side.

The deadband is removed before scaling. Counts inside the center deadband produce exactly zero. Counts outside the deadband are mapped to signed Q15:

```text
-32767 ... 0 ... +32767
```

The code deliberately avoids `-32768`, which means inversion is always representable without a signed-overflow corner case.

## Response shaping

The Phase-3 shaping function blends between linear and cubic response in integer Q15 math.

```text
curve = 0       → linear
curve = 32767   → cubic
```

Intermediate values blend the two.

A cubic curve reduces sensitivity around center without changing the full-scale endpoint. That is a useful software primitive, but the final production curve is a human-factors and hardware-validation decision. Phase 3 proves the math, not the preferred wheelchair feel.

## Speed limiting

Maximum speed is a Q15 scalar applied after response shaping.

This gives us a clean separation:

```text
calibration = what the hardware physically does
response     = how the stick should feel
speed limit  = how much command authority is available
```

Those are different engineering problems and they stay different in the data model.

## Requested command

Phase 3 maps:

```text
Y axis → forward_q15
X axis → turn_q15
```

through `JoystickProcessing_BuildRequestedDriveCommand()`.

The request contains sequence, input sequence, generation time, forward, turn, maximum speed, and enable request.

If configuration validation or processing fails, the function fails closed:

```text
forward = 0
turn = 0
maximum_speed = 0
enable_request = false
```

## Persistent configuration boundary

Phase 3 defines the persistent record format and redundant-slot selection logic, but it does not write MCU flash.

That is deliberate.

The safety path already says flash writes do not belong in normal drive operation. We can prove record encoding, CRC handling, generation ordering, and torn-write recovery on the host without pretending the eventual STM32 flash transaction and brownout behavior have been measured.

The future storage backend will own the physical flash operation. The portable Phase-3 configuration module owns what a valid record means.

## Transaction model

The persistent model assumes two slots:

```text
slot A
slot B
```

Each complete record contains its own generation number and CRC32.

On startup:

1. validate both slots independently;
2. reject malformed/version-mismatched/CRC-failed records;
3. select the valid record with the highest generation;
4. fail configuration validity if neither slot is valid.

A future write operation should target the inactive/older slot, write the complete next generation, verify it, and only then allow that record to become authoritative.

Power loss during the write should therefore leave either the old complete record or the new complete record. A half-written record does not get a vote.

## What remains physically inhibited

Phase 3 does not change these facts:

- `APP_RS485_PHYSICAL_LINK_ENABLE == 0`;
- the existing PCB RS-485 direction conflict remains unresolved;
- runtime `configuration_valid` in Safety Control remains false;
- the physical `AppAuthorizedDriveCommand` remains zero;
- `drive_authorized` remains false;
- no Phase-3 module calls STM32 flash-programming functions.

We are building the software model right up to the hardware boundary and stopping there on purpose.
