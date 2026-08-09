# Phase 3 Persistent Joystick Configuration Format

The configuration record is deliberately boring: fixed size, explicit byte order, versioned, CRC-protected, and independent of compiler struct layout.

Do not cast flash bytes onto a C struct.

## Record size

Every record is exactly 64 bytes.

All multi-byte integers are little-endian.

| Offset | Size | Field |
|---:|---:|---|
| 0 | 4 | magic `JSC3` |
| 4 | 1 | record version = 1 |
| 5 | 1 | payload length = 30 |
| 6 | 2 | reserved = 0 |
| 8 | 4 | generation |
| 12 | 2 | X minimum counts |
| 14 | 2 | X center counts |
| 16 | 2 | X maximum counts |
| 18 | 2 | X deadband counts |
| 20 | 1 | X inverted, 0/1 |
| 21 | 1 | reserved = 0 |
| 22 | 2 | Y minimum counts |
| 24 | 2 | Y center counts |
| 26 | 2 | Y maximum counts |
| 28 | 2 | Y deadband counts |
| 30 | 1 | Y inverted, 0/1 |
| 31 | 1 | reserved = 0 |
| 32 | 2 | maximum speed Q15 |
| 34 | 2 | response curve Q15 |
| 36 | 2 | reference profile |
| 38 | 22 | reserved = 0 |
| 60 | 4 | CRC32 |

## CRC

CRC is standard reflected CRC-32/IEEE over bytes 0 through 59:

```text
polynomial (reflected): 0xEDB88320
initial value:          0xFFFFFFFF
xor out:                0xFFFFFFFF
```

Golden vector:

```text
"123456789" → 0xCBF43926
```

## Validation

A CRC match is necessary, not sufficient.

A decoded record still has to pass semantic validation:

- generation is nonzero;
- minimum < center < maximum;
- ADC values fit the 12-bit domain;
- each side of center has a minimum useful span;
- deadband is nonzero and smaller than both calibrated sides;
- maximum speed is 1..32767;
- response curve is 0..32767;
- reserved bytes are zero.

A corrupted record that happens to contain a plausible CRC is still not automatically a usable configuration.

## Redundant slots

Phase 3 models two redundant slots and selection of the newest valid generation.

It does not yet implement the STM32 flash erase/program sequence. That backend belongs to a hardware-validation phase because erase timing, sector ownership, brownout behavior, and reset behavior need to be proven on the actual MCU/board.
