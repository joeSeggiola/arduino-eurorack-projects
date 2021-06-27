
// WRITE I2C ADDRESS (device ID) for MCP4728 =================================
//
// Author: Neuroelec
// Source (archived): http://web.archive.org/web/20130630070633/http://neuroelec.com:80/2011/02/soft-i2c-and-programmable-i2c-address
// Extracted from: https://code.google.com/archive/p/neuroelec/source/default/source
// I2C fix from: https://github.com/TrippyLighting/SoftI2cMaster
// MCP4728 datasheet: http://ww1.microchip.com/downloads/en/devicedoc/22187e.pdf
// 
// Because of critical timing of LDAC latch during the address write and read, 
// the library use software I2C master library just for address read and writing.
// Included modified SoftI2cMaster library is required.
// Original library is from fat16lib, http://forums.adafruit.com/viewtopic.php?f=25&t=13722
// 
// If you are using new chip, device ID is 0.
// If you don't know current device ID, just run this scketch and check the serial monitor.
// Once you get current device ID, put proper current device ID in writeAddress() command.
//
// ===========================================================================

#include <Wire.h>

#include "./SoftI2cMaster.h"
#include "./MCP4728.cpp"

#define SCL_PIN A5
#define SDA_PIN A4
#define LDAC_PIN 4

#define CURRENT_DEVICE_ID 1
#define NEW_DEVICE_ID 1

SoftI2cMaster i2c;
MCP4728 dac;

void setup() {

	// Read and write address
	Serial.begin(9600);
	i2c.init(SCL_PIN, SDA_PIN);
	pinMode(LDAC_PIN, OUTPUT);
	writeAddress(CURRENT_DEVICE_ID, NEW_DEVICE_ID);
	delay(100);
	readAddress(); // Read current device ID
	delay(100);

	// Init I2C communication and DAC for testing
	Wire.begin();
	Wire.setClock(400000); // Fast mode
	dac.init(Wire, NEW_DEVICE_ID, LDAC_PIN);
	dac.selectVref(MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V);
	dac.selectPowerDown(MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL);
	dac.selectGain(MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2);
	dac.enable(true);

}

void loop() { 

	// Test ramp
	dac.analogWrite(0, 0, 0, 0);
	delay(2000);
	for (int i = 0; i < 4000; i++) {
		dac.analogWrite(i, i, i, i);
		delayMicroseconds(200);
	}
	delay(2000);

}

void readAddress() {
	digitalWrite(LDAC_PIN, HIGH);
	i2c.start(0B00000000);
	i2c.ldacwrite(0B00001100, LDAC_PIN); // Modified command for LDAC latch
	i2c.restart(0B11000001);
	uint8_t address = i2c.read(true);
	i2c.stop();
	int readAddress = (address & 0B00001110) >> 1;
	Serial.print("Read address: ");
	Serial.print(readAddress, DEC);
	Serial.print(" (");
	Serial.print(readAddress, BIN);
	Serial.println(")");
}

void writeAddress(int oldAddress, int newAddress) {
	Serial.print("Writing address: ");
	Serial.print(newAddress, DEC);
	Serial.print(" (");
	Serial.print(newAddress, BIN);
	Serial.println(")");
	digitalWrite(LDAC_PIN, HIGH);
	i2c.start(0B11000000 | (oldAddress << 1));
	i2c.ldacwrite(0B01100001 | (oldAddress << 2), LDAC_PIN); // Modified command for LDAC latch
	i2c.write(0B01100010 | (newAddress << 2));
	i2c.write(0B01100011 | (newAddress << 2));
	i2c.stop();
}
