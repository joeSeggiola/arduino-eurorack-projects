#ifndef CV_h
#define CV_h

#include "Arduino.h"

class CV {
	
	public:
		
		/** 
		 * Setup the analog input reader (CV input or knob), specifying optional thresholds
		 */
		void init(byte pin, int thresholdLow = 0, int thresholdHigh = 1023, bool invert = false) {
			
			this->pin = pin;
			
			this->thresholdLow = thresholdLow;
			this->thresholdHigh = thresholdHigh;
			this->invert = invert;
			
		}
		
		/**
		 * Return the raw reading, as returned by analogRead()
		 */
		int readRaw() {
			return analogRead(this->pin);
		}
		
		/** 
		 * Return the reading as a float number between 0 and 1, included.
		 * Optional thresholds are used to map the raw values into the returned 0..1 range.
		 */
		float read() {
			
			int r = this->readRaw();
			float f;
			
			if (r <= this->thresholdLow) {
				f = 0.0;
			} else if (r >= this->thresholdHigh) {
				f = 1.0;
			} else {
				f = float(r - this->thresholdLow) / float(this->thresholdHigh - this->thresholdLow);
			}
			
			if (this->invert) {
				return 1.0 - f;
			} else {
				return f;
			}
			
		}
		
	private:
		byte pin;
		int thresholdLow;
		int thresholdHigh;
		bool invert;
		
};

#endif