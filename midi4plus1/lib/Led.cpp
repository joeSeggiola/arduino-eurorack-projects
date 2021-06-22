#ifndef Led_h
#define Led_h

#include "Arduino.h"

class Led {
	
	public:
		
		/** 
		 * Setup the LED, specifying and optional minimum "on" duration for user visibility
		 */
		void init(byte pin, unsigned int minDurationMs = 0) {
			
			this->pin = pin;
			this->minDurationMs = minDurationMs;
			
			this->state = false;
			this->stateHardware = false;
			this->blinkMs = 0;
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
			
			this->blinkMs = 0; // Stop blinking
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
		 * Starts blinking with given period, until any other method is called.
		 * Use duty to specify how long the LED will be on, and invert to flip the blinking phase.
		 * This method can also be used make it fade, using a short period and duty to adjust brightness.
		 * Make sure to call loop() to keep the LED blinking.
		 */
		void blink(unsigned int periodMs, float duty = 0.5, bool invert = false) {
			this->blinkMs = periodMs;
			this->blinkDuty = max(0, min(periodMs, duty * periodMs));
			this->blinkStartedMs = millis() - (invert ? this->blinkDuty : 0);
		}
		
		/**
		 * Turn the LED off if necessary, or keep it blinking.
		 * Call this in the main loop.
		 */
		void loop() {
			
			if (this->blinkMs > 0) {
				
				unsigned long t = ((millis() - this->blinkStartedMs) % this->blinkMs);
				this->stateHardware = t < this->blinkDuty;
				digitalWrite(this->pin, this->stateHardware);
				
			} else {
				
				// Turn the LED off if necessary
				if (!this->state && this->stateHardware) {
					if (millis() - this->lastOnMs >= this->minDurationMs) {
						this->stateHardware = false;
						digitalWrite(this->pin, LOW);
					}
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
		
		/**
		 * Turn on the LED, then turn it off immediately.
		 * A single impulse of light will be visible if LED's minDurationMs is long enough.
		 */
		void flash() {
			this->set(true);
			this->set(false);
		}
		
		/** 
		 * Set the optional minimum "on" duration for user visibility
		 */
		void setMinDurationMs(unsigned int minDurationMs = 0) {
			this->minDurationMs = minDurationMs;
			this->loop();
		}
		
	private:
		byte pin;
		unsigned int minDurationMs;
		bool state;
		bool stateHardware;
		unsigned int blinkMs;
		unsigned long blinkStartedMs;
		unsigned int blinkDuty;
		unsigned long lastOnMs;
		
};

#endif