//============================================================
// A demonsstration of using the Watchdog Timer (WDT) to flash
// an LED on pin D7 every second, while at the same time, the
// loop() function will be flashing the builtin LED every 5 
// seconds. This shows how interrupts don't care what calls to
// delay() your code makes!
//
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
//
// Norman Dunbar
// 27 October 2020.
//============================================================
#include "AVR_wdt.h"

// LED pin for the WDT to flash.
#define WDT_LED 7

void wdtInterrupt() {
    digitalWrite(WDT_LED, !digitalRead(WDT_LED));
}


void setup() {
    // Initialise Serial.
    //Serial.begin(9600);

    // Do the pins. Pin 7 is for the WDT interrupt.
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(WDT_LED, OUTPUT);

    // Enable the WDT to fire every second.
    AVRwdt.begin(wdt::TIMEOUT_1S, wdtInterrupt, false);

    // Inform the user as to what will happen.
    //Serial.print("WDT will reset: ");
    //Serial.println((AVRwdt.willReset()) ? "YES" : "NO");
    
    //Serial.print("WDT will interrupt: ");
    //Serial.println((AVRwdt.willInterrupt()) ? "YES" : "NO");    
}


void loop() {
    // Flash the builtin LED every 5 seconds.    
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(5000);
}
