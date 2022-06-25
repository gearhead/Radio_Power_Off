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
 * door to pin 3
 */
 
// code to turn off radio when door is opened or after 15 min
#define indPin 1  // on board led high to turn on
#define relayPin 0
#define runPin 2
#define doorPin 4 // previously pin 5 a gnd on 5 will shut it down

bool engineOn = false;  // engine running flag
unsigned long currentMillis;
unsigned long nextMillis;
int runDelay = 1000; // how often to run loop (led flash)
unsigned long offTime;  // millis value when to turn off
unsigned long offDelay = 900000; // 900000 is 15 min
bool ledState = 0;
bool ignState;
bool lastignState;
bool doorState = 1;
bool lastdoorState;

void setup() {
            
  // initialize the digital pin as an output.
  pinMode(doorPin,INPUT_PULLUP);  // high when running
  pinMode(runPin, INPUT);  // high when running - this needs a pull down
  pinMode(indPin, OUTPUT); //LED on DigiSpark board, LOW to turn on
  pinMode(relayPin, OUTPUT); // LOW to turn on
  digitalWrite(relayPin, LOW);  // turn on relay at power up
  digitalWrite(indPin, HIGH);
  currentMillis = millis();
  offTime = currentMillis + offDelay;
  nextMillis = currentMillis + runDelay;
}

void loop() {
   currentMillis = millis();
     if (currentMillis > nextMillis) { // do this once a second
      doorState = digitalRead(doorPin);  // HIGH door closed, LOW door opened
      delay(5);
      if (doorState != lastdoorState) {
      digitalWrite(indPin, !doorState);  // pin hi is led off
      }
      
      ignState = digitalRead(runPin);

      if(ignState == 1) {
        // ign still on increase delay shutoff time
        offTime = currentMillis + offDelay;
      }
      
     if (currentMillis >= offTime) {
      // if door pin goes low or we exceed time turn off relay
      //digitalWrite(indPin, LOW);  // turn off indicator
      digitalWrite(relayPin, HIGH);  // open relay
      }
       nextMillis = currentMillis + runDelay;
       lastdoorState = doorState;
       lastignState = ignState;
     }

}
