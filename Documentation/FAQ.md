# FAQ

### Can I buy one?
No, not at the moment. However, thanks to Hugen, some initial engineering samples have been distributed and if testing goes well, this project might be for sale at some point.

### Can I build one?
Absolutely. All required files are available and if you are not afraid to solder a lot of small components you can certainly build your own. The total BOM cost (without the aluminium shielding) is about 200€ when only buying parts for one PCB.

There are some [basic instructions](DeveloperInfo/BuildAndFlash.md) to help get started on the software side.

### Why the dual stage conversion?
There are some advantages to having the second stage mixers (mostly to compensate for limited fractional dividers in the MAX2871) but for most operations, a single stage conversion would probably work almost as well. However, compared to other parts the cost for the second stage conversion is not that high and without it certain functionality would not be available (e.g. spectrum analyzer mode with low RBW). There is a bit longer explanation [here](https://github.com/jankae/VNA2/issues/2#issuecomment-700218226).

### Why are there no aliasing filters in front of the ADC?
The actually are RC lowpass filters in front of the ADC but they are not very sharp and even for signals way above the nyquist frequency they only provide a small amount of attenuation. With my combination of final IF and ADC samplerate, proper alias filters are difficult to implement. Thankfully, they are not absolutely required in a VNA (same reason why image reject filters reject filters after the mixers are not required: the device already knows which frequency to look for, because it generates the stimulus signal itself).

However, in spectrum analyzer mode, all these missing filters are a problem and lead to a lot of images that have to be removed by signal identification. 

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
