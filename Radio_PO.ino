 /*
 * Program to control relay to mimic GM "Retained Accessory Power"
 * http://www.the12volt.com/relays/relaydiagram30.html
 * using Digispark Pro http://digistump.com/wiki/digispark/tutorials/connecting
 * attiny85 based device with 5v regulator
 * power on and assume that the radio can stay on
 * when pin is grounded, the processor resets and shuts down. 
 * 
 * led on pin 1
 * 30k resistor and 5.6v zener on door and run pins
 * 1n4001 from +15 to radio lead
 * red +30, yellow +15, blue radio, black gnd 
 * Pin 5 acts as reset, so when this grounds, it powers off.
 * Moved door pin to pin 3
 * 
 * Made a schematic and added a lot of components to protest both the car and the tiny
 * 
 * pcint set for ignition off. When switched off, switch the pin to the door and await the 
 * pin going low to turn off or the timer to expire. 
 * 
 * For some reason digispark will not register hi to low on pin 4
 * 
 * rev 4 worked but relied on reset and polling...
 * rev 5 re-write to use pcint and to make it work -- had to move door to pin 3 from 4
 */
 
// code to turn off radio when door is opened or after 15 min
#define relayPin 0 // output to relay
#define ledPin 1   // on board led high to turn on
#define ignPin 2   // igniton on = HIGH
#define doorPin 3  // gnd will shut it down

unsigned long currentMillis;
unsigned long previousMillis;
unsigned long interval = 500;    // how often to flash led
unsigned long offDelay = 900000; // 900000 is 15 min
unsigned long offTime;           // calculated time 15 min after power off
bool ledState = LOW;             // start blink off
bool ignState = HIGH;            // assume ign ON as power up causes it to go on
bool doorState = HIGH;           // waiting for a gnd
volatile bool intSignal = HIGH;  // powered up and waiting for a turn off

void setup() { 
  // initialize the digital pin as an output.
  pinMode(doorPin,INPUT);  // high when running
  pinMode(ignPin, INPUT);  // high when running (15) - this needs a pull down
  pinMode(relayPin, OUTPUT); // LOW to turn on relay
  digitalWrite(relayPin, LOW);  // turn on relay at power up
  pinMode(ledPin, OUTPUT); //LED on DigiSpark board, HIGH to turn on
  digitalWrite(ledPin, HIGH); // turn on LED

  cli();
  PCMSK = 0b00000100;    // turn on PCINT interrupt on pin PB2 - wait for ign off 
  GIMSK = 0b00100000;    // turns on pin change interrupts
  sei();
}

void loop() {
  currentMillis = millis();
  if (!intSignal && ignState) {  // we turned ign off
    // start timer
    offTime = currentMillis + offDelay;  // set turn off time
    ignState = LOW;                      // ign flag off
    interval = 250;                      // blink led faster after ign off
    cli();
    PCMSK = 0b00001000;    // trigger PCINT0_vect on pin PB3 - waiting for door 
    sei();
    intSignal = HIGH;      // set back to HIGH for door interrupt
  }

  if (!ignState && currentMillis >= offTime) { // ign off and timer expired
    interval = 100;                             // blink even faster
    digitalWrite(relayPin, HIGH);               // turn off relay - powers it all down
  }

  if (!ignState && !intSignal) {               // ign off waiting for a door 
    digitalWrite(relayPin, HIGH);               // turn off relay - powers it all down
  }

  if (currentMillis - previousMillis >= interval) { // led blinking
    // save the last time the LED blinked
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}

/*
 * Command: interrupt handler
 */
ISR(PCINT0_vect) {
  intSignal = LOW;
}
