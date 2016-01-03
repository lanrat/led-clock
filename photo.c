#include <wiringPi.h>
#include <unistd.h>
#include <stdio.h>
#include "brightness.h"

int main(void)
{
    brightnessInit(13);
    for (;;)
    {
        printf("%lf\n", brightnessGet());
        sleep(1);
    }
}