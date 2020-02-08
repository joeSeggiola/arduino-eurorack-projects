/* Arduino SoftI2cMaster and TwiMaster Libraries
 * Copyright (C) 2009 by William Greiman
 *
 * This file is part of the Arduino SoftI2cMaster and TwiMaster Libraries
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
 * along with the Arduino SoftI2cMaster and TwiMaster Libraries.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TWO_WIRE_BASE_H
#define TWO_WIRE_BASE_H
#include <Arduino.h>

// R/W direction bit to OR with address for start or restart
#define I2C_READ 1
#define I2C_WRITE 0

class TwoWireBase {
public:
  /** read a byte and send Ack if last is false else Nak to terminate read */
  virtual uint8_t read(uint8_t last) = 0;
  
  /** send new address and read/write bit without stop */
  virtual uint8_t restart(uint8_t addressRW) = 0;
  
  /** issue a start condition for i2c address with read/write bit */
  virtual uint8_t start(uint8_t addressRW) = 0;
  
  /** issue a stop condition */
  virtual void stop(void) = 0;
  
  /** write byte and return true for Ack or false for Nak */
  virtual uint8_t write(uint8_t data) = 0;
};
#endif // TWO_WIRE_BASE_H
