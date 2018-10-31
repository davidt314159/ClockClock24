#include <Wire.h>

int count;

void setup() {
    count = 0;
    Wire.begin();
    pinMode(13, OUTPUT);

    //Set all ports to output
    Wire.beginTransmission(0x20);
    Wire.write(0x00); // IODIRA register
    Wire.write(0x00); // set all of port A to outputs
    Wire.endTransmission();
    digitalWrite(13, HIGH);
}

void loop() {
    digitalWrite(13, HIGH);
    step(2048);
    delay(1000);
    digitalWrite(13, LOW);
    delay(1000);
}

void step(int numSteps) {
    for (int i = 0; i < numSteps; i++) {
        count = i % 4;

        Wire.beginTransmission(0x20);
        Wire.write(0x14); //writing to A pins 0-3
        Wire.write(1 << count);
        Wire.endTransmission();
        delay(5);
    }

    Wire.beginTransmission(0x20);
    Wire.write(0x14);
    Wire.write(0);
    Wire.endTransmission();
}

void portWrite(int addr, int output) {
}
