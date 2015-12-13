#include <wiringPi.h>
#include <unistd.h>
#include "brightness.h"

static int pin;

//static int samples[SAMPLE_SIZE];
// ref:  https://learn.adafruit.com/basic-resistor-sensor-reading-on-raspberry-pi/basic-photocell-reading

void brightnessInit(int p) {
    pin = p;
    wiringPiSetupPhys();
}

int brightnessSample() {
    int reading = 0;
    
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    usleep(0.1 * 1000000);

    pinMode(pin, INPUT);

    while (digitalRead(pin) == LOW) {
        reading++;
        if (reading > 300000) {
            break;
        }
    }

    return reading;
}

unsigned char brightnessGet() {
    int total = 0;
    for (int i = 0; i < SAMPLE_SIZE;  i++) {
        //samples[i] = brightnessSample();
        total = total + brightnessSample();
    }
    int avg = total / SAMPLE_SIZE;

    if (avg > 200000) {
        return 64;
    }else if (avg > 50000) {
        return 128;
    }
    return 255;

    // bright ~= 400
    // dark ~= 600000
    //return 255 - ((avg * 255) / 500000);
}

