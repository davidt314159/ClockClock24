#include <stdlib.h>
#include <limits.h>
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

const double clockRatio;

//Number of 1/8 hour intervals after 12:00
const byte targetPositions[10][6] = {
    {50, 46, 48, 48, 24, 72}, //0
    {61, 52, 61, 48, 61, 0}, //1
    {26, 46, 50, 72, 24, 78}, //2
    {26, 46, 26, 72, 26, 72}, //3
    {52, 52, 24, 48, 61, 0}, //4
    {50, 78, 24, 46, 26, 72}, //5
    {50, 78, 48, 46, 24, 72}, //6
    {26, 46, 61, 48, 61, 0}, //7
    {50, 46, 24, 72, 24, 72}, //8
    {50, 46, 24, 48, 26, 72} //9
};
int error[4][6] = {
    {0, 0, 0, 0, 0, 0}, //hours tens
    {0, 0, 0, 0, 0, 0}, //hours ones
    {0, 0, 0, 0, 0, 0}, //minutes tens
    {0, 0, 0, 0, 0, 0} //minutes ones
};
byte prevHour;
byte prevMinute;
byte currHour;
byte currMinute;
byte digitsChanged;
bool prevForward;
bool forward;
DateTime now;

void setup() {
    while (!rtc.begin()) {
        delay(1000);
    }

    now = rtc.now();
    prevHour = (now.hour() <= 12) ? now.hour() : now.hour() - 12;
    prevMinute = now.minute();
    prevForward = true;
    
    Wire.begin();
    //pin A4 is SDL
    //pin A5 is SCK

    //Set all ports to output
    for (byte addr = 0; addr < 6; addr++) { //6 MCP23017 chips
        Wire.beginTransmission(0x20 + addr);
        Wire.write(0x00); // IODIRA register
        Wire.write(0x00); // set all of port A to outputs
        Wire.write(0x01); // IODIRB register
        Wire.write(0x00); // set all of port B to outputs
        Wire.endTransmission();
    }
}

void loop() {
    now = rtc.now();
    if (now.minute() != prevMinute){
        currHour = (now.hour() <= 12) ? now.hour() : now.hour() - 12;
        currMinute = now.minute();
        
        //Determine which digits to change
        digitsChanged = 0; //Stores binary representation of digits, starting with hour tens in the rightmost bit
        if (currHour/10 != prevHour/10) digitsChanged += 1;
        if (currHour%10 != prevHour%10) digitsChanged += (1 << 1);
        if (currMinute/10 != prevMinute/10) digitsChanged += (1 << 2);
        if (currMinute%10 != prevMinute%10) digitsChanged += (1 << 3);

        //Update error
        forward = (maxError(error, digitsChanged) <= 0);
        for (int i = 0; i < 6; i++){
            error[0][i] += (getDiff(forward, targetPositions[currHour/10][i], targetPositions[prevHour/10][i]));
            error[1][i] += (getDiff(forward, targetPositions[currHour%10][i], targetPositions[prevHour%10][i]));
            error[2][i] += (getDiff(forward, targetPositions[currMinute/10][i], targetPositions[prevMinute/10][i]));
            error[3][i] += (getDiff(forward, targetPositions[currMinute%10][i], targetPositions[prevMinute%10][i]));
        }

        //Pick up slack in clock mechanisms
        if (forward != prevForward){
            fixSlack(forward);
        }

        //Move hands
        

        prevHour = currHour;
        prevMinute = currMinute;
        prevForward = forward;
    }
    
    delay(1000);
}

void step(int numSteps) {
    //tens digit
    for (int addr = 0x20; addr <= 0x22; addr++) {

    }

    Wire.beginTransmission(0x20);
    Wire.write(0x14);
    Wire.write(0);
    Wire.endTransmission();
}

void portWrite(int addr, int port, int output) {
    Wire.beginTransmission(addr);
    Wire.write(0x14 + port); // OLATA / OLATB register
    Wire.write(output);
    Wire.endTransmission();
}

void fixSlack(){
    
}

int getDiff(bool forward, int current, int prev){
    if (forward){
        return current - prev >= 0 ? current - prev : current - prev + 96;
    } else {
        return current - prev <= 0 ? current - prev : current - prev - 96;
    }
}

int maxError(int** error, byte digitsChanged){
    int max = INT_MIN;
    byte i, j;
    for (i = 0; i <= 4; i++){
        if (digitsChanged & (1 << i) > 0){ //binary represents which digits are changing
            for (j = 0; j <= 6; j++){
                if (abs(error[i][j]) > abs(max)){
                    max = error[i][j];
                }
            }
        }
    }
    return max;
}

