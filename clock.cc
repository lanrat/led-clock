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
static bool debug = false;

#define BUFFER_SIZE 80 

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


static char clockBuffer[BUFFER_SIZE];
void updateClock() {
  static time_t rawtime;
  static struct tm * timeinfo;

  while (true) {
    // update var
    time (&rawtime);
    // convert to string info
    timeinfo = localtime(&rawtime);
    // format
    strftime(clockBuffer,BUFFER_SIZE,"%H:%M:%S",timeinfo);

    usleep(0.5 * 1000000);
  }
}

static char calendarBuffer[BUFFER_SIZE];
void updateCalendar() {
  static time_t rawtime;
  static struct tm * timeinfo;

  while (true) {
    // update var
    time (&rawtime);
    // convert to string info
    timeinfo = localtime(&rawtime);
    // format
    strftime(calendarBuffer,BUFFER_SIZE,"%a %e ",timeinfo);

    // TODO sleep until time to change date
    usleep(60 * 1000000);
  }
}

const static char * weatherBuffer;
void updateWeather(){
  weatherInit();
  while (true) {
    const auto weather = weatherRun();
    weatherBuffer = weather.c_str();
    usleep(10 * 60 * 1000000);
  }
}

const static char * muniBuffer;
void updateMuni() {
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
    muniBuffer = oss.str().c_str(); 

    // TODO use epoch time to get moare acurate ETA with less requests
    usleep(60 * 1000000);
  }

  muniCleanup();
}

void updateBrightness() {
  brightnessInit(13);
  unsigned char b;

  while (true) {
    b = brightnessGet();

    printf("Updating brightness to %d\n", b);
    // TODO force redraw to adjust brightness
    // allow for redraw without calculations
    red = rgb_matrix::Color(b, 0, 0);

    sleep(5);
  }
}


void renderClock(int x, int y) {
  rgb_matrix::DrawText(matrix, font6x10, x, y + font6x10.baseline(), red, &blank, clockBuffer);
}

void renderCalendar(int x, int y) {
  rgb_matrix::DrawText(matrix, font4x6, x, y + font4x6.baseline(), red, &blank, calendarBuffer);
}

void renderMuni(int x, int y) {
  rgb_matrix::DrawText(matrix, font4x6, x, y + font4x6.baseline(), red, &blank, muniBuffer); 
}

void renderWeather(int x, int y) {
  rgb_matrix::DrawText(matrix, font6x10, x, y + font6x10.baseline(), red, &blank, weatherBuffer);
}

void run() {
  std::thread clockThread(updateClock);
  std::thread calendarThread(updateCalendar);
  std::thread muniThread(updateMuni);
  std::thread weatherThread(updateWeather);
  if (!debug) {
    std::thread brightnessThread(updateBrightness);
    brightnessThread.detach();
  }
  sleep(5);

  matrix->Clear();
  while (true) {

    renderClock(0, 0);
    renderCalendar(0, font6x10.height());
    renderWeather(50, 0);
    renderMuni(32, font6x10.height());

    if (debug){
      ((VirtualCanvas*)matrix)->Show();
    }

    usleep(1.0 * 1000000);
  }

}

void showLoading(int l) {
  matrix->SetPixel(l, 0, 255, 0, 0);
  matrix->SetPixel(l, 1, 255, 0, 0);
  matrix->SetPixel(l+1, 0, 255, 0, 0);
  matrix->SetPixel(l+1, 1, 255, 0, 0);
}

int main(int argc, char *argv[]) {
  debug = argc > 1 && std::string(argv[1]) == "debug";
  if (!debug) {
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
    //printf("r: %d, c: %d, p: %d\n", rows, chain, parallel);
  } else {
    matrix = new VirtualCanvas(16, 64);
  }
  printf("%dx%d=%d\n",  matrix->width(), matrix->height(), matrix->width() * matrix->height());

  showLoading(0);
  initFonts();

  showLoading(4);
  run();

  delete matrix;
  return 0;
}
