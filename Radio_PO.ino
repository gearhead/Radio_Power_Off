/*
 * Program to control relay to mimic GM "Retained Accessory Power"
 * http://www.the12volt.com/relays/relaydiagram30.html
 * using Digispark http://digistump.com/wiki/digispark/tutorials/connecting
 * leveraging pin change interrupts https://thewanderingengineer.com/2014/08/11/pin-change-interrupts-on-attiny85/
 * attiny85 based device with 5v regulator
 * power on and assume that the radio can stay on
 * when pin is grounded, the processor resets and shuts down. 
 * 
 * led on pin 1
 * 200 resistor and 5.6v zener on door and run pins
 * 1n4001 from +15 to radio lead
 * 100k lullup on door pin, 100k pulldown on ign pin
 * 2n2222 to turn on relay (switched polarity on pin 1)
 * red +30, yellow +15, blue radio, black gnd 
 * pins 3 and 4 are used for USB and have 3v6 zeners and resistors so their thresholds are
 * not good for triggering. Ensure the relay is on 4 and the pb0 and pb1 pins are used for triggers
 * 
 * Made a schematic and added a lot of components to protect both the car and the tiny
 * 
 * pcint set waiting for ignition off. When switched off, await the 
 * pin going low to turn off or the timer to expire.
 * 
 * For some reason digispark will not register hi to low on pin 4
 * 
 * rev 4 worked but relied on reset and polling...
 * rev 5 re-write to use pcint and to make it work -- had to move door to pin 3 from 4
 * rev 6 simplify if statements add delay/debounce to interrupt, add checks to ign state
 * rev 7 add transistor to turn on relay. voltage droop is enough to drop out
 * rev 8 logic: if key off then door close it should wait 15 min then turn off - untested
 * rev 9 turn off 15 seconds after door pin is pulled, instead of 15 minutes - could lock when ign back on
 * rev 10 create state machine with case switch what is the state of the door pin at off, key on and open
 */
 
// code to turn off radio when door is opened or after 15 min
#define ignPin 0   // igniton on = HIGH waiting for LOW
#define ledPin 1   // on board led high to turn on
#define doorPin 2  // HIGH waiting for LOW
#define relayPin 4 // output to relay HIGH to turn on

const unsigned long debounce = 100;  // ms debounce on interrupt pins

unsigned long currentMillis;
unsigned long previousMillis;
unsigned long interval = 1000;    // how often to flash led
unsigned long offDelay = 898000; // 900000 is 15 min, this is 15 seconds short
unsigned long offTime;           // calculated time 15 min after power off
bool ledState = LOW;             // start blink off
bool shutDown = LOW;
int powerState = 0;
volatile bool intSignal = HIGH;  // powered up and waiting for a turn off - LOW
// interrupt debounce vars
volatile unsigned long last_interrupt_time;
volatile unsigned long interrupt_time;

void setup() { 
  // initialize the pins
  pinMode(ignPin, INPUT);  // high when running (15) - this needs a pull down
  pinMode(doorPin,INPUT);  // high when running - needs pull down/
  // do not read door Pin as door may be open then closed as car is started assume HIGH
  pinMode(relayPin, OUTPUT); // LOW to turn on relay
  digitalWrite(relayPin, HIGH);  // turn on relay at power up
  pinMode(ledPin, OUTPUT); //LED on DigiSpark board, HIGH to turn on
  // set up PCINT
  cli();
  // set PCINT0_vect to be triggered by the ignition pin first 
  GIMSK = 0b00100000;    // turns on pin change interrupts: generic look for a change on all pins
  //PCMSK |= (1 << ignPin); // check this with a digitalRead
  PCMSK = 0b00000101;  // pins 0 and 2
  sei();
}

void loop() {
  currentMillis = millis();
  switch (powerState) {
    case 0: // powered on check for ign off
      if (!intSignal) {
        if (!digitalRead(ignPin)) {                  //  turned ign off. wait for door
        // start timer
        offTime = currentMillis + offDelay;          // set turn off time
        interval = interval >> 1;                    // blink led faster after ign off
        powerState = 1;
        intSignal = HIGH;
        }
      }  
    break;
    case 1: // ign off. Check for timer or door or ign back on
      if(!intSignal) {                               // we got an interrupt 
        if(digitalRead(ignPin)) {                    // turned ign back on
          powerState = 0;                            // reset to state 0
          interval = interval << 1;
          break;
        }
        if(!digitalRead(doorPin)) {                  // we got a door pin gnd
          powerState = 2;                            // get ready to turn off
          interval = interval >> 1;                  // blink even faster
          offTime = currentMillis + 2000;            // turn it off in 5 seconds
        }
      intSignal = HIGH;                              // reset in case state 0
      break;
      }
      if (currentMillis >= offTime) {                // timer expired
          interval = interval >> 1;                  // blink even faster
          powerState = 2;
          offTime = currentMillis + 2000;    
          //turn it off in 5 seconds
          break;
      }
    break;
    case 2:                                          // Power down
      if (currentMillis >= offTime) {
      digitalWrite(relayPin, LOW);                   // turn off relay - powers it all down
      }
    break;
  }

  if (currentMillis - previousMillis >= interval) {  // blink the LED
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
  interrupt_time = currentMillis;
  // If interrupts come faster than debounce ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > debounce) {
    intSignal = LOW; // interrupt pin changes state - LOW
  }
  last_interrupt_time = interrupt_time;
}
