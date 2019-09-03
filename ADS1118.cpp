/**
*  Arduino Library for Texas Instruments ADS1118 - 16-Bit Analog-to-Digital Converter with 
*  Internal Reference and Temperature Sensor
*  
*  @author Alvaro Salazar <alvaro@denkitronik.com>
*  http://www.denkitronik.com
*
*/


/**
 * The MIT License
 *
 * Copyright 2018 Alvaro Salazar <alvaro@denkitronik.com>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#include "ADS1118.h"
#include "Arduino.h"

/**
* Debugging:
*   Uncomment some of the 3 lines above to debug the method you want. 
* Warning: 
*   Keep them commented in production
*/
//#define DEBUG_BEGIN  			//Debug begin() method
//#define DEBUG_GETADCVALUE  	//Debug getADCValue() method
//#define DEBUG_GETTEMPERATURE  //Debug getTemperature() method

#ifdef DEBUG_BEGIN
    #define DEBUG_BEGIN(x) decodeConfigRegister(x)
#else
    #define DEBUG_BEGIN(x)
#endif

#ifdef DEBUG_GETADCVALUE
    #define DEBUG_GETADCVALUE(x) decodeConfigRegister(x)
#else
    #define DEBUG_GETADCVALUE(x)
#endif

#ifdef DEBUG_GETTEMPERATURE
    #define DEBUG_GETTEMPERATURE(x) decodeConfigRegister(x)
#else
    #define DEBUG_GETTEMPERATURE(x)
#endif



#if defined(__AVR__)
/**
 * Constructor of the class
 * @param io_pin_cs a byte indicating the pin to be use as the chip select pin (CS)
 */
ADS1118::ADS1118(uint8_t io_pin_cs) {
    cs = io_pin_cs;
}						///< This method initialize the SPI port and the config register        
#elif defined(ESP32)
/**
 * Constructor of the class
 * @param io_pin_cs a byte indicating the pin to be use as the chip select pin (CS)
 */
ADS1118::ADS1118(uint8_t io_pin_cs, SPIClass *spi) {
    cs = io_pin_cs;
    pSpi = spi;
}       
#endif


#if defined(__AVR__)
/**
 * This method initialize the SPI port and the config register
 */
void ADS1118::begin() {
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
    SPI.begin();
    SPI.beginTransaction(SPISettings(SCLK, MSBFIRST, SPI_MODE1));
    configRegister.bits={RESERVED, VALID_CFG, DOUT_PULLUP, ADC_MODE, RATE_8SPS, SINGLE_SHOT, FSR_0256, DIFF_0_1, START_NOW}; //Default values    
    DEBUG_BEGIN(configRegister); //Debug this method: print the config register in the Serial port
}						///< This method initialize the SPI port and the config register        
#elif defined(ESP32)
/**
 * This method initialize the SPI port and the config register
 */
void ADS1118::begin() {
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
    pSpi->begin();
    configRegister.bits={RESERVED, VALID_CFG, DOUT_PULLUP, ADC_MODE, RATE_8SPS, SINGLE_SHOT, FSR_0256, DIFF_0_1, START_NOW}; //Default values
    DEBUG_BEGIN(configRegister); //Debug this method: print the config register in the Serial port
}      

void ADS1118::begin(uint8_t sclk, uint8_t miso, uint8_t mosi) {
    pinMode(cs, OUTPUT);
	digitalWrite(cs, HIGH);
    pSpi->begin(sclk, miso, mosi, cs);
	configRegister.bits={RESERVED, VALID_CFG, DOUT_PULLUP, ADC_MODE, RATE_8SPS, SINGLE_SHOT, FSR_0256, DIFF_0_1, START_NOW}; //Default values
    DEBUG_BEGIN(configRegister); //Debug this method: print the config register in the Serial port
}


/**
 * Getting a sample from the specified input if data is ready
 * @param pin_drdy io pin connected to ADS1118 DOUT/DRDY. value Reference of ADC value to be fetched
 * @return True if ADC data is ready
 */
