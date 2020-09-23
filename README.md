# 1MHz - 6GHz VNA2
**!work in progress, more information will be added in the future!**

This is the improved version of my [first attempt](https://www.github.com/jankae/VNA) at a VNA.

Main differences to first version:
* Exchanged some RF chips for slightly cheaper versions with similar specifications
* Power supply scheme changed to use 5V instead of 12V, potentially allowing the device to be powered from USB
* FPGA gets configured from the microcontroller, removing the need for a JTAG programmer. This also allows firmware updates of both the FPGA and the microcontroller through the USB port
* RF sections distributed differently on PCB to increase isolation between ports
