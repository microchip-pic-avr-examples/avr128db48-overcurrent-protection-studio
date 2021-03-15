<!-- Please do not change this html logo with link -->
<a href="https://www.microchip.com" rel="nofollow"><img src="images/microchip.png" alt="MCHP" width="300"/></a>

# AVR128DB48 Current Spike Detection
A new feature introduced in the AVR® DB family is the Analog Signal Conditioning (OPAMP) peripheral. In this example, the OPAMP is used together with an Analog Comparator (AC) to form a current spike detector. The fan is enabled by a TCD waveform output. This output will be shut off automatically when the current spike is detected by the detector, making it an ideal solution for applications where functional safety is needed. An example of an application where a current spike might indicate failure or danger, is a computer fan. Under normal operation, the current consumption is nonlinear, but does not exceed a certain threshold, dependent on the fans spec. By autotuning the gain of the internal OPAMP by using the internal resistor ladders, we can automatically boost the signal to a measurable range, and automatically set a trigger level for the AC by use of the on board DAC to define what an "unnaturally" high current consumtion is, indicating a touch or something stuck in the fan.

## Related Documentation

* [AN3860 - Overcurrent Protection Using the Integrated Op Amps on AVR® DB](https://microchip.com/DS00003860)
* [AVR128DB48 device page](https://www.microchip.com/wwwproducts/en/AVR128DB48)
* [AVR128DB48 Curiosity Nano User Guide](https://www.microchip.com/DS50003037)

## Software Used

* [Microchip Studio](https://www.microchip.com/mplab/microchip-studio) 7.0.2542 or later
* [Atmel AVR-Dx_DFP version 1.6.76](http://packs.download.atmel.com/#collapse-Atmel-AVR-Dx-DFP-pdsc) or later
* For the MCC MPLAB® X version of this project, please go to [this repository](https://github.com/microchip-pic-avr-examples/avr128db48-overcurrent-protection-mplab-mcc.git)
* For the Atmel START version of this project, please go to [this repository](https://github.com/microchip-pic-avr-examples/avr128db48-overcurrent-protection-studio-start.git)

## Hardware Used

* [AVR128DB48 Curiosity Nano](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/EV35L43A)
* A small computer fan (M1)
* 3x 1 kOhm resistor (R2, R3, R5)
* 1x 10 kOhm resistor (R6)
* 1x 0.1 Ohm current sense resistor (R1)
* 2x 100nF capasitors (C1, C2)
* 1x N-Channel MOSFET transistor (Q1)
* 1x Signal Diode (D1)


## Setup
* Hook up the circuit as shown in the figure below
* For minimal noise, soldering the circuit onto a perfboard/stripboard is prefered, although a breadboard could be used with worse performance
<p><img src="images/circuit_drawing.png" alt="Circuit Drawing" width="400"/></p>

## Operation
* Connect the AVR128DB48 Curiosity Nano to a computer using a USB cable
* Download the zip file or clone the example to get the source code
* Open the .atsln with Atmel Studio
* Press *Start without debugging* or use CTRL+ALT+F5 hotkeys to run the application
* After the LED stops flashing, it is done calibrating
* Gently apply preassure to the fan, being very careful not to hurt yourself. The fan should now stop.
* Click the onboard button (SW1) to re-enable the fan


## Conclusion
After going through this example, you should have a better understanding of how to combine the OPAMP and AC peripherals to make a current spike detector.
