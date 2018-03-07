#include <wiringPi.h>
#include <unistd.h>
#include <sys/time.h>
#include "brightness.h"

static int pin;

//static int samples[SAMPLE_SIZE];
// ref:  https://learn.adafruit.com/basic-resistor-sensor-reading-on-raspberry-pi/basic-photocell-reading

void brightnessInit(int p) {
    pin = p;
    wiringPiSetupPhys();
}

double brightnessSample() {
    struct timeval t1, t2;
    double elapsedTime;
    
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    usleep(0.1 * 1000000);

    pinMode(pin, INPUT);

    // start timer
    gettimeofday(&t1, NULL);

    while (digitalRead(pin) == LOW) {
        usleep(100);
        gettimeofday(&t2, NULL);
        // if its been more than 1s stop
        if ((t2.tv_sec - t1.tv_sec) > BRIGHTNESS_MAX_TIME ) {
            break;
        }
        continue;
    }
    
    // stop timer
    gettimeofday(&t2, NULL);\

    // compute the elapsed time in millisec
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms

    return elapsedTime;
}

double brightnessGet() {
    double total = 0;
    for (int i = 0; i < BRIGHTNESS_SAMPLE_SIZE;  i++) {
        //samples[i] = brightnessSample();
        total = total + brightnessSample();
        usleep(100);
    }
    double avg = total / BRIGHTNESS_SAMPLE_SIZE;

    return avg;
}

