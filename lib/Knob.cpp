#ifndef Knob_h
#define Knob_h

#include "Arduino.h"

class Knob {
	
	public:
		
		/** 
		 * Setup the knob or CV input, specifying and optional thresholds
		 */
		Knob(int pin, unsigned long thresholdLow = 0, unsigned long thresholdHigh = 1023) {
			
			this->pin = pin;
			this->thresholdLow = thresholdLow;
			this->thresholdHigh = thresholdHigh;
			
		}
		
		/**
		 * Return the raw reading, as returned by analogRead()
		 */
		int readRaw() {
			return analogRead(this->pin);
		}
		
		/** 
		 * Return the reading as a float number between 0 and 1, included.
		 * Optional thresholds are used map the raw values into the returned 0..1 range.
		 */
		float read() {
			
			int r = this->readRaw();
			
			if (r <= this->thresholdLow) {
				return 0.0;
			} else if (r >= this->thresholdHigh) {
				return 1.0;
			} else {
				return float(r - this->thresholdLow) / float(this->thresholdHigh - this->thresholdLow);
			}
			
		}
		
	private:
		int pin;
		int thresholdLow;
		int thresholdHigh;
		
};

#endif