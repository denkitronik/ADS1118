# ADS1118 Arduino Library
The offical HydroSense library for the TI ADS1118 16-Bit ADC chip.

Created By: Colby Rome (cdr013@bucknell.edu)
Last Updated By: Ward Prescott (erp006@bucknell.edu) on Mar 9, 2016 12:08 AM

## ADS1118 Class
### Constructor
`Ads1118(int CS_pin)`
### Methods
- `.begin(void)`
- `adsRead(int port)`returns the floating point representation of the voltage at the port.  Macros for the different ports are defined in the header file and are:
	- DIF01
	- DIF23
	- AIN0
	- AIN1
	- AIN2
	- AIN3
- `readTemp(void)` is being debugged and may or may not work.
- `setGain(int GainSet)` 
- `selfTest(void)` the method returns1 if there is an error communicating with the device.  This may be due to a wiring problem or other SPI error.

### Parameters
There no public parameters of interest.

## Usage
Clone repository into your Arduino libraries folder (typically ~/Arduino/libraries/)

See the examples folder.

## Notes
None yet.

## Further Work
1. Temperature readings
2. Low power mode (current it is set to constantly sample)
