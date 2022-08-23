# Radio_Power_Off
Arduino/Digispark/Attiny85 code to turn off the radio with door open/15 minutes

 * Program to control relay to mimic GM "Retained Accessory Power"
 * http://www.the12volt.com/relays/relaydiagram30.html
 * using Digispark http://digistump.com/wiki/digispark/tutorials/connecting
 * attiny85 based device with 5v regulator
 * power on and assume that the radio can stay on
 * when pin is grounded, the processor resets and shuts down. 
 * 
 * led on pin 1
 * 30k resistor and 5.6v zener on door and run pins
 * 1n4001 from +15 to radio lead
 * red +30, yellow +15, blue radio, black gnd 
 * Pins 3 and 4 are used for USB and have 3v6 zeners and resistors so their thresholds are
 * not good for triggering. 
 * Ensure the relay is on Pin 4 and the pb0 and pb1 pins are used for triggers
 * 
 * Made a schematic and added a lot of components to protect both the car and the tiny
 * 
 * pcint set waiting for ignition off. When switched off, switch the pin to the door and await the 
 * pin going low to turn off or the timer to expire. 
 * 
 * For some reason digispark will not register hi to low on pin 4
