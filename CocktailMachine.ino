int in1pins[] = {6,4,2};
int in2pins[] = {7,5,3};
int flowPins[] = {13, 12, 11};

char receivedChar; // store info
boolean newData = false; // create a true/false statement

byte sensorInterrupts[] = {0, 1, 2};  // 0 = digital pin 2

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;

volatile byte pulseCounts[3];  

float flowRates[3];
unsigned int flowMilliLitres[3];
unsigned long totalMilliLitres[3];

unsigned long oldTimes[3];

// Define stepper motor connections and steps per revolution:
#define dirPin 8
#define stepPin 9
#define stepsPerRevolution 200

#define clockwise true // to the right
#define counterclockwise false // to the left

int currentBottle = -1;

int stepsForGap[] = {100, 700, 475}; // First element is dist from bottle -1 to bottle 0

void setup() {

  // Declare pins as output:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  // pinMode(rotaryClk, INPUT);
  // pinMode(rotaryDt, INPUT);

  digitalWrite(stepPin, LOW);
  digitalWrite(dirPin, LOW);

  // Set initial motor direction
  setDirection(clockwise);

  for (auto &pin : in1pins) {
    pinMode(pin, OUTPUT);
  }
  for (auto &pin : in2pins) {
    pinMode(pin, OUTPUT);
  }
  for (int i = 0; i < sizeof(flowPins)/sizeof(flowPins[0]); i++) {
    pinMode(flowPins[i], INPUT);
    digitalWrite(flowPins[i], HIGH);

    pulseCounts[i]        = 0;
    flowRates[i]          = 0.0;
    flowMilliLitres[i]   = 0;
    totalMilliLitres[i]  = 0;
    oldTimes[i]           = 0;

    // Configured to trigger on a FALLING state change (transition from HIGH
    // state to LOW state)
    if (i == 0) {
      attachInterrupt(sensorInterrupts[i], pulseCounter0, FALLING);
    } else if (i == 1) {
      attachInterrupt(sensorInterrupts[i], pulseCounter1, FALLING);
    } else {
      attachInterrupt(sensorInterrupts[i], pulseCounter2, FALLING);
    }
  }

  Serial.begin(9600); // start up serial communication
}

void stepMotor(int steps) {
  // Spin the stepper motor 1 revolution slowly:
  for (int i = 0; i < steps; i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPin, HIGH);
    delay(5);
    digitalWrite(stepPin, LOW);
    delay(5);
  }
}

void setDirection(bool dir) {
  if (dir == clockwise) {
    // Set the spinning direction clockwise:
    digitalWrite(dirPin, HIGH);
  } else {
    // Set the spinning direction counterclockwise:
    digitalWrite(dirPin, LOW);
  }
}


void updateFlows() {
  for (int i = 0; i < sizeof(flowPins)/sizeof(flowPins[0]); i++) {
    if((millis() - oldTimes[i]) > 1000)    // Only process counters once per second
    { 
      // Disable the interrupt while calculating flow rate and sending the value to
      // the host
      detachInterrupt(sensorInterrupts[i]);
          
      // Because this loop may not complete in exactly 1 second intervals we calculate
      // the number of milliseconds that have passed since the last execution and use
      // that to scale the output. We also apply the calibrationFactor to scale the output
      // based on the number of pulses per second per units of measure (litres/minute in
      // this case) coming from the sensor.
      flowRates[i] = ((1000.0 / (millis() - oldTimes[i])) * pulseCounts[i]) / calibrationFactor;
      
      // Note the time this processing pass was executed. Note that because we've
      // disabled interrupts the millis() function won't actually be incrementing right
      // at this point, but it will still return the value it was set to just before
      // interrupts went away.
      oldTimes[i] = millis();
      
      // Divide the flow rate in litres/minute by 60 to determine how many litres have
      // passed through the sensor in this 1 second interval, then multiply by 1000 to
      // convert to millilitres.
      flowMilliLitres[i] = (flowRates[i] / 60) * 1000;
      
      // Add the millilitres passed in this second to the cumulative total
      totalMilliLitres[i] += flowMilliLitres;
        
      unsigned int frac;
      
      // Print the flow rate for this second in litres / minute
      Serial.print("Flow rate for ");
      Serial.print(int(i));
      Serial.print(": ");
      Serial.print(int(flowRates[i]));  // Print the integer part of the variable
      Serial.print("L/min");
      Serial.print("\t"); 		  // Print tab space

      // Print the cumulative total of litres flowed since starting
      Serial.print("Output Liquid Quantity: ");        
      Serial.print(totalMilliLitres[i]);
      Serial.println("mL"); 
      Serial.print("\t"); 		  // Print tab space
      Serial.print(totalMilliLitres[i]/1000);
      Serial.print("L");
      

      // Reset the pulse counter so we can start incrementing again
      pulseCounts[i] = 0;
      
      // Enable the interrupt again now that we've finished sending output
      if (i == 0) {
        attachInterrupt(sensorInterrupts[i], pulseCounter0, FALLING);
      } else if (i == 1) {
        attachInterrupt(sensorInterrupts[i], pulseCounter1, FALLING);
      } else {
        attachInterrupt(sensorInterrupts[i], pulseCounter2, FALLING);
      }
    }
  }
}