bool ADS1118::getADCValueNoWait(uint8_t pin_drdy, uint16_t &value) {
    byte dataMSB, dataLSB;	
	pSpi->beginTransaction(SPISettings(SCLK, MSBFIRST, SPI_MODE1));
	digitalWrite(cs, LOW);
	if (digitalRead(pin_drdy)) {
		digitalWrite(cs, HIGH);
		pSpi->endTransaction();
		return false;
	}

	dataMSB = pSpi->transfer(configRegister.byte.msb);
	dataLSB = pSpi->transfer(configRegister.byte.lsb);
	digitalWrite(cs, HIGH);
	pSpi->endTransaction();

	value = (dataMSB << 8) | (dataLSB);
    return true;
}

/**
 * Getting the millivolts from the settled inputs
 * @return A double (32bits) containing the ADC value in millivolts
 */
bool ADS1118::getMilliVoltsNoWait(uint8_t pin_drdy, double &volts) {
    float fsr = pgaFSR[configRegister.bits.pga];
	uint16_t value;
	bool dataReady=getADCValueNoWait(pin_drdy, value);
	if (!dataReady) return false;
	if(value>=0x8000){
		value=((~value)+1); //Applying binary twos complement format
		volts=((float)(value*fsr/32768)*-1);
	} else {
		volts=(float)(value*fsr/32768);
	}
    volts = volts*1000;
	return true;
}
#endif



/**
 * Getting a sample from the specified input
 * @param inputs Sets the input of the ADC: Diferential inputs: DIFF_0_1, DIFF_0_3, DIFF_1_3, DIFF_2_3. Single ended input: AIN_0, AIN_1, AIN_2, AIN_3
 * @return A word containing the ADC value
 */
uint16_t ADS1118::getADCValue(uint8_t inputs) {
    uint16_t value;
    byte dataMSB, dataLSB, configMSB, configLSB, count=0;
	if(lastSensorMode==ADC_MODE)  //Lucky you! We don't have to read twice the sensor
		count=1;
	else
	configRegister.bits.sensorMode=ADC_MODE; //Sorry but we will have to read twice the sensor
	configRegister.bits.mux=inputs;
    do{
#if defined(ESP32)
	pSpi->beginTransaction(SPISettings(SCLK, MSBFIRST, SPI_MODE1));
#endif        
	digitalWrite(cs, LOW);
#if defined(__AVR__)	
        dataMSB = SPI.transfer(configRegister.byte.msb);
        dataLSB = SPI.transfer(configRegister.byte.lsb);
        configMSB = SPI.transfer(configRegister.byte.msb);
        configLSB = SPI.transfer(configRegister.byte.lsb);        
#elif defined(ESP32)
        dataMSB = pSpi->transfer(configRegister.byte.msb);
	dataLSB = pSpi->transfer(configRegister.byte.lsb);
	configMSB = pSpi->transfer(configRegister.byte.msb);
	configLSB = pSpi->transfer(configRegister.byte.lsb);        
#endif 

	
        digitalWrite(cs, HIGH);
#if defined(ESP32)        
	pSpi->endTransaction();
#endif         
	for(int i=0;i<CONV_TIME[configRegister.bits.rate];i++) //Lets wait the conversion time
            delayMicroseconds(1000);
            count++;
	}while (count<=1);  //We make two readings because the second reading is the ADC conversion.	
        DEBUG_GETADCVALUE(configRegister);  //Debug this method: print the config register in the Serial port
	value = (dataMSB << 8) | (dataLSB);
    return value;
}


/**
 * Getting the millivolts from the specified inputs
 * @param inputs Sets the inputs to be adquired. Diferential inputs: DIFF_0_1, DIFF_0_3, DIFF_1_3, DIFF_2_3. Single ended input: AIN_0, AIN_1, AIN_2, AIN_3
 * @return A double (32bits) containing the ADC value in millivolts
 */
