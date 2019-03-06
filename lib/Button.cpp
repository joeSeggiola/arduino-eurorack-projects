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
			this->longPressStartMs = 0;
			
			pinMode(this->pin, INPUT);
			
		}
		
		/** 
		 * Get the button state, TRUE if the pin is HIGH.
		 * Immediately reads presses, but the release can be delayed according to debouncing.
		 */
		bool read() {
			
			int reading = digitalRead(this->pin);
			
			if (reading) {
				
				// Pressed: return TRUE
				this->lastPressedMs = millis(); // Remember time for debouncing
				if (this->longPressStartMs == 0) this->longPressStartMs = millis(); // Start long press detection
				return true;
				
			} else {
				
				// Released: wait for debouncing and return FALSE
				if (this->lastPressedMs > 0) {
					if (millis() - this->lastPressedMs >= debounceDelayMs) {
						this->lastPressedMs = 0; // Reset remembered time
						this->longPressStartMs = 0; // Stop long press detection
					} else {
						return true; // Waiting for debouncing...
					}
				}
				
			}
			
			return false;
			
		}
		
		/** 
		 * Detect button long press, TRUE if the pin was HIGH for longer than given diration.
		 */
		bool readLongPress(unsigned long durationMs) {
			
			if (this->read()) {
				if (millis() - this->longPressStartMs >= durationMs) {
					return true;
				}
			}
			
			return false;
			
		}
		
	private:
		int pin;
		unsigned long debounceDelayMs;
		unsigned long lastPressedMs;
		unsigned long longPressStartMs;
		
};

#endif