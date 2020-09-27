# FAQ

### Can I buy one?
No, not at the moment. So far this is just a private hobby project.

### Can I build one?
Absolutely. All required files are available and if you are not afraid to solder a lot of small components you can certainly build your own. The total BOM cost (without the aluminium shielding) is about 200€ when only buying parts for one PCB.

There are some [basic instructions](DeveloperInfo/BuildAndFlash.md) to help get started on the software side.

### Why is the final IF so high?
I am using 250kHz as the final IF frequency. Compared to some other VNA projects this is rather high and requires a fast (more expensive) ADC. However, it also makes the sweep rate faster: each measurement point takes at least one IF period (more if noise should be reduced to practical levels).

### Why the FPGA?
Why not sample the ADCs directly with (a more powerful) STM32? Or even use the ADC on the µC itself?
* I wanted more than the 12bits provided with by internal STM32 ADCs (and also better SNR)
* Sampling the three 16bit 1MSamples/s ADCs with an STM might be possible but certainly would be a challenge
* More flexibility with the FPGA controlling all critical RF components (e.g. setting up all PLLs concurrently for each measurement point, further increasing sweep rate)
* Enough power for future features (e.g. spectrum analyzer mode with FFT)

### If there has to be an FPGA, why one with a closed-source toolchain?
* Sorry, thats just what I had experience with (also, the toolchain is free, just not open-source)
