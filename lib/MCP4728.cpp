
// LIBRARY FOR MCP4728 
// Link: https://github.com/hideakitai/MCP4728
// Author: Hideaki Tai
// License: MIT (https://github.com/hideakitai/MCP4728/blob/master/LICENSE)
// Extended by Joe Seggiola to include optional LDAC and calibration

#pragma once
#ifndef MCP4728_H
#define MCP4728_H

#include "Arduino.h"
#include <Wire.h>

class MCP4728 {
	
	public:

		enum class CMD {
			FAST_WRITE = 0x00,
			MULTI_WRITE = 0x40,
			SINGLE_WRITE = 0x58,
			SEQ_WRITE = 0x50,
			SELECT_VREF = 0x80,
			SELECT_GAIN = 0xC0,
			SELECT_PWRDOWN = 0xA0
		};
		
		enum class VREF { VDD, INTERNAL_2_8V };
		enum class PWR_DOWN { NORMAL, GND_1KOHM, GND_100KOHM, GND_500KOHM };
		enum class GAIN { X1, X2 };

		void init(TwoWire& w, uint8_t addr = 0, int8_t pin = -1) {
			wire_ = &w;
			addr_ = I2C_ADDR + addr;
			pin_ldac_ = pin;
			calibrate(0, 0, 1.0);
			calibrate(1, 0, 1.0);
			calibrate(2, 0, 1.0);
			calibrate(3, 0, 1.0);
			if (pin_ldac_ > -1) {
				pinMode(pin_ldac_, OUTPUT);
				enable(false);
			}
			readRegisters();
		}
		
		// Multi-point calibration method: send nominal values and pass actual measured mV values
		void calibrate(uint8_t ch, int16_t m0, int16_t m400, int16_t m1000, int16_t m2000, int16_t m3000, int16_t m4000) {
			uint8_t N = 6;
			uint16_t points[N] = { 0, 400, 1000, 2000, 3000, 4000 }; // Nominal values
			uint16_t values[N] = { m0, m400, m1000, m2000, m3000, m4000 }; // Actual measured values
			for (uint8_t i = 1; i < N; i++) {
			    float x1 = points[i - 1], x2 = points[i];
			    float y1 = values[i - 1], y2 = values[i];
				if (i < N - 1) cal_r_[ch][i - 1] = values[i]; // Range definition
				cal_m_[ch][i - 1] = (x1 - x2) / (y1 - y2); // Inverse of segment slope
    			cal_q_[ch][i - 1] = ((x1 * y2 - x2 * y1) / (x1 - x2)) + 0.5; // Segment intercept (0.5 is for rounding)
			}
		}
		
		// Compatibility (old) calibration method: single full linear range
		void calibrate(uint8_t ch, int16_t offset, float gain) {
			cal_r_[ch][0] = 0xFFFF;
			cal_m_[ch][0] = gain;
			cal_q_[ch][0] = -offset;
		}

		void enable(bool b) {
			if (pin_ldac_ > -1) {
				digitalWrite(pin_ldac_, !b);
			}
		}

		uint8_t analogWrite(uint8_t ch, uint16_t data, bool b_eep = false) {
			if (b_eep) {
				eep_[ch].data = this->getCalibratedData(ch, data);
				return singleWrite(ch);
			} else {
				reg_[ch].data = this->getCalibratedData(ch, data);
				return fastWrite();
			}
		}

		uint8_t analogWrite(uint16_t a, uint16_t b, uint16_t c, uint16_t d, bool b_eep = false) {
			if (b_eep) {
				reg_[0].data = eep_[0].data = this->getCalibratedData(0, a);
				reg_[1].data = eep_[1].data = this->getCalibratedData(1, b);
				reg_[2].data = eep_[2].data = this->getCalibratedData(2, c);
				reg_[3].data = eep_[3].data = this->getCalibratedData(3, d);
				return seqWrite();
			} else {
				reg_[0].data = this->getCalibratedData(0, a);
				reg_[1].data = this->getCalibratedData(1, b);
				reg_[2].data = this->getCalibratedData(2, c);
				reg_[3].data = this->getCalibratedData(3, d);
				return fastWrite();
			}
		}

		uint8_t selectVref(VREF a, VREF b, VREF c, VREF d) {
			reg_[0].vref = a;
			reg_[1].vref = b;
			reg_[2].vref = c;
			reg_[3].vref = d;
			uint8_t data = (uint8_t)CMD::SELECT_VREF;
			for (uint8_t i = 0; i < 4; ++i) bitWrite(data, 3 - i, (uint8_t)reg_[i].vref);
			wire_->beginTransmission(addr_);
			wire_->write(data);
			return wire_->endTransmission();
		}

		uint8_t selectPowerDown(PWR_DOWN a, PWR_DOWN b, PWR_DOWN c, PWR_DOWN d) {
			reg_[0].pd = a;
			reg_[1].pd = b;
			reg_[2].pd = c;
			reg_[3].pd = d;
			uint8_t h = ((uint8_t)CMD::SELECT_PWRDOWN) | ((uint8_t)a << 2) | (uint8_t)b;
			uint8_t l = 0 | ((uint8_t)c << 6) | ((uint8_t)d << 4);
			wire_->beginTransmission(addr_);
			wire_->write(h);
			wire_->write(l);
			return wire_->endTransmission();
		}

