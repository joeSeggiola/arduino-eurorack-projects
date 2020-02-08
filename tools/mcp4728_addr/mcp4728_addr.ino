
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

#include "./SoftI2cMaster.h"

#define SCL_PIN A5
#define SDA_PIN A4
#define LDAC_PIN 4

#define CURRENT_DEVICE_ID 0
#define NEW_DEVICE_ID 0

SoftI2cMaster i2c;

void setup() {
    Serial.begin(9600);
    i2c.init(SCL_PIN, SDA_PIN);
    pinMode(LDAC_PIN, OUTPUT);
    writeAddress(CURRENT_DEVICE_ID, NEW_DEVICE_ID);
    delay(100); // Wait for EEPROM writing 
    readAddress(); // Read current device ID 
}

void loop() { }

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
