#include <Wire.h>

const double clockRatio;

const int expanderValues[4] = {
  0x0001,
  0x0010,
  0x0100,
  0x1000
}

int count;

void setup() {
  count = 0; 
  Wire.begin();

  //Set all ports to output
  for(int addr = 0; addr < 3; addr++){ //3 MCP23017 chips
    Wire.beginTransmission(0x20 + addr);
    Wire.write(0x00) // IODIRA register
    Wire.write(0x00) // set all of port A to outputs
    //Sequential mode automatically increments to port B?
    Wire.write(0x00) // set all of port B to outputs
    Wire.endTransmission();
  }
}

void loop() {
  step(2048);
  delay(1000);
}

void step(int numSteps){
  for (int i = 0; i < numSteps; i++){
    int count = i % 4;
    digitalWrite(8, steps[count][0]);
    digitalWrite(9, steps[count][1]);
    digitalWrite(10, steps[count][2]);
    digitalWrite(11, steps[count][3]);
    delay(5);

    Wire.beginTransmission(0x20);
    Wire.write(0x14);
    Wire.write(1 << count);
    Wire.endTransmission();
  }

  digitalWrite(8, 0);
  digitalWrite(9, 0);
  digitalWrite(10, 0);
  digitalWrite(11, 0);

  Wire.beginTransmission(0x20);
  Wire.write(0x14);
  Wire.write(0);
  Wire.endTransmission();
}

void portWrite(int addr, int output){
}

