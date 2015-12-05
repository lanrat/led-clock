#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <cstdio>
#include <sstream>
#include <thread>

#include "led-matrix.h"
#include "graphics.h"
#include "muni.h"
#include "weather.h"
#include "brightness.h"
#include "virtualcanvas.h"

rgb_matrix::Canvas *matrix;
rgb_matrix::Font font6x10;
rgb_matrix::Font font4x6;
auto red = rgb_matrix::Color(255, 0, 0);
auto blank = rgb_matrix::Color(0, 0, 0);

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
  static time_t rawtime;
  static struct tm * timeinfo;

  while (true)
  {
    // update var
    time (&rawtime);
    // convert to string info
    timeinfo = localtime(&rawtime);
    // format
    strftime(buffer,80,"%H:%M:%S",timeinfo);
    // print
    rgb_matrix::DrawText(matrix, font6x10, x, y + font6x10.baseline(), red, &blank, buffer);

    usleep(0.5 * 1000000);
  }
}

void updateCalendar(int x, int y) {
  static char buffer[80];
  static time_t rawtime;
  static struct tm * timeinfo;

  while (true)
  {
    // update var
    time (&rawtime);
    // convert to string info
    timeinfo = localtime(&rawtime);
    // format
    strftime(buffer,80,"%a",timeinfo);
    // print
    rgb_matrix::DrawText(matrix, font4x6, x, y + font4x6.baseline(), red, &blank, buffer);

    strftime(buffer,80,"%e",timeinfo);
    // print
    rgb_matrix::DrawText(matrix, font4x6, x + 13, y + font4x6.baseline(), red, &blank, buffer);

    // TODO sleeo until time to change date
    usleep(60 * 1000000);
  }
}

void updateWeather(int x, int y){
    weatherInit();
    while (true){
        const auto weather = weatherRun();
        rgb_matrix::DrawText(matrix, font6x10, x, y + font6x10.baseline(), red, &blank, weather.c_str());
        usleep(60 * 1000000);
    }
}

void updateMuni(int x, int y) {
  muniInit();
  std::ostringstream oss;

  while (true)
  {
    muniETA eta = muniRun();
    oss.str("");
    oss.clear();

    if (eta.N.size() > 0) {
      oss << "N" << eta.N[0];
      if (eta.N.size() > 1 && eta.N[1] < 60) {
        oss << "," << eta.N[1];
      }

    }
    if (eta.NX.size() > 0) {
      oss << " NX" << eta.NX[0];
      if (eta.NX.size() > 1 && eta.NX[1] < 60) {
        oss << "," << eta.NX[1];
      }
    }
    oss << "  ";
    // print
    rgb_matrix::DrawText(matrix, font4x6, x, y + font4x6.baseline(), red, &blank, oss.str().c_str()); 

    // TODO use epoch time to get moare acurate ETA with less requests
    usleep(60 * 1000000);
  }

  muniCleanup();
}

void updateBrightness()
{
  brightnessInit(27);
  unsigned char b;

  while (true) {
    b = brightnessGet();

    //printf("Updating brightness to %d\n", b);
    // TODO force redraw to adjust brightness
    // allow for redraw without calculations
    red = rgb_matrix::Color(b, 0, 0);

    sleep(10);
  }
}

void run(bool debug) {
  matrix->Clear();

  std::thread clockThread(updateClock, 0, 0);
  std::thread calendarThread(updateCalendar, 0, font6x10.height());
  std::thread muniThread(updateMuni, 32, font6x10.height());
  std::thread weatherThread(updateWeather, 50, 0);
  if (!debug){
      std::thread brightnessThread(updateBrightness);
  }

  while (true)
  {
      if (debug){
          ((VirtualCanvas*)matrix)->Show();
      }

    usleep(1000000);
  }

}

void showLoading()
{
  matrix->SetPixel(0, 0, 255, 0, 0);
  matrix->SetPixel(0, 1, 255, 0, 0);
  matrix->SetPixel(1, 0, 255, 0, 0);
  matrix->SetPixel(1, 1, 255, 0, 0);
}

int main(int argc, char *argv[]) {
    /*
     * Set up GPIO pins. This fails when not running as root.
     */
    const auto debug = argc >= 1 && std::string(argv[1]) == "debug";
    if (!debug) {
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
        //printf("r: %d, c: %d, p: %d\n", rows, chain, parallel);
    } else {
        matrix = new VirtualCanvas(16, 64);
    }
    printf("%dx%d=%d\n",  matrix->width(), matrix->height(), matrix->width() * matrix->height());

    showLoading();
    initFonts();
    printf("Fonts loaded\n");

    run(debug);

    delete matrix;
    return 0;
}
