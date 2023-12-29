//============================================================
// A demonstration of using the Watchdog Timer (WDT) to be set
// up to reset the device if 8 seconds have passed. The loop()
// function will blick the builtin LED every 2 seconds and 
// reset the WDT timer.
//
// This script WILL NOT reset the Arduino.
//
//
// The Serial Monitor will show something similar to this:
//
// 14:38:15.384 -> WDT will reset: YES
// 14:38:15.384 -> WDT will interrupt: NO
//
// I have "Show Timestamp" enabled in the monitor.
// 
// You can see that the device resets only once, at the start.
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
    // Prevent WDT REset Loops UNO ONLY!!!!!!!!
    AVRwdt.end();
    
    // Initialise Serial.
    Serial.begin(9600);

    // Do the pins. Pin 7 is for the WDT interrupt.
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(WDT_LED, OUTPUT);

    // Enable the WDT to reset after 8 seconds if we don't
    // call AVRwdt.reset();    
    AVRwdt.begin(wdt::TIMEOUT_8S, nullptr, true);

    // Inform the user as to what will happen.
    Serial.print("WDT will reset: ");
    Serial.println((AVRwdt.willReset()) ? "YES" : "NO");
    
    Serial.print("WDT will interrupt: ");
    Serial.println((AVRwdt.willInterrupt()) ? "YES" : "NO");  

    // Time spent in Serial needs a reset.
    AVRwdt.reset();
}


void loop() {
    // Flash the builtin LED every 2 seconds.    
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(2000);

    // Reset the WDT timer.
    AVRwdt.reset();    
}
