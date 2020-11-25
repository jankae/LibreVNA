# Build instructions
This project consists of two software components: the PC application (written in C++/Qt) and the embedded VNA firmware (C/C++). The VNA firmware is further split into the MCU firmware and the FPGA configuration bitstream.
Creating all the different parts from the source code requires quite a few tools:
* For building the PC application:
  * [Qt Creator](https://www.qt.io/offline-installers) (at least with Qt 5.9)
  * [libusb-1.0](https://libusb.info/)
* For building the VNA firmware:
  * [CubeMX:](https://www.st.com/en/development-tools/stm32cubemx.html) Code generator for initializing STM32 microcontrollers (only required when changing device initialization settings in CubeMX)
  * [SW4STM32:](https://www.st.com/en/development-tools/sw4stm32.html) Eclipse based IDE, compatible with CubeMX (other IDEs could also be used but this is supported out of the box with CubeMX)
  * [OpenOCD](http://openocd.org/) or other tool to flash firmware into the microcontroller
* For creating the FPGA bitstream:
  * [Xilinx ISE](https://www.xilinx.com/products/design-tools/ise-design-suite.html) with support for Spartan 6 FPGA
 
## Building the PC application:
* Install the required tools (Qt and libusb-1.0):
  ```
  sudo apt-get install qt5-default qt5-qmake qtbase5-dev libusb-1.0-0-dev 
  ```
* Build the application:
  * either:
  ```
  cd Software/PC_Application
  qmake
  make
  ```
  * or:
    * Open Software/PC_Application/Application.pro with Qt Creator
    * Build the application with Qt Creator

## Building the MCU firmware:
* Generating initialization code (only required when changes to CubeMX have been made):
  * Open Software/VNA_embedded/VNA_embedded.ioc with CubeMX
  * Click "Generate Code"
* Importing the project (only has to be done once):
  * Open SW4STM32 with the workspace set to Software/
  * Import the project: File > Import... > General > Exisiting Projects into Workspace > Select root directory (should already be set to Software/) > Select the "VNA_embedded" project > Finish
* Build project: Project > Build Project
* Flashing/Debugging the MCU (only required once for a new hardware or when debugging):
  * Create build configuration if it doesn't exist alreay: Run > Run configurations > Create new "Ac6 STM32 Debugging" > In tab "Main" select the compiled .elf-file
  * Connect to the MCU with an ST-Link and the SWD pads on the PCB (requires removing the aluminium enclosure)
  * Flash/debug: Run > Run or Run > Debug

## Creating the FPGA bitstream:
* Open FPGA/VNA/VNA.xise in Xilinx ISE
* In the Design tab, select the implementation view and then "top - Behavioral"
* In the Processes tab, doubleclick "Generate Programming File". This will take a couple of minutes.

## Assembling the combined VNA firmware:
Only on a new hardware with an empty microcontroller it is required to flash the MCU firmware once. After that updates are possible through the USB port. Updating the firmware requires a combined fimrware image that contains both the MCU firmware as well as the FPGA bitstream and some header data.
* Make sure you compiled the MCU firmware: there should be a binary file "VNA_embedded.bin" in Software/VNA_embedded/[Debug/Release]
* Make sure you have the FPGA bitstream: there should be a file "top.bin" in FPGA/VNA
* Execute the script "AssembleFirmware.py" in the root directory. This creates a combined firmware file "combined.vnafw"

## Getting a new board running:
1. Flash the compiled MCU firmware by directly connecting to the SWD pads on the board with an ST-Link and using OpenOCD/ST-Link utility/...
2. The "Booting" LED should indicate an error code 2 (invalid FLASH content). This does not refer to MCU flash but rather the onboard flash chip which is empty at this point.
3. Perform a firmware update using the PC application and the combined firmware:
    1. Open the compiled PC application
    2. Connect to the device if not already connected: Device > Update Device List and then Device > Connect to > Select your serialnumber
    3. Perform firmware update: Device > Firmware Update, select the "combined.vnafw" and click Start
4. The board should reboot and not report an error condition anymore (only "Ready" and "Power" LED on)