double ADS1118::getMilliVolts(uint8_t inputs) {
    float volts;
    float fsr = pgaFSR[configRegister.bits.pga];
    uint16_t value;
    value=getADCValue(inputs);
    if(value>=0x8000){
	value=((~value)+1); //Applying binary twos complement format
	volts=((float)(value*fsr/32768)*-1);
    } else {
	volts=(float)(value*fsr/32768);
    }
    return volts*1000;
}


/**
 * Getting the millivolts from the settled inputs
 * @return A double (32bits) containing the ADC value in millivolts
 */
double ADS1118::getMilliVolts() {
    float volts;
    float fsr = pgaFSR[configRegister.bits.pga];
    uint16_t value;
    value=getADCValue(configRegister.bits.mux);
    if(value>=0x8000){
	value=((~value)+1); //Applying binary twos complement format
	volts=((float)(value*fsr/32768)*-1);
    } else {
	volts=(float)(value*fsr/32768);
    }
    return volts*1000;
}



/**
 * Getting the temperature in degrees celsius from the internal sensor of the ADS1118
 * @return A double (32bits) containing the temperature in degrees celsius of the internal sensor
 */
double ADS1118::getTemperature() {
    uint16_t convRegister;
    uint8_t dataMSB, dataLSB, configMSB, configLSB, count=0;
    if(lastSensorMode==TEMP_MODE)
        count=1;  //Lucky you! We don't have to read twice the sensor
    else
	configRegister.bits.sensorMode=TEMP_MODE; //Sorry but we will have to read twice the sensor
    do{
#if defined(ESP32)
	pSpi->beginTransaction(SPISettings(SCLK, MSBFIRST, SPI_MODE1));
#endif
	digitalWrite(cs, LOW);
        
#if defined(__AVR__)	
        dataMSB = SPI.transfer(configRegister.byte.msb);
        dataLSB = SPI.transfer(configRegister.byte.lsb);
        configMSB = SPI.transfer(configRegister.byte.msb);
        configLSB = SPI.transfer(configRegister.byte.lsb);        
#elif defined(ESP32)
        dataMSB = pSpi->transfer(configRegister.byte.msb);
	dataLSB = pSpi->transfer(configRegister.byte.lsb);
	configMSB = pSpi->transfer(configRegister.byte.msb);
	configLSB = pSpi->transfer(configRegister.byte.lsb);        
#endif
	digitalWrite(cs, HIGH);
#if defined(ESP32)
	pSpi->endTransaction();
#endif
	for(int i=0;i<CONV_TIME[configRegister.bits.rate];i++) //Lets wait the conversion time
            delayMicroseconds(1000);
        count++;
    }while (count<=1);  //We make two readings because the second reading is the temperature.
    DEBUG_GETTEMPERATURE(configRegister);  //Debug this method: print the config register in the Serial port
    convRegister = ((dataMSB << 8) | (dataLSB))>>2;
    if((convRegister<<2) >= 0x8000){
        convRegister=((~convRegister)>>2)+1; //Converting to right-justified and applying binary twos complement format
        return (double)(convRegister*0.03125*-1);
    }
    return (double)convRegister*0.03125;
}

/**
 * Setting the sampling rate specified in the config register
 * @param samplingRate It's the sampling rate: RATE_8SPS, RATE_16SPS, RATE_32SPS, RATE_64SPS, RATE_128SPS, RATE_250SPS, RATE_475SPS, RATE_860SPS
 */
void ADS1118::setSamplingRate(uint8_t samplingRate){
    configRegister.bits.rate=samplingRate;
}

/**
 * Setting the full scale range in the config register
 * @param fsr The full scale range: FSR_6144 (±6.144V)*, FSR_4096(±4.096V)*, FSR_2048(±2.048V), FSR_1024(±1.024V), FSR_0512(±0.512V), FSR_0256(±0.256V). (*) No more than VDD + 0.3 V must be applied to this device.
 */
void ADS1118::setFullScaleRange(uint8_t fsr){
    configRegister.bits.pga=fsr;
}

