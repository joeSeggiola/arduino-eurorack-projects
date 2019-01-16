#ifndef Button_h
#define Button_h

#include "Arduino.h"

class Button {
	
	public:
		
		/** 
		 * Setup the button, specifying and optional debounce delay
		 */
		Button(int pin, unsigned long debounceDelayMs = 0) {
			
			this->pin = pin;
			this->debounceDelayMs = debounceDelayMs;
			
			this->lastPressedMs = 0;
			
			pinMode(this->pin, INPUT);
			
		}
		
		/** 
		 * Get the button state, TRUE if the pin is HIGH.
		 * Immediately reads presses, but the release can be delayed according to debouncing.
		 */
		bool read() {
			
			int reading = digitalRead(this->pin);
			
			if (reading) {
				
				// Pressed: remember time and return TRUE
				this->lastPressedMs = millis();
				return true;
				
			} else {
				
				// Released: wait for debouncing and return FALSE
				if (this->lastPressedMs > 0) {
					if (millis() - this->lastPressedMs >= debounceDelayMs) {
						this->lastPressedMs = 0; // Reset remembered time
						return false;
					} else {
						return true; // Waiting for debouncing...
					}
				} else {
					return false;
				}
				
			}
			
		}
		
	private:
		int pin;
		unsigned long debounceDelayMs;
		unsigned long lastPressedMs;
		
};

#endif