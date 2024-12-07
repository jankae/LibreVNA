# Changelog

## v1.6.1

Mostly bugfixes with only minor new features

- New features:
  - New SCPI commands:
    - switch between linear/log sweeps in VNA mode
    - delete traces
  - Option to align axis ticks for left and right Y-axes
  - Import/export options in file menu
  - User-selectable handling of NaN values for limit checking
  - drop files on trace list to import them

- Bugfixes:
  - center TDR output values at zero (fixes impedance calculations in cases where there is a reflection from before the calibration plane)
  - fix calibration calculation when non-required S parameters are not available
  - improve LibreCAL coefficient extraction
  - use native OS dialogs for opening/saving files
  - future Qt version fix: prevent crash after taking calibration measurement
  - SCPI command handling improved
  - fix menus after loading setup files in certain scenarios

## v1.6.0

This update contains API breaking changes to the SCPI server. This was necessary because the previous implementation did not follow the standard completely. If you were talking to the SCPI server directly yourself, you will need to make the required changes. If you are using the provided libreVNA.py class, you just need to update that file.

- New featues:
  - New SCPI command: read/write preferences
  - New API: option to stream live trace data

- Bugfixes and improvements:
  - Keep de-embedding active when changing settings
  - Improvements to trace averaging
  - adjust SCPI API to conform better with the standard:
    - new required commands RST, CLS, ESE, ESR, OPC and WAI
    - no responses to commands, errors are reported through status registers instead
  - TDR updates happen during a sweep not just afterwards (with configurable update rate)
  - Additional sanity checks when loading calibration files

## v1.5.1

Mostly bugfixes with only minor new features

- New features:
  - Z0 of through standard now adjustable
  - Option to automatically adjust the span to the active calibration
- Bugfixes:
  - Make De-embedding menu visible on MacOS
  - Detection of handling of LibreCAL in the automatic calibration dialog
  - Allow updates of devices with older firmwares (a matching older GUI version had to be used previously)
  - Fix port excitation when not all S-parameters are measured
  - Do not use DFT in spectrum analyzer when in zero span mode
  - Readback of certain GUI values on Windows now correct (was stuck on 0 before)
  - Prevent crash when disconnecting the device while a dialog was open
  - Prevent occasional crash at the end of calibration measurements
  - Fix spikes in sweep when a frequency calibration is set
  - Improve sample timing between ADC and FPGA (resulted in very noisy traces on some devices)

## v1.5.0

- New features:
  - Further abstraction from the LibreVNA hardware. The GUI now supports VNAs with up to 8 ports
  - Capsulation of communication to the hardware inside of the DeviceDriver class. This simplifies the integration of drivers for other equipment
    - Experimental drivers for Siglent SSA3000X and SNA5000A series devices as examples
  - Allow cascading of muliple LibreVNAs, see Preferences->Device Drivers->LibreVNA/Compound. You need to configure a compound device first (consisting of up to 4 LibreVNAs). Afterwards, you are able to use them as one larger, virtual VNA with more ports (or more physical separation between the ports). Measurements within one physical LibreVNA will contain phase information, measurements between different LibreVNAs have their phase set to zero
  - Support for calibrating with sliding loads
  - New graph type: Eye Diagram
  - Restrict markers to a certain frequency range
  - Switched to a dedicated USB VID/PID thanks to pid.codes
  - Switched to Qt6
  - Easier 

- API-breaking changes: Unfortunately, some major rework was required to make the GUI compatible to devices with more than two ports. In some areas, the file storage format and the SCPI API had to change. Almost all changes are within the calibration system.
  - Calibration kit file format changed (old calibration kits can be imported)
  - Calibration file format changed (old calibration can be imported)
  - Calibration API changed significantly, see Programming Guide
  
- Lots of bugfixes, for example:
  - Fix X-label clipping on graphs
  - Prevent activation of a calibration without defined calibration standards
  - Fix isolation measurements for calibration
  - Valgrind issues fixed (mostly variable initializations)
  - made USB communication more robust
  - PLL locking issue fixed
  - no more disappearing device log

## v1.4.1
- Bugfix: Configure voltage regulator for correct voltage at startup

## v1.4.0

- New features:
  - Configurable constant lines for XY-plots
  - Limit testing on XY-plots
  - Single sweep functionality
  - Zero span support for VNA and SA
  - Math operations across traces
  
- Bugfixes:
  - Stop duplicate GUI elements from appearing when loading a setup file
  - Prevent wrong frequency output when sweep starts at DC
  - Improve automatic source/receiver calibration
  - Fix signal ID issue which caused valid signals to disappear at low RBWs
  - Changing trace color triggers replot of graphs
  - Remove output spikes in generator mode, caused by continuously reconfiguring the PLL
  - Fix MCU temperature calculation

- Small improvements:
  - Split plots without closing the plot first
  - Configurable marker sort order
  - Refactoring

## v1.3.0

Some new software features along with a few bug fixes.

- New features:
  - Allow multiple VNA/Signal generator/Spectrum analyzer tabs
  - Software work-around to allow up to 65535 points per sweep
  - Impedance renormalizetion (as de-embedding option)
  - Waterfall plot
  - CSV export options extended
  - logarithmic sweeps (logarithmic frequency steps)
  - Graph display improvements:
    - (Optionally) show units on all axes
    - Configurable font size
	- New Y axis options: dBuV, linear magnitude, unwrapped phase, group delay
	- Zoom feature for smith chart
  - User-selectable IF frequencies
  
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
