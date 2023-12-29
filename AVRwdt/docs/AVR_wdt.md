## AVR_wdt Library

This library can be usd with the Arduino IDE and also with other systems, such as PlatformIO. The library makes handling the Atmega328's Watchdog Timer, known as WDT from here on, including interrupts, very simple indeed.

## Library Documentation

### Types

#### onInterrupt

This type defines the function that will be called when the interrupt is triggered. The function must be as follows:

```
void interruptFunction() {
    // Do something on interrupt here;
}
```

It will be called if, and only if, it has been attached to the interrupt by the `begin()` function. See below for details.

**WARNING** Do not call the `reset()` function from within the interrupt function. If you do, the results will be *undefined* as the data sheet puts it!


#### timout_t

The `timout_t` type defines the 10 different timeout periods that the WDT counter's prescaler can facilitate. No other values are permitted and while the prescaler takes 4 bits to define a timeout period, only the first 10 are valid. The timeout period is passed to the `begin()` function. The different values are:

* **wdt::TIMEOUT_16MS**   The watchdog will timeout after 16 milliseconds.
* **wdt::TIMEOUT_32MS**   The watchdog will timeout after 32 milliseconds.
* **wdt::TIMEOUT_64MS**   The watchdog will timeout after 64 milliseconds.
* **wdt::TIMEOUT_125MS**  The watchdog will timeout after 125 milliseconds.
* **wdt::TIMEOUT_250MS**  The watchdog will timeout after 250 milliseconds.
* **wdt::TIMEOUT_500MS**  The watchdog will timeout after 500 milliseconds.
* **wdt::TIMEOUT_1S**     The watchdog will timeout after 1 second or 1,000 milliseconds.
* **wdt::TIMEOUT_2S**     The watchdog will timeout after 2 seconds or 2,000 milliseconds.
* **wdt::TIMEOUT_4S**     The watchdog will timeout after 4 seconds or 4,000 milliseconds.
* **wdt::TIMEOUT_8S**     The watchdog will timeout after 8 seconds or 8,000 milliseconds.

**NOTE** These timings are "exact" (roughly!) when the microcontroller is running on a 5V supply. See the data sheet for details.


### Functions

To use the library, you simply need one line of code:

```
#include "AVR_wdt.h"
```

By including this header file, a new WDT object named `AVRwdt` is created and allows access to the Watchdog.

The `AVRwdt` object has a number of functions to make your handling of the WDT much easier. Those functions are described in the following sections.


**`void AVRwdt.begin(timeout_t timeout = wdt::TIMEOUT_1S, onInterrupt wdtFunction = nullptr, bool resetRequired = false)`**

Configures the WDT in one of three modes depending on the parameters passed.

The parameters are:

* **`timeout_t timeout`** - specifies the delay period before the WDT triggers the interrupt and/or resets the system. This is a mandatory parameter and defaults to a 1 second delay.
  
* **`onInterrupt wdtFunction`** - this parameter specifies the name of the function to be called when the WDT interrupt fires. It is defined as a `void` function which takes no parameters -- `void functionName()`.
  
  This is an optional parameter and defaults to `nullptr` which means that there will not be an interrupt configured for the WDT.

* **`bool resetRequired`** - this optional parameter defaults to `false` and is used to determine if the WDT will be put into one of the two *System Reset* modes, as described shortly.
  
  If `true`, the the system will reset if the WDT counter is not reset, by calling `reset()` before the timeout period expires.

The WDT modes are:

* **System Reset Mode** - requires a timeout and `resetRequired` to be `true`. Unless `reset()` is called in your code at regular intervals, which must be less than the timeout supplied, then the board will be reset. It is not advised to run this mode on older Arduinos -- pretty much anything prior to the Uno Mk3 -- or anything else with a bootloader.

  The reason being that the bootloader will take a certain time to run, and if it doesn't turn off or reconfigure  the WDT quickly enough, then another reset will happen -- repeat forever. This has been proved on an Arduino Duemilanove. An Uno mk3 works fine and it uses a different bootloader whihc gets hold of the WDT first.

  The problem is caused by the WDT remaining enabled after the reset, but when it restarts, the WDT is running with a new timeout, `wdt::TIMEOUT_16MS`, so any bootloader must respond faster than 16 milliseconds to prevent a reset loop.

  In the event of a loop, the only way to get control of your board is to burn a bootloader with an ICSP (or use another Arduino as an ICSP) as the same bootloader is still running, and looping!