void moveToBottle(int from, int to) {
  bool isClockwise = false;
  if (to > from) {
    setDirection(clockwise);
    isClockwise = true;
  } else if (from > to) {
    setDirection(counterclockwise);
  }

  int gaps = abs(to-from);
  for (int i = 0; i < gaps; i++) {
    int currBot = -1;
    int stepAmt = 0;
    if (isClockwise) {
      currBot = from + i;
      stepAmt = stepsForGap[currBot+1];
    } else {
      currBot = from - i;
      stepAmt = stepsForGap[currBot];
    }
    stepMotor(stepAmt);
  }
}

// int i = 0;
void loop() {

  recvData(); // read and store data for the valve motorsg

  // if (i==0) {
  //   Serial.println("MOVINGNOW");
  //   int destBottle = 3;
  //   destBottle -= 3;
  //   moveToBottle(currentBottle, 1);
  //   currentBottle = destBottle;
  // }
  // i++;

  while (newData==true) {
    int to = (receivedChar - '0');
    if (to >= -1 && to < 3) {
      moveToBottle(currentBottle, to);
      currentBottle = to;
    }
    newData=false;
    break;
    for (int i = 0; i < sizeof(in1pins)/sizeof(in1pins[0]); i++) { 
      moveMotor(i); // move motor according to data and then reset
    }
  }

  updateFlows();


}

void recvData() {

  if (Serial.available() > 0) { // if the serial monitor has a reading

    receivedChar = Serial.read(); // set char to be what is read
    newData = true; // make statement true

  }

}

void moveMotor(int motorNum) {
  // 1 == OPEN, 2 == CLOSE
  int in1pin = in1pins[motorNum];
  int in2pin = in2pins[motorNum];

  int motordirection = (receivedChar - '0'); // turn recieved data into usable form and give it a name

  //while(newData == true) {

    Serial.println(motordirection); // print motor direction

    if (motordirection == 1) { // if it reads 1...

      digitalWrite(in1pin, HIGH); // turn motor one way
      digitalWrite(in2pin, LOW);

      delay(250);

    }

    else if (motordirection == 2) { // if it reads 2...

      digitalWrite(in1pin, LOW); // turn motor other way
      digitalWrite(in2pin, HIGH);

      delay(250);

    }

    else { // if nothing is read

      digitalWrite(in1pin, LOW); // motor is off
      digitalWrite(in2pin, LOW);

    }

    newData = false; // reset value to false

  //}

}

/*
Interrupt Service Routine for flowMeter
 */
void pulseCounter0()
{
  // Increment the pulse counter
  pulseCounts[0]++;
}
void pulseCounter1()
{
  // Increment the pulse counter
  pulseCounts[1]++;
}
void pulseCounter2()
{
  // Increment the pulse counter
  pulseCounts[2]++;
}