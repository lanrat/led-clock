#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <cstdio>
#include <sstream>

#include "led-matrix.h"
#include "graphics.h"
#include "muni.h"


rgb_matrix::RGBMatrix *matrix;
rgb_matrix::Font font6x10;
rgb_matrix::Font font4x6;
auto red = rgb_matrix::Color(255, 0, 0);
auto blank = rgb_matrix::Color(0, 0, 0);


// clock vars
time_t rawtime;
struct tm * timeinfo;


static void initFonts() {
    if (!font6x10.LoadFont("matrix/fonts/6x10.bdf")) {
    fprintf(stderr, "Couldn't load font\n");
    return;
  }
    if (!font4x6.LoadFont("matrix/fonts/4x6.bdf")) {
    fprintf(stderr, "Couldn't load font\n");
    return;
  }
}

void updateClock(int x, int y) {
  static char buffer[80];
  // update var
  time (&rawtime);
  // convert to string
  timeinfo = localtime(&rawtime);
  // format
  strftime(buffer,80,"%H:%M",timeinfo);
  // print
  rgb_matrix::DrawText(matrix, font6x10, x, y + font6x10.baseline(), red, &blank, buffer);
}

void updateCalendar(int x, int y) {
  static char buffer[80];
  // format
  strftime(buffer,80,"%a",timeinfo);
  // print
  rgb_matrix::DrawText(matrix, font4x6, x, y + font4x6.baseline(), red, &blank, buffer);

  strftime(buffer,80,"%e/%m",timeinfo);
  // print
  rgb_matrix::DrawText(matrix, font4x6, x + 13, y + font4x6.baseline(), red, &blank, buffer); 
}

void updateMuni(int x, int y) {
  muniETA eta = muniRun();
  std::ostringstream oss;

  if (eta.N.size() > 0) {
    oss << "N" << eta.N[0];
    if (eta.N.size() > 1) {
      oss << "," << eta.N[1];
    }

  }
  if (eta.NX.size() > 0) {
    oss << " NX" << eta.NX[0];
    if (eta.NX.size() > 1) {
      oss << "," << eta.NX[1];
    }
  }
  // print
  rgb_matrix::DrawText(matrix, font4x6, x, y + font4x6.baseline(), red, &blank, oss.str().c_str()); 
}

 // look at http://www.riosscheduler.org/ for scheduler example?


void run() {
  for (;;) {
    updateClock(0,0);

    updateCalendar(0,font6x10.height());

    updateMuni(34,font6x10.height());

    usleep(5 * 1000000);
  }
}

int main(int argc, char *argv[]) {
    /*
     * Set up GPIO pins. This fails when not running as root.
     */
    rgb_matrix::GPIO io;
    if (!io.Init()) {
      fprintf(stderr, "Unable to init GPIO\n");
      return 1;
    }

    /*
     * Set up the RGBMatrix. It implements a 'Canvas' interface.
     */
    int rows = 16;    // A 32x32 display. Use 16 when this is a 16x32 display.
    int chain = 2;    // Number of boards chained together.
    int parallel = 1; // Number of chains (must be 1 on origional Pi)
    matrix = new rgb_matrix::RGBMatrix(&io, rows, chain, parallel);
    
    printf("r: %d, c: %d, p: %d\n", rows, chain, parallel);
    printf("w: %d, h: %d, T: %d\n",  matrix->width(), matrix->height(), matrix->width() * matrix->height());

    initFonts();
    printf("Fonts loaded\n");

    muniInit();

    run();

    muniCleanup();

    delete matrix;
    return 0;
}
