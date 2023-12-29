#ifndef WDT_H
#define WDT_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

namespace wdt {

    // a void function to call when the interrupt occurs.
    typedef void (*onInterrupt)();

    // Enumeration of the various WDT timeout settings.
    // These match the ones in avr/wdt.h in value but that
    // file has WDTO_15MS instead of 16 milliseconds. Strange?
    typedef enum timeout : uint8_t {
        TIMEOUT_16MS = WDTO_15MS,
        TIMEOUT_32MS,
        TIMEOUT_64MS,
        TIMEOUT_125MS,
        TIMEOUT_250MS,
        TIMEOUT_500MS,
        TIMEOUT_1S,
        TIMEOUT_2S,
        TIMEOUT_4S,
        TIMEOUT_8S
    } timeout_t ;


	class AVR_wdt {

	public:

		// Constructor. There's no destructor as the one
		// AVRwdt object will never be destructed.
		AVR_wdt();

		// Enable the WDT.
		// If a function pointer is passed, then that will be
		// called every timeout assuming no reset is required.
		// If the device is required to reset, the interrupt
		// will fire once, then on the next timeout, the
		// board will reset.
		//
		//=====================================================
		// WARNING WARNING WARNING WARNING WARNING WARNING 
		//=====================================================
		// On some devices, this reset will cause a
		// reset loop - Duemilanove, Nano, I'm looking at you!
		// The Uno seems ok.
		//
		// If you get a reset loop, it's because on restarting,
		// the WDT is still active, plus, it has been reset to
		// timeout after every 16 milliseconds! Not good.
		// The Duemilanove/Nano bootloaders take too long to 
		// get to the point of resetting the WDT, so constantly
		// reset.
		//
		// The ONLY WAY to reset these devices is to burn a
		// bootloader with an ICSP or "Arduino as ICSP".
		//=====================================================

		void begin(timeout_t timeout = wdt::TIMEOUT_1S,
		            onInterrupt wdtFunction = nullptr,
		            bool resetRequired = false);

		// Reset the WDT counter if we requested a reset.
		void reset() {
		    wdt_reset();
		}

		// Turn off the WDT. If the WDT is planning to reset, 
		// we should run the timed instruction sequence again.
		void end();

		// Will we reset the device?
		bool willReset() {
		    return resetRequested;
		}

		// Will we interrupt?
		bool willInterrupt() {
		    return (interruptFunction != nullptr);
		}

		// Execute the interrupt code. This should be called 
		// from the ISR.
		void interrupt() {
		    if (interruptFunction) {
		        interruptFunction();
		    }
		}

	private:

		// (User) function to call when triggered.
		onInterrupt interruptFunction;

		// Will we reset if the WDT timesout?
		bool resetRequested;


	};

} // End of namespace wdt.

//There will be one of these in the cpp file for this class.
extern wdt::AVR_wdt AVRwdt;

#endif // WDT_H
