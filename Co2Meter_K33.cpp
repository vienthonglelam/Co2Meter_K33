// CO2 Meter K‐series Example Interface
// by Andrew Robinson, CO2 Meter <co2meter.com>
// Talks via I2C to K33‐ELG/BLG Sensors for Host‐Initiated Data Collection 
// 4.1.2011

//Modified into Arduio Library by Kina Smith <kinasmith.com>
//06.09.2016

#include <inttypes.h>
#include <Wire.h>
#include "Arduino.h"
#include "Co2Meter_K33.h"

//int devAddr = 0x68; // This is the default address of the CO2 sensor, 7bits shifted left. 
// int devAddr = 0x7F;  // This is the broadcast address. There is conflicting addresses with the DS1307 RTC

Co2Meter_K33::Co2Meter_K33() {
	devAddr = K33_BLG_ELG_ADDRESS;
}

Co2Meter_K33::Co2Meter_K33(uint8_t address) {
	devAddr = address;
}

Co2Meter_K33::~Co2Meter_K33(){}


///////////////////////////////////////////////////////////////////
// Function : void wakeSensor()
// Executes : Sends wakeup commands to K33 sensors.
// Note : THIS COMMAND MUST BE MODIFIED FOR THE SPECIFIC AVR YOU ARE USING 
// THE REGISTERS ARE HARD‐CODED 
/////////////////////////////////////////////////////////////////
void Co2Meter_K33::wakeSensor() {
	// This command serves as a wakeup to the CO2 sensor, for K33‐ELG/BLG Sensors Only
	// You'll have the look up the registers for your specific device, but the idea here is simple: 
	// 1. Disabled the I2C engine on the AVR
	// 2. Set the Data Direction register to output on the SDA line
	// 3. Toggle the line low for ~1ms to wake the micro up. Enable I2C Engine
	// 4. Wake a millisecond.
	
	TWCR &= ~(1 << 2); // Disable I2C Engine
	DDRC |= (1 << 4); // Set pin to output mode
	PORTC &= ~(1 << 4); // Pull pin low
	delay(1);
	PORTC |= (1 << 4); // Pull pin high again
	TWCR |= (1 << 2); // I2C is now enabled
	delay(1);
}

////////////////////////////////////////////////////////////////// 
// Function : void initPoll()
// Executes : Tells sensor to take a measurement.
// Notes
//
// ///////////////////////////////////////////////////////////////////
void Co2Meter_K33::initPoll() {
	wakeSensor();
	Wire.beginTransmission(devAddr);
	Wire.write(0x11);
	Wire.write(0x00);
	Wire.write(0x60);
	Wire.write(0x35);
	Wire.write(0xA6);
	Wire.endTransmission();
	delay(20);
	Wire.requestFrom(devAddr, 2);
	byte i = 0;
	byte buffer[2] = {0, 0};
	while (Wire.available()) {
		buffer[i] = Wire.read();
		i++;
	}
}

/////////////////////////////////////////////////////////////////// 
// Function : double readCo2()
// Returns : The current CO2 Value, -1 if error has occured 
///////////////////////////////////////////////////////////////////
double Co2Meter_K33::readCo2() {
	wakeSensor();
	int co2_value = 0;
	// We will store the CO2 value inside this variable. digitalWrite(13, HIGH);
	// On most Arduino platforms this pin is used as an indicator light.
	
	//////////////////////////
	/* Begin Write Sequence */
	//////////////////////////
	
	Wire.beginTransmission(devAddr);
	Wire.write(0x22);
	Wire.write(0x00);
	Wire.write(0x08);
	Wire.write(0x2A);
	Wire.endTransmission();
	
	/*
		 We wait 10ms for the sensor to process our command.
		 The sensors's primary duties are to accurately
		 measure CO2 values. Waiting 10ms will ensure the
		 data is properly written to RAM
	*/
	
	delay(20);
	/////////////////////////
	/* Begin Read Sequence */
	/////////////////////////
	/*
		 Since we requested 2 bytes from the sensor we must
		 read in 4 bytes. This includes the payload, checksum,
		 and command status byte.
	*/
	
	Wire.requestFrom(devAddr, 4);
	byte i = 0;
	byte buffer[4] = {0, 0, 0, 0};
	
	/*
		Wire.available() is not nessessary. 
		Implementation is obscure but we leave it in here for portability and to future proof our code
	*/
	
	while (Wire.available()) {
		buffer[i] = Wire.read();
		i++;
	}
	co2_value = 0;
	co2_value |= buffer[1] & 0xFF;
	co2_value = co2_value << 8;
	co2_value |= buffer[2] & 0xFF;
	byte sum = 0;
	sum = buffer[0] + buffer[1] + buffer[2];
	if (sum == buffer[3]) {
		// Success!
		// digitalWrite(13, LOW);
		//Checksum Byte
		//Byte addition utilizes overflow
		return ((double) co2_value / (double) 1);
	}
	else {
		// Failure!
		/*
			Checksum failure can be due to a number of factors,
			fuzzy electrons, sensor busy, etc.
		*/
		
		// digitalWrite(13, LOW);
		return (double) -1;
	}
}

/////////////////////////////////////////////////////////////////// 
// Function : double readTemp()
// Returns : The current Temperture Value, -1 if error has occured 
///////////////////////////////////////////////////////////////////
double Co2Meter_K33::readTemp() {
	wakeSensor();
	int tempVal = 0;
	// digitalWrite(13, HIGH);
	Wire.beginTransmission(devAddr);
	Wire.write(0x22);
	Wire.write(0x00);
	Wire.write(0x12);
	Wire.write(0x34);
	Wire.endTransmission();
	delay(20);

	Wire.requestFrom(devAddr, 4);
	byte i = 0;
	byte buffer[4] = {0, 0, 0, 0};
	while (Wire.available()) {
		buffer[i] = Wire.read();
		i++;
	}
	tempVal = 0;
	tempVal |= buffer[1] & 0xFF;
	tempVal = tempVal << 8;
	tempVal |= buffer[2] & 0xFF;
	byte sum = 0;
	sum = buffer[0] + buffer[1] + buffer[2];
	//Checksum Byte
	//Byte addition utilizes overflow
	if (sum == buffer[3]) {
		// digitalWrite(13, LOW);
		return ((double) tempVal / (double) 100);
	}
	else {
		// digitalWrite(13, LOW);
		return -1;
	}
}

/////////////////////////////////////////////////////////////////// 
// Function : double readRh()
// Returns : The current Rh Value, -1 if error has occured 
///////////////////////////////////////////////////////////////////
double Co2Meter_K33::readRh() {
	wakeSensor();
	int tempVal = 0;
	// digitalWrite(13, HIGH);
	Wire.beginTransmission(devAddr);
	Wire.write(0x22);
	Wire.write(0x00);
	Wire.write(0x14);
	Wire.write(0x36);
	Wire.endTransmission();
	delay(20);
	Wire.requestFrom(devAddr, 4);
	byte i = 0;
	byte buffer[4] = {0, 0, 0, 0};
	while (Wire.available()) {
		buffer[i] = Wire.read();
		i++;
	}
	tempVal = 0;
	tempVal |= buffer[1] & 0xFF;
	tempVal = tempVal << 8;
	tempVal |= buffer[2] & 0xFF;
	byte sum = 0;
	sum = buffer[0] + buffer[1] + buffer[2];
	//Checksum Byte
	//Byte addition utilizes overflow
	if (sum == buffer[3]) {
		// digitalWrite(13, LOW);
		return (double) tempVal / (double) 100;
	}
	else {
		// digitalWrite(13, LOW);
		return -1;
	}
}