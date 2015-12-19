#include <wiringPi.h>
#include <unistd.h>
#include <stdio.h>
#include "brightness.h"

int main(void)
{
    brightnessInit(13);
    for (;;)
    {
        printf("%d\n", brightnessGet());
        sleep(1);
    }
}