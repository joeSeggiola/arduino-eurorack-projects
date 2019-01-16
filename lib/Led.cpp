#ifndef Led_h
#define Led_h

#include "Arduino.h"

class Led {
	
	public:
		
		/** 
		 * Setup the LED, specifying and optional minimum "on" duration for user visibility
		 */
		Led(int pin, unsigned long minDurationMs = 0) {
			
			this->pin = pin;
			this->minDurationMs = minDurationMs;
			
			this->state = false;
			this->stateHardware = false;
			this->lastOnMs = 0;
			
			pinMode(this->pin, OUTPUT);
			digitalWrite(this->pin, LOW);
			
		}
		
		/** 
		 * Turn the LED on or off. 
		 * If a minimum duration was set, it could not turn off if it was 
		 * on for too little, you need to call loop() to update the state.
		 */
		void set(bool state) {
			
			this->state = state;
			
			if (state) {
				
				// Remember last time it was requested to be on
				this->lastOnMs = millis();
				
				// Turn on the LED if necessary
				if (!this->stateHardware) {
					this->stateHardware = true;
					digitalWrite(this->pin, HIGH);
				}
				
			} else {
				
				// Turn the LED off if necessary
				this->loop();
				
			}
			
		}
		
		/**
		 * Turn the LED off if necessary.
		 * Call this in the main loop if a minimum duration was set.
		 */
		void loop() {
			
			// Turn the LED off if necessary
			if (!this->state && this->stateHardware) {
				if (millis() - this->lastOnMs >= this->minDurationMs) {
					this->stateHardware = false;
					digitalWrite(this->pin, LOW);
				}
			}
			
		}
		
		void on() {
			this->set(true);
		}
		
		void off() {
			this->set(false);
		}
		
		void toggle() {
			this->set(!state);
		}
		
	private:
		int pin;
		bool state;
		bool stateHardware;
		unsigned long lastOnMs;
		unsigned long minDurationMs;
		
};

#endif