# Arduino Library for Texas Instruments ADS1118 - 16-Bit Analog-to-Digital Converter with internal Reference and Temperature Sensor

This is an easy to use Arduino library for ADC Converter ADS1118

## Getting Started

Learn with the examples provided: For basic use "basicExampleAds1118" and "ads1118example" for a detailed use.

### Prerequisites

None


### Installing

Download this library as zip file, then click in "Sketch" -> "Include Library" ->"Add .ZIP Library" and select the zip file downloaded.

## Running the tests
1. Connect the ADS1118 in the 3.3v power supply pins of your Arduino.
2. Connect the ADS1118 CS pin to the pin 5 of your Arduino.
3. Connect the ADS1118 MISO (DOUT), MOSI(DIN) and SCLK pins to the MISO, MOSI and SCLK of your Arduino (search on internet "Arduino ICSP pinout").

|ARDUINO (ICSP PINS) |ADS1118 PINS |
| --- | --- |
| MOSI | DIN |
| MISO | DOUT |	
| SCLK | SCLK |
| 5 (change this pin to fit your needs) |CS |

![alt text](https://github.com/denkitronik/ADS1118/blob/master/conexionAds1118.png)
4. Run the examples provided in your Arduino IDE. 
Go to "File" -> "Examples" -> "ADS1118 library" -> "basicExampleAds1118" or "ads1118example" 

## Example with a thermocouple
This is the typical circuit for a "thermocouple measurement system" as shown in the TI [ADS1118 datasheet page 32](http://www.ti.com/lit/ds/symlink/ads1118.pdf). 
![alt text](https://github.com/denkitronik/ADS1118/blob/master/thermocouple.png)


## Built With

* [Arduino IDE](https://www.arduino.cc/)

## Contributing

Everyone is invited to contribute
When contributing to this repository, please first discuss the change you wish to make via issue, email, or any other method with me before making a change.

## Release History
    1.0.0
        CHANGE: Initial commit

## Versioning
I am using [SemVer](http://semver.org/) for versioning. 

## Authors

* **Alvaro Salazar** - *Initial work* - [alvaro-salazar](https://github.com/alvaro-salazar)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE.md) file for details.
