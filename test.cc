
#include <unistd.h>
#include <ctime>
#include <cstdio>

#include "led-matrix.h"
#include "graphics.h"


using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

int main(int argc, char *argv[]) {
    /*
     * Set up GPIO pins. This fails when not running as root.
     */
     GPIO io;
     if (!io.Init())
      return 1;

    /*
     * Set up the RGBMatrix. It implements a 'Canvas' interface.
     */
    int rows = 16;    // A 32x32 display. Use 16 when this is a 16x32 display.
    int chain = 2;    // Number of boards chained together.
    int parallel = 1; // Number of chains (must be 1 on origional Pi)
    RGBMatrix *matrix = new RGBMatrix(&io, rows, chain, parallel);

    printf("r: %d, c: %d, p: %d\n", rows, chain, parallel);
    printf("w: %d, h: %d, T: %d\n",  matrix->width(), matrix->height(), matrix->width() * matrix->height());

    rgb_matrix::Font font;
    if (!font.LoadFont("matrix/fonts/8x13.bdf")) {
      fprintf(stderr, "Couldn't load font\n");
      return 1;
    }
    printf("font laoded\n");

    rgb_matrix::Color red(255, 0, 0);
    rgb_matrix::DrawText(matrix, font, 0, 0 + font.baseline(), red, "1234567890"); 

    usleep(5 * 1000000);

    delete matrix;
    return 0;
  }