		uint8_t selectGain(GAIN a, GAIN b, GAIN c, GAIN d) {
			reg_[0].gain = a;
			reg_[1].gain = b;
			reg_[2].gain = c;
			reg_[3].gain = d;
			uint8_t data = (uint8_t)CMD::SELECT_GAIN;
			for (uint8_t i = 0; i < 4; ++i) bitWrite(data, 3 - i, (uint8_t)reg_[i].gain);
			wire_->beginTransmission(addr_);
			wire_->write(data);
			return wire_->endTransmission();
		}
		
		void readRegisters() {
			wire_->requestFrom((int)addr_, 24);
			if (wire_->available() == 24) {
				for (uint8_t i = 0; i < 8; ++i) {
					uint8_t data[3];
					bool isEeprom = i % 2;
					for (uint8_t i = 0; i < 3; ++i) data[i] = wire_->read();
					uint8_t ch = (data[0] & 0x30) >> 4;
					if (isEeprom) {
						read_eep_[ch].vref = (VREF)	((data[1] & 0b10000000) >> 7);
						read_eep_[ch].pd   = (PWR_DOWN)((data[1] & 0b01100000) >> 5);
						read_eep_[ch].gain = (GAIN)	((data[1] & 0b00010000) >> 4);
						read_eep_[ch].data = (uint16_t)((data[1] & 0b00001111) << 8 | data[2]);
					} else {
						read_reg_[ch].vref = (VREF)	((data[1] & 0b10000000) >> 7);
						read_reg_[ch].pd   = (PWR_DOWN)((data[1] & 0b01100000) >> 5);
						read_reg_[ch].gain = (GAIN)	((data[1] & 0b00010000) >> 4);
						read_reg_[ch].data = (uint16_t)((data[1] & 0b00001111) << 8 | data[2]);
					}
				}
			}
		}

		uint8_t getVref(uint8_t ch, bool b_eep = false) { 
			return b_eep ? (uint8_t)read_eep_[ch].vref : (uint8_t)read_reg_[ch].vref; 
		}
		
		uint8_t getGain(uint8_t ch, bool b_eep = false) { 
			return b_eep ? (uint8_t)read_eep_[ch].gain: (uint8_t)read_reg_[ch].gain; 
		}
		
		uint8_t getPowerDown(uint8_t ch, bool b_eep = false) { 
			return b_eep ? (uint8_t)read_eep_[ch].pd : (uint8_t)read_reg_[ch].pd; 
		}
		
		uint16_t getDACData(uint8_t ch, bool b_eep = false) { 
			return b_eep ? (uint16_t)read_eep_[ch].data : (uint16_t)read_reg_[ch].data; 
		}
		
	private:
		
		uint16_t getCalibratedData(uint8_t ch, uint16_t data) {
			int16_t v = 0;
			for (uint8_t i = 0; i < 5; i++) {
				if (i == 4 || data < cal_r_[ch][i]) { // Default is last segment (no range)
					v = (data - cal_q_[ch][i]) * cal_m_[ch][i];
					break;
				}
			}
			return v < 0 ? 0 : (v > 0xFFF ? 0xFFF : v);
		}

		uint8_t fastWrite() {
			wire_->beginTransmission(addr_);
			for (uint8_t i = 0; i < 4; ++i) {
				wire_->write((uint8_t)CMD::FAST_WRITE | highByte(reg_[i].data));
				wire_->write(lowByte(reg_[i].data));
			}
			return wire_->endTransmission();
		}

		uint8_t multiWrite() {
			wire_->beginTransmission(addr_);
			for (uint8_t i = 0; i < 4; ++i) {
				wire_->write((uint8_t)CMD::MULTI_WRITE | (i << 1));
				wire_->write(((uint8_t)reg_[i].vref << 7) | ((uint8_t)reg_[i].pd << 5) | ((uint8_t)reg_[i].gain << 4) | highByte(reg_[i].data));
				wire_->write(lowByte(reg_[i].data));
			}
			return wire_->endTransmission();
		}

		uint8_t seqWrite() {
			wire_->beginTransmission(addr_);
			wire_->write((uint8_t)CMD::SEQ_WRITE);
			for (uint8_t i = 0; i < 4; ++i) {
				wire_->write(((uint8_t)eep_[i].vref << 7) | ((uint8_t)eep_[i].pd << 5) | ((uint8_t)eep_[i].gain << 4) | highByte(eep_[i].data));
				wire_->write(lowByte(eep_[i].data));
			}
			return wire_->endTransmission();
		}

		uint8_t singleWrite(uint8_t ch) {
			wire_->beginTransmission(addr_);
			wire_->write((uint8_t)CMD::SINGLE_WRITE | (ch << 1));
			wire_->write(((uint8_t)eep_[ch].vref << 7) | ((uint8_t)eep_[ch].pd << 5) | ((uint8_t)eep_[ch].gain << 4) | highByte(eep_[ch].data));
			wire_->write(lowByte(eep_[ch].data));
			return wire_->endTransmission();
		}

	private:

		struct DACInputData {
			VREF vref;
			PWR_DOWN pd;
			GAIN gain;
			uint16_t data;
		};

		const uint8_t I2C_ADDR {0x60};

		uint8_t addr_ {I2C_ADDR};
		int8_t pin_ldac_;

		DACInputData reg_[4];
		DACInputData eep_[4];
		DACInputData read_reg_[4];
		DACInputData read_eep_[4];
		
		int16_t cal_r_[4][4]; // Ranges definition
		float cal_m_[4][5]; // Inverse of segments slope
		int16_t cal_q_[4][5]; // Segments intercept

		TwoWire* wire_;
	
};

#endif