* **System Reset plus Interrupt Mode** - requires all three parameters to be passed. It will trigger the interrupt after the timeout period has expired for the first time, then will reset the device after the next timeout period.
  
  This mode allows the application to save any required data off to EEPROM, or similar, within the interrupt, prior to the system being reset by the next timeout. It is effectively, your last chance to save any volatile data before it is lost.

  In this mode, the hardware will disable the interrupt when it has completed it's work to prevent it being triggered again on the next timeout. The hardware effectively converts this mode into *System Reset* mode.

* **Interrupt Mode** - this mode requires a timeout and a function to be passed. The `resetRequired` parameter should be `false`. 
  
  In this mode, the interrupt will be triggered after each timeout period. There is no need to call `reset()` at any time while running in this mode.

  The system *will not* be reset while running in this mode.

Examples:

```
// Run WDT in System Reset Mode. The system
// will reset after 8 seconds if no call to 
// reset() is made.
AVRwdt.begin(wdt::TIMEOUT_8S,
             nullptr,
             true);
```

```
// WDT interrupt function.
void wdtInterrupt() {
    // Do something here.
}


// Run WDT in System Reset plus Interrupt Mode. 
// The system will execute wdtInterrupt() after 8 seconds
// if no call to reset() is made. It will then reset the
// system after a further 8 seconds, unless reset.
//
// NOTE: After the interrupt triggers, it will be 
// disabled and never trigger again.
AVRwdt.begin(wdt::TIMEOUT_8S,
             wdtInterrupt,
             true);
```

```
// WDT interrupt function.
void wdtInterrupt() {
    // Blink an LED, or something, here.
}

// Run WDT in Interrupt Mode. Every second the wdtInterrupt()
// function will be called. The system will never
// be reset in this mode. Calling the reset() function
// will have no effect.
AVRwdt.begin(wdt::TIMEOUT_1S,
             wdtInterrupt,
             false);
```


**`void AVRwdt.end()`**

Calling this function disables the WDT completely. Any configured interrupt function will no longer be executed and the system will no longer reset if configured to do so.

The WDT is reconfigured to its disabled state and if subsequently required, should be reconfigured by calling `begin()` again.

For example:

```
// Disable the Watchdog.
AVRwdt.end();
```


**`void AVRwdt.reset()`**

Resets the WDT timer and allows it to start counting down again. In either of the *System Reset* modes, you must call `reset()` within the timeout period, if you wish to prevent the WDT from resetting the system. 
If your code takes longer than the timeout period, then you can call `reset()` more frequently.

For example, when the code executes in less time than the timeout:

```
// Configure System Reset Mode, with 2 second timeout.
AVRwdt.begin(wdt::TIMEOUT_2S,
             nullptr,
             true);

// Main loop
while (1) {
    // Do some stuff.
    ...

    // Reset the Watchdog.
    AVRwdt.reset();
}
```

For example, when the code executes in more time than the timeout, occasionally, but usually less:

```
// Configure System Reset Mode, with 2 second timeout.
AVRwdt.begin(wdt::TIMEOUT_2S,
             nullptr,
             true);

// Main loop
while (1) {
    // Do some stuff.
    ...

    // Reset the Watchdog.
    AVRwdt.reset();

    // Do some more stuff.
    ...

    // Reset the Watchdog again.
    AVRwdt.reset();
}
```

In the *Interrupt Mode*, the system won't be reset so there's no need to call `reset()`.


**`bool AVRwdt.willReset()`**

Returns `true` if the WDT has been configured to reset the system. This function simply returns the value of the `resetRequired` parameter passed to the `begin()` function.

 For example:

```
// If we configured the WDT to reset, better reset the counter!
if (AVRwdt.willReset())
    AVRwdt.reset();
```

**`bool AVRwdt.willInterrupt()`**

Returns `true` if the WDT has been configured to trigger the interrupt. This function simply returns whether the value of the `wdtFunction` parameter passed to the `begin()` function was `nullptr`  (returns `false`) or otherwise (returns `true`).

 For example:

```
// If the WDT is going to interrupt, that's fine, otherwise we
// need to do something else instead.
if (AVRwdt.willInterrupt())
    doSomething();
else
    doSomethingElse();
```

**`void AVRwdt.interrupt()`**

This function is intended for internal use only. It is what allows the ISR (Interrupt Service Routine) for the WDT to execute the function passed to `begin()`.

The `interrupt()` function can be called from user code, as a means of manually executing the interrupt function, but this is a *bad idea* because:

* When an interrupt is triggered, global interrupts are turned off to prevent nested interrupts;

* While the ISR is executing your function code, the WDT will not timeout and reset the system, assuming it is running in *System Reset plus Interrupt Mode*, when you call it manually, all timing bets are off.

Just don't do it. The function has to be `public` because the ISR isn't part of the `AVRwdt` object, and cannot be. (Unless you know different?)


### Example Arduino Sketch

```
//============================================================
// A demonstration of using the Watchdog Timer (WDT) to flash
// an LED on pin D7 every second, while at the same time, the
// loop() function will be flashing the built in LED every 5 
// seconds. This shows how interrupts don't care what calls to
// delay() your code makes!
//
//============================================================
// WARNING
//
// DO NOT enable a WDT reset if you are using a bootloader. It
// might work on an UNO mk3, but on a Duemilanove and other
// older Arduino boards, there will be a "WDT Reset Loop" as:
//
// The WDT is reconfigured after a reset to 16 milliseconds;
// The WDT is still enabled after a WDT reset;
// The bootloader takes too long to execute and the WDT then
// resets the board again and again and again and again ... 
//============================================================
//
// Norman Dunbar
// 27 October 2020.
//============================================================

// Create the AVRwdt object.
#include "AVR_wdt.h"

// LED pin for the WDT to flash.
#define WDT_LED 7

void wdtInterrupt() {
    digitalWrite(WDT_LED, !digitalRead(WDT_LED));
}


void setup() {
    // Initialise Serial.
    Serial.begin(9600);

    // Do the pins. Pin 7 is for the WDT interrupt.
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(WDT_LED, OUTPUT);

    // Enable the WDT to fire every second.
    AVRwdt.begin(wdt::TIMEOUT_125MS, wdtInterrupt, false);

    // Inform the user as to what will happen.
    Serial.print("WDT will reset: ");
    Serial.println((AVRwdt.willReset()) ? "YES" : "NO");
    
    Serial.print("WDT will interrupt: ");
    Serial.println((AVRwdt.willInterrupt()) ? "YES" : "NO");    
}


void loop() {
    // Flash the builtin LED every 5 seconds.    
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(5000);
}
```

### Example for PlatformIO

```
//============================================================
// A demonstration of using the Watchdog Timer (WDT) to flash
// an LED on pin PD7 every second, while at the same time, the
// main loop will be flashing the LED on PB5 every 5 seconds.
// This shows how interrupts don't care what calls to delay
// functions your code makes!
//
//============================================================
// WARNING
//
// DO NOT enable a WDT reset if you are using a bootloader. It
// might work on an UNO mk3, but on a Duemilanove and other
// older Arduino boards, there will be a "WDT Reset Loop" as:
//
// The WDT is reconfigured after a reset to 16 milliseconds;
// The WDT is still enabled after a WDT reset;
// The bootloader takes too long to execute and the WDT then
// resets the board again and again and again and again ...
//============================================================
// Norman Dunbar
// 27 October 2020.
//============================================================

#include "AVR_wdt.h"
#include "util/delay.h"

// ISR to flash PD7 every second.
void wdtInterrupt() {
    PIND |= (1 << PIND7);
}


int main() {
    // Setup here.
    // Do the pins. Pin PD7 is for the WDT interrupt.
    DDRB |= (1 << DDB5);
    DDRD |= (1 << DDD7);

    // Enable the WDT to fire every second.
    AVRwdt.begin(wdt::TIMEOUT_1S, wdtInterrupt, false);

    // Arduino starts interrupts. Nothing else does!
    // So don't be like me, and forget!
    sei();


    // Loop here.
    while (1) {
        // Flash the builtin LED every 5 seconds.
        PINB |= (1 << PINB5);
        _delay_ms(5000);
    }
}
```
