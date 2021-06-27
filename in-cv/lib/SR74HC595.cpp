#ifndef SR74HC595_h
#define SR74HC595_h

#include "Arduino.h"

class SR74HC595 {
	
	public:
		
		/** 
		 * Setup the shift register interface. The "clock pin" is for the shift register 
		 * clock (SCK), the "latch pin" is for the storage register clock (RCK). 
		 * https://www.arduino.cc/en/Tutorial/ShiftOut
		 */
		void init(byte dataPin, byte clockPin, byte latchPin) {
			
			this->dataPin = dataPin;
			this->clockPin = clockPin;
			this->latchPin = latchPin;
			
			pinMode(this->dataPin, OUTPUT);
			pinMode(this->clockPin, OUTPUT);
			pinMode(this->latchPin, OUTPUT);
			
		}
		
		/**
		 * Writes 8 bits to the shift register, and enables the storage register when finished (latch).
		 * The default order is MSBFIRST (most significant bit first), but it can be changed to LSBFIRST.
		 */
		void write(byte value, uint8_t order = MSBFIRST) {
			digitalWrite(latchPin, LOW); // So the outputs don't change while sending in bits
			shiftOut(this->dataPin, this->clockPin, order, value);
			digitalWrite(latchPin, HIGH); // The outputs update at once
		}
		
	private:
		byte dataPin;
		byte clockPin;
		byte latchPin;
		
};

#endif