# Phase 5 fault matrix

| Fault / event | Immediate software effect | Recovery requirement | Physical output |
|---|---|---|---|
| Corrupt status CRC | link qualification dropped | fresh valid frames + safety qualification | zero |
| Stale ACK | link qualification dropped | fresh valid frames + safety qualification | zero |
| Wrong session echo | link qualification dropped | fresh session-valid frames | zero |
| Controller restart | controller session changes, link synchronizes | new-session link qualification + neutral safety qualification | zero |
| Remote controller fault | link enters FAULT, old valid-frame count discarded | two fresh valid frames + safety qualification | zero |
| Remote not ready | remote-ready prerequisite false | controller reports ready/brakes + qualification | zero |
| Response silence | timeout -> OFFLINE | responses resume + qualification | zero |
| Invalid configuration | requested path inhibited | valid decoded configuration | zero |
| Task health failure | authorization blocked | mandatory task progress healthy again + safety qualification | zero |
| Power-good loss | input invalid | power restored + valid input + safety qualification | zero |
| DMA overrun | input invalid | clean acquisition + safety qualification | zero |
| Stale ADC snapshot | input invalid | fresh acquisition + safety qualification | zero |
| ADC rail/range fault | input invalid | plausible input + safety qualification | zero |
| CALIBRATION / SERVICE mode | speed ceiling forced to zero | explicit qualified mode transition | zero |
| Firmware reboot with stick displaced | boot/link requalification begins, neutral absent | stick neutral for full dwell | zero |
| 32-bit tick rollover | unsigned elapsed-time paths continue | none if prerequisites remain valid | zero |

The table is intentionally repetitive in the last column. That is the invariant we are protecting.
