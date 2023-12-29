#include "AVR_wdt.h"

namespace wdt {

	// Constructor.
	AVR_wdt::AVR_wdt() {
		// Make sure we initialise the function pointer.
		interruptFunction = nullptr;
		resetRequested = false;
	}

	// Initialises the WDT to timeout as requested. If
	// there is a function to call, we also initialise
	// the interrupt.
	void AVR_wdt::begin(timeout_t timeout,
		                onInterrupt wdtFunction,
		                bool resetRequired) {

		// Save the SREG as interrupts might be enabled - but
		// we don't know that.
		uint8_t oldSREG = SREG;

		// Master setting for the WDT timeout etc.
		uint8_t wdtSetting = 0;

		// If timeout > 2 seconds, we need WDP3 set in the
		// prescaler.
		if (timeout > wdt::TIMEOUT_2S) {
		    wdtSetting |= (1 << WDP3);
		}

		// Otherwise, it's just the timeout AND 7.
		wdtSetting |= (timeout & 7);

		// If we have an interrupt function, save it and
		// enable the interrupt.
		if (wdtFunction) {
		    interruptFunction = wdtFunction;
		    wdtSetting |= (1 << WDIE);
		}

		// If we need to reset on WDT timeout, set that bit.
		if (resetRequired) {
		    wdtSetting |= (1 << WDE);
		    resetRequested = true;
		}

		// Prevent a spurious interrupt.
		wdtSetting |= (1 << WDIF);

		// Now the juicy bit, stop interrupts & reset the WDT.
		cli();
		wdt_reset();

		// Clear the "WDT reset the device" bit in MCUSR.
		MCUSR &= ~(1 << WDRF);

		// Do the timed instruction sequence.
		WDTCSR |= ((1 << WDCE) | (1 << WDE));
		WDTCSR = wdtSetting;

		// Restore previous interrupt state. From now on, if
		// interrupts are on, code needs to reset the timer if
		// a reset was requested. If no reset is needed, 
		// there's no need to reset the timer.
		SREG = oldSREG;
	}


	// Turn off the WDT. In case the WDT is planning to reset, 
	// we should run the timed instruction sequence.
	void AVR_wdt::end() {
		// Save the SREG as interrupts might be enabled - but
		// we don't know that.
		uint8_t oldSREG = SREG;

		// Stop interrupts and reset the WDT.
		cli();
		wdt_reset();

		// Clear the "WDT reset the device" bit in MCUSR. This
		// bit overrides WDE so must be cleared first.
		MCUSR &= ~(1 << WDRF);

		// Do the timed instruction sequence. Then clear all
		// bits in the WDTCSR register. Normally to clear WDIF
		// you would write a 1 to it, but under the timed 
		// sequence this is apparently not needed. 
		// (See data sheet.)
		WDTCSR |= ((1 << WDCE) | (1 << WDE));
		WDTCSR = 0;

		// We no longer will reset.
		resetRequested = false;

		// Should be off now.
		SREG = oldSREG;
	}

} // End of namespace wdt.

// Define our one instance of the AVR_wdt class.
wdt::AVR_wdt AVRwdt;


// This is the ISR for the WDT interrupt. It will
// call the required user function if there is one.
ISR(WDT_vect) {
    AVRwdt.interrupt();
}

