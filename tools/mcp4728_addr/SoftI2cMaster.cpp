/* Arduino SoftI2cMaster Library
 * Copyright (C) 2009 by William Greiman
 *
 * This file is part of the Arduino SoftI2cMaster Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SoftI2cMaster Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "./SoftI2cMaster.h"
//------------------------------------------------------------------------------
// WARNING don't change anything unless you verify the change with a scope
//------------------------------------------------------------------------------
// init pins and set bus high
void SoftI2cMaster::init(uint8_t sclPin, uint8_t sdaPin)
{
  sclPin_ = sclPin;
  sdaPin_ = sdaPin;
  pinMode(sclPin_, OUTPUT);
  digitalWrite(sdaPin_, HIGH); //Mark_H fix
  pinMode(sdaPin_, OUTPUT);
  digitalWrite(sclPin_, HIGH);
  digitalWrite(sdaPin_, HIGH);
}
//------------------------------------------------------------------------------
// read a byte and send Ack if last is false else Nak to terminate read
uint8_t SoftI2cMaster::read(uint8_t last)
{
  uint8_t b = 0;
  // make sure pullup enabled
  digitalWrite(sdaPin_, HIGH);
  pinMode(sdaPin_, INPUT);
  // read byte
  for (uint8_t i = 0; i < 8; i++) {
    // don't change this loop unless you verify the change with a scope
    b <<= 1;
    delayMicroseconds(I2C_DELAY_USEC);
    digitalWrite(sclPin_, HIGH);
    if (digitalRead(sdaPin_)) b |= 1;
    digitalWrite(sclPin_, LOW);
  }
  // send Ack or Nak
  digitalWrite(sdaPin_, HIGH); //Mark_H fix
  pinMode(sdaPin_, OUTPUT);
  digitalWrite(sdaPin_, last);
  digitalWrite(sclPin_, HIGH);
  delayMicroseconds(I2C_DELAY_USEC);
  digitalWrite(sclPin_, LOW);
  digitalWrite(sdaPin_, HIGH);
  return b;
}
//------------------------------------------------------------------------------
// send new address and read/write without stop
uint8_t SoftI2cMaster::restart(uint8_t addressRW)
{
  digitalWrite(sclPin_, HIGH);
  return start(addressRW);
}
//------------------------------------------------------------------------------
// issue a start condition for i2c address with read/write bit
uint8_t SoftI2cMaster::start(uint8_t addressRW)
{
  digitalWrite(sdaPin_, LOW);
  delayMicroseconds(I2C_DELAY_USEC);
  digitalWrite(sclPin_, LOW);
  return write(addressRW);
}
//------------------------------------------------------------------------------
// issue a stop condition
void SoftI2cMaster::stop(void)
{
  digitalWrite(sdaPin_, LOW);
  delayMicroseconds(I2C_DELAY_USEC);
  digitalWrite(sclPin_, HIGH);
  delayMicroseconds(I2C_DELAY_USEC);
  digitalWrite(sdaPin_, HIGH);
  delayMicroseconds(I2C_DELAY_USEC);
}
//------------------------------------------------------------------------------
// write byte and return true for Ack or false for Nak
uint8_t SoftI2cMaster::write(uint8_t b)
{
  // write byte
  for (uint8_t m = 0X80; m != 0; m >>= 1) {
    // don't change this loop unless you verivy the change with a scope
    digitalWrite(sdaPin_, m & b);
    digitalWrite(sclPin_, HIGH);
    delayMicroseconds(I2C_DELAY_USEC);
    digitalWrite(sclPin_, LOW);
  }
  // get Ack or Nak
  digitalWrite(sdaPin_, HIGH);
  pinMode(sdaPin_, INPUT);
  digitalWrite(sclPin_, HIGH);
  b = digitalRead(sdaPin_);
  digitalWrite(sclPin_, LOW);
  digitalWrite(sdaPin_, HIGH); //Mark_H fix
  pinMode(sdaPin_, OUTPUT);
  return b == 0;
}

//------------------------------------------------------------------------------
// write byte and return true for Ack or false for Nak
uint8_t SoftI2cMaster::ldacwrite(uint8_t b, uint8_t ldacpin)
{
  // write byte
  for (uint8_t m = 0X80; m != 0; m >>= 1) {
    // don't change this loop unless you verivy the change with a scope
    digitalWrite(sdaPin_, m & b);
    digitalWrite(sclPin_, HIGH);
    delayMicroseconds(I2C_DELAY_USEC);
    digitalWrite(sclPin_, LOW);
  }
  // get Ack or Nak
  digitalWrite(ldacpin, LOW);
  digitalWrite(sdaPin_, HIGH);
 
  pinMode(sdaPin_, INPUT);
  digitalWrite(sclPin_, HIGH);
  b = digitalRead(sdaPin_);
  digitalWrite(sclPin_, LOW);
  digitalWrite(sdaPin_, HIGH); //Mark_H fix
  pinMode(sdaPin_, OUTPUT);
  return b == 0;
}
