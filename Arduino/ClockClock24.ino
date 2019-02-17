#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <Wire.h>
#include "RTClib.h"

//Number of 4-step cycles in each 1/8 clock 
const int eigthCycles = 55;

//Red gear: 13 teeth
//Middle gear: 45 teeth outside, 10 teeth inside
//Minute hand: 15 teeth
//Hour hand: 40 teeth
//(13/15) * 64 4-step cycles per 1/8 revolution ~= 55.467 steps

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
int change[4][6] = {
    {0, 0, 0, 0, 0, 0}, //hours tens
    {0, 0, 0, 0, 0, 0}, //hours ones
    {0, 0, 0, 0, 0, 0}, //minutes tens
    {0, 0, 0, 0, 0, 0} //minutes ones
}
byte prevHour;
byte prevMinute;
byte currHour;
byte currMinute;
byte digitsChanged;
bool prevForward;
bool forward;
RTC_DS3231 rtc;
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
        Wire.endTransmission();

        Wire.beginTransmission(0x20 + addr);
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

        //Determine direction of movement
        forward = (maxError(error, digitsChanged) <= 0);
        
        //Calculate change and update error
        for (byte i = 0; i < 6; i++){
            change[0][i] = (getDiff(targetPositions[currHour/10][i], targetPositions[prevHour/10][i]));
            error[0][i] += change[0][i];
            change[0][i] = abs(change[0][i]) * eigthCycles;
            
            change[1][i] = (getDiff(targetPositions[currHour%10][i], targetPositions[prevHour%10][i]));
            error[1][i] += change[1][i];
            change[1][i] = abs(change[1][i]) * eigthCycles;
            
            change[2][i] = (getDiff(targetPositions[currMinute/10][i], targetPositions[prevMinute/10][i]));
            error[2][i] += change[2][i];
            change[2][i] = abs(change[2][i]) * eigthCycles;
            
            change[3][i] = (getDiff(targetPositions[currMinute%10][i], targetPositions[prevMinute%10][i]));
            error[3][i] += change[3][i];
            change[3][i] = abs(change[3][i]) * eigthCycles;
        }

        //Move hands
        if (forward){
            for (int cycle = maxCount(change); cycle > 0; cycle--){
                for (byte digit = 0; digit < 4; digit++){
                    for (byte board = 0; board < 3; board++){
                        byte bitmask = 0;
                        if (change[digit][2*board] <= cycle) bitmask = bitmask | 0b00001111;
                        if (change[digit][2*board+1] <= cycle) bitmask = bitmask | 0b11110000;
                        if (bitmask != 0){ // complete one cycle
                            for (int i = 0; i < 4; i++){
                                portWrite(0x20 + 3*(digit/2) + board, 0x14 + (digit%2), bitmask & (0b00010001 << i));
                            }
                        }
                    }
                }
                delay(5);
            }
        } else {
            for (int cycle = maxCount(change); cycle > 0; cycle--){
                for (byte digit = 0; digit < 4; digit++){
                    for (byte board = 0; board < 3; board++){
                        byte bitmask = 0;
                        if (change[digit][2*board] <= cycle) bitmask = bitmask | 0b00001111;
                        if (change[digit][2*board+1] <= cycle) bitmask = bitmask | 0b11110000;
                        if (bitmask != 0){ // complete one cycle
                            for (int i = 0; i < 4; i++){
                                portWrite(0x20 + 3*(digit/2) + board, 0x14 + (digit%2), bitmask & (0b10001000 >> i));
                            }
                        }
                    }
                }
                delay(5);
            }
        }
        

        prevHour = currHour;
        prevMinute = currMinute;
        prevForward = forward;
    }
    
    delay(1000);
}

void portWrite(byte hardwareAddr, byte registerAddr, byte data) {
    Wire.beginTransmission(hardwareAddr);
    Wire.write(registerAddr); 
    Wire.write(data);
    Wire.endTransmission();
}

int getDiff(int current, int prev){
    if (forward){
        return current - prev >= 0 ? current - prev : current - prev + 96;
    } else {
        return current - prev <= 0 ? current - prev : current - prev - 96;
    }
}

int maxError(int** error, byte digitsChanged){
    int max = INT_MIN;
    byte i, j;
    for (i = 0; i < 4; i++){
        if (digitsChanged & (1 << i) > 0){ //binary represents which digits are changing
            for (j = 0; j < 6; j++){
                if (abs(error[i][j]) > abs(max)){
                    max = error[i][j];
                }
            }
        }
    }
    return max;
}

int maxCount(int** count){
    int max = INT_MIN;
    for (byte i = 0; i < 4; i++){
        for (byte j = 0; j < 6; j++){
            if (count[i][j] > max){
                max = count[i][j];
            }
        }
    }
    return max;
}

