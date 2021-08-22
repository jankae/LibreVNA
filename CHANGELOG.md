# Changelog

## v1.2.0

- Additional SCPI commands
- Marker improvements:
  - create groups to move markers together
  - calculate additional parameters (R/L/C,...)
- Fullscreen mode of single graph by double-clicking
- Error flags for ADC overload and output amplitude unlevel in VNA mode
- New feature: power sweeps (sweep the stimulus power instead of the frequency)
- General bugfixes

Significant changes have been made to the embedded firmware as well as the FPGA configuration to support power sweeps. A firmware update on the LibreVNA is mandatory to use this new software version.

## v1.1.2

- Software:
  - Fix parsing of calibration kit settings

No changes have been made to the embedded software, updating the firmware on the LibreVNA is not required.
