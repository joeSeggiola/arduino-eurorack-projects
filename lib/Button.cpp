#ifndef Button_h
#define Button_h

#include "Arduino.h"

class Button {
	
	public:
		
		/** 
		 * Setup the button, specifying and optional debounce delay
		 */
		void init(int pin, unsigned int debounceDelayMs = 0) {
			
			this->pin = pin;
			this->debounceDelayMs = debounceDelayMs;
			
			this->lastPressedMs = 0;
			this->longPressStartMs = 0;
			this->shortOrLongPressStartMs = 0;
			
			this->readOnceFlag = false;
			this->readLongPressOnceFlag = false;
			this->readShortOrLongPressOnceFlag = false;
			
			pinMode(this->pin, INPUT);
			
		}
		
		/** 
		 * Get the button state, TRUE if the pin is HIGH.
		 * Immediately reads presses, but the release can be delayed according to debouncing.
		 */
		bool read() {
			
			bool reading = digitalRead(this->pin);
			
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
		 * Same as read(), but returns TRUE only once, until the button is released.
		 */
		bool readOnce() {
			if (this->read()) {
				if (!this->readOnceFlag) {
					this->readOnceFlag = true;
					return true;
				}
			} else {
				this->readOnceFlag = false;
			}
			return false;
		}
		
		/** 
		 * Detect button long press, TRUE if the pin was HIGH for longer than given duration.
		 */
		bool readLongPress(unsigned long durationMs) {
			if (this->read()) {
				if (millis() - this->longPressStartMs >= durationMs) {
					return true;
				}
			}
			return false;
		}
		
		/** 
		 * Same as readLongPress(), but returns TRUE only once, until the button is released.
		 */
		bool readLongPressOnce(unsigned long durationMs) {
			if (this->readLongPress(durationMs)) {
				if (!this->readLongPressOnceFlag) {
					this->readLongPressOnceFlag = true;
					return true;
				}
			} else {
				this->readLongPressOnceFlag = false;
			}
			return false;
		}
		
		/*
		 * A combined readOnce() and readLongPressOnce() for a multi-purpose button.
		 * Returns 1 when the button is released before specified duration (short press).
		 * Returns 2 as soon is the button has been pressed for specified duration.
		 * Returns 0 in subsequent calls, while idle or while being pressed.
		 */
		byte readShortOrLongPressOnce(unsigned long longPressDurationMs) {
			byte r = 0;
			if (this->read()) {
				if (!this->readShortOrLongPressOnceFlag) {
					if (this->shortOrLongPressStartMs == 0) {
						this->shortOrLongPressStartMs = millis();
					} else {
						if (millis() - this->shortOrLongPressStartMs >= longPressDurationMs) {
							this->readShortOrLongPressOnceFlag = true;
							r = 2;
						}
					}
				}
			} else {
				if (!this->readShortOrLongPressOnceFlag) {
					if (this->shortOrLongPressStartMs != 0) {
						r = 1;
					}
				}
				this->shortOrLongPressStartMs = 0;
				this->readShortOrLongPressOnceFlag = false;
			}
			return r;
		}
		
	private:
		byte pin;
		unsigned int debounceDelayMs;
		unsigned long lastPressedMs;
		unsigned long longPressStartMs;
		unsigned long shortOrLongPressStartMs;
		bool readOnceFlag;
		bool readLongPressOnceFlag;
		bool readShortOrLongPressOnceFlag;
		
};

#endif