/**
 * Setting the inputs to be adquired in the config register. 
 * @param input The input selected: Diferential inputs: DIFF_0_1, DIFF_0_3, DIFF_1_3, DIFF_2_3. Single ended input: AIN_0, AIN_1, AIN_2, AIN_3
 */
void ADS1118::setInputSelected(uint8_t input){
    configRegister.bits.mux=input;
}

/**
 * Setting to continuous adquisition mode
 */
void ADS1118::setContinuousMode(){
    configRegister.bits.operatingMode=CONTINUOUS;
}

/**
 * Setting to single shot adquisition and power down mode
 */
void ADS1118::setSingleShotMode(){
    configRegister.bits.operatingMode=SINGLE_SHOT;
}

/**
 * Disabling the internal pull-up resistor of the DOUT pin
 */
void ADS1118::disablePullup(){
    configRegister.bits.operatingMode=DOUT_NO_PULLUP;
}

/**
 * Enabling the internal pull-up resistor of the DOUT pin
 */
void ADS1118::enablePullup(){
    configRegister.bits.operatingMode=DOUT_PULLUP;
}

/**
 * Decoding a configRegister structure and then print it out to the Serial port
 * @param configRegister The config register in "union Config" format
 */
void ADS1118::decodeConfigRegister(union Config configRegister){
    String mensaje=String();
    switch(configRegister.bits.singleStart){
	case 0: mensaje="NOINI"; break;
	case 1: mensaje="START"; break;
    }
    mensaje+=" ";
    switch(configRegister.bits.mux){
	case 0: mensaje+="A0-A1"; break;
        case 1: mensaje+="A0-A3"; break;
	case 2: mensaje+="A1-A3"; break;
	case 3: mensaje+="A2-A3"; break;
	case 4: mensaje+="A0-GD"; break;
	case 5: mensaje+="A1-GD"; break;
	case 6: mensaje+="A2-GD"; break;
	case 7: mensaje+="A3-GD"; break;
    }
    mensaje+=" ";
    switch(configRegister.bits.pga){
	case 0: mensaje+="6.144"; break;
        case 1: mensaje+="4.096"; break;
	case 2: mensaje+="2.048"; break;
        case 3: mensaje+="1.024"; break;
	case 4: mensaje+="0.512"; break;
	case 5: mensaje+="0.256"; break;
	case 6: mensaje+="0.256"; break;
	case 7: mensaje+="0.256"; break;
    }
    mensaje+=" ";		
    switch(configRegister.bits.operatingMode){
	case 0: mensaje+="CONT."; break;
        case 1: mensaje+="SSHOT"; break;
    }
    mensaje+=" ";		
    switch(configRegister.bits.rate){
	case 0: mensaje+="8 SPS"; break;
        case 1: mensaje+="16SPS"; break;
	case 2: mensaje+="32SPS"; break;
	case 3: mensaje+="64SPS"; break;
	case 4: mensaje+="128SP"; break;
	case 5: mensaje+="250SP"; break;
	case 6: mensaje+="475SP"; break;
	case 7: mensaje+="860SP"; break;
    }
    mensaje+=" ";		
    switch(configRegister.bits.sensorMode){
	case 0: mensaje+="ADC_M"; break;
        case 1: mensaje+="TMP_M"; break;
    }
    mensaje+=" ";		
    switch(configRegister.bits.pullUp){
        case 0: mensaje+="DISAB"; break;
        case 1: mensaje+="ENABL"; break;
    }
    mensaje+=" ";		
    switch(configRegister.bits.noOperation){
        case 0: mensaje+="INVAL"; break;
        case 1: mensaje+="VALID"; break;
        case 2: mensaje+="INVAL"; break;
        case 3: mensaje+="INVAL"; break;
    }
    mensaje+=" ";		
    switch(configRegister.bits.reserved){
        case 0: mensaje+="RSRV0"; break;
        case 1: mensaje+="RSRV1"; break;
    }	
    Serial.println("\nSTART MXSEL PGASL MODES RATES ADTMP PLLUP NOOPE RESER");
    Serial.println(mensaje);
}
