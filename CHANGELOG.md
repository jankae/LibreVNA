# Changelog

## v1.2.2

Mostly bugfixes along with the occasional new feature.

- Additional graph features:
  - Group delay option
  - (Optionally) show units on all axes
  
- General bugfixes, among others:
  - Fixed valgrind issues (mostly uninitialized variables)
  - Prevent overflow in SCPI commands

## v1.2.1

Mostly bugfixes along with the occasional new feature.

- Calibration:
  - File format changed to json
  - Multiple measurements can be taken/deleted at the same time
  - Calibration kit allows separate male/female standards
  - configurable Z0 for short/open
- SCPI commands:
  - load/save calibration files
  - export to touchstone file format directly
  - additional command for reading trace data
  - fix typo in documentation
- UI improvements:
  - Additional Y-axis options: Reactance/Real/Imaginary
  - Graphs look a bit nicer
  - Configurable line width for graphs
  - finally added an application logo
- General bugfixes, among others:
  - PLL divider calculation fixed for certain frequencies
  - Improved USB buffer handling
  - Better error handling when opening invalid files
  - Various bugs when adding/deleting markers
  - graph autoscaling with invisible traces

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
