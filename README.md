# Introduction
The PiPicoFX is a simple multieffects unit for Guitar built around a Raspberry Pi Pico board. It is capable of serving as a standalone Amp-Simulator in conjuctions with a general-purpose PA system. 
## Audio Engine Specs
* 48 kHz sampling rate
* 0.6 ms Latency
* 16 bit fixed point accuracy
* Mono Input and output
## Controls
* 3 Parameters directly accessible via Knobs
* 128*64 Pixel monochrome display
* 1 Encoder, 2 Pushbuttons
* Analog Volume Control 
# Building
## Material needed
* Raspberry Pi Pico Board
* Audio Codec Board
* 128*64 Pixel OLED Display, compatible with SSD1306 driver
* Rotary Encoder (full quadrature)
* 5 10k Potentiometer
* 2 Pushbuttons
* 2 6.3 Mono Jacks 
* Enclosure
* Power Connector
## Audio-Codec Board
Generally an audio codec able to operate in 48 kHz I2S Master Mode is able to work with the raspberry pi pico. In fact at the beginning of the development even the internal ADC of the Pi Pico served as an input. The most straightforward and probably best-sounding way is to build the audio-codec pcb's using the [http://gerberfiles.zip] from your favorite small-batch manufacturer and then using the [bom.csv] to order the parts from your preferred electronics distributor. Of course some SMD-soldering is required as well... 
## Software Installation
Download [pipicofx.uf2]. Then connect the Raspberry Pi Pico while having the "boot" button depressed. Drag the file downloaded to the mass-storage device showing up, then you are done.

 ## Assembly
 Mount the UI Elements (Buttons, Encoder, Potentiometers, Jacks, Power connector) to the Enclosure the wire up the entire system using the sketch shown below

