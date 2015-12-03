#include <wiringPi.h>
#include <unistd.h>
#include <stdio.h>

int photo(int pin)
{
    int reading = 0;
    
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    usleep(0.1 * 1000000);

    pinMode(pin, INPUT);

    while (digitalRead(pin) == LOW)
    {
        reading++;
    }

    return reading;
}

int main(void)
{
    wiringPiSetupGpio() ;
    for (;;)
    {
        printf("%d\n", photo(27));
    }
}