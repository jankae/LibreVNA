# Frequency-plan check (host)

Compiles the firmware's own `Si5351C`, `MAX2871` and `Algorithm` drivers on a PC with a
stub HAL. It then replays the frequency choices of `VNA::Setup()` and
`VNA::SweepHalted()` for one sweep.

The register values the drivers produce are decoded back into the frequencies the
hardware actually makes:

- Si5351 PLLs and multisynths, from the I2C register file (AN619 P1/P2/P3)
- MAX2871 outputs, from the N/FRAC/M/DIVA fields sent to the FPGA, using the Si5351
  reference output that is active at that point
- the DFT bin, from the FPGA clock, ADC prescaler and phase increment

The result is the true offset of the final IF from the DFT bin at every point, for all
three receivers. The tool prints it next to the firmware's own estimate.

It exists to catch the bug fixed in 8b44421: the firmware believed it was using a
99 MHz reference that was really 99 MHz + 3.9 Hz. At 10 Hz IFBW, blocks of points lost
70–150 dB.

```
./run_checks.sh ../../../.. [LIMIT]       # exit 1 if any point is off by more than LIMIT x IFBW (default 0.5)
./build.sh ../../../.. /tmp/fwplan && /tmp/fwplan 1000000 6000000000 501 10 > points.csv
```

`build.sh` reads two things from the tree, so the tool can also check older releases:

- whether `Si5351C::SetPLL` has the `exactFrequency` argument; if it doesn't, it builds
  with `-DFW_V164`
- the 2nd-LO retune tolerance in `needs2LOshift()`

Needs g++ (C++17) and bash. Tested with g++ 15 on Ubuntu.

| Tree | Worst offset (×IFBW) | `run_checks.sh` at 0.5 |
|---|---|---|
| v1.6.4 | 47 (5 Hz IFBW), 24 (10 Hz) | 11 of 13 sweeps fail |
| v1.6.5 / master 13f37c5 | 0.87 (5 Hz), 0.76 (10 Hz) | 6 fail |
| this branch (rounded fast divider, IFBW/4 tolerance) | 0.38 | pass |

## Limits

- `VNA.cpp` isn't compiled; it depends on the FPGA, USB and the RTOS. The parts that
  choose frequencies (`setPLLFrequencies`, `needs2LOshift`, the halt logic and the
  Si5351 writes in `SweepHalted`) are copied into `fwplan.cpp`. Any change there has to
  be mirrored by hand.
- Not modelled:
  - harmonic mode above 6 GHz
  - the external 10 MHz reference
  - the TCXO frequency correction
  - the spectrum analyser and the generator
