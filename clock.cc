#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <cstdio>
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
    exit(1);
  }
  if (!font4x6.LoadFont("matrix/fonts/4x6.bdf")) {
    fprintf(stderr, "Couldn't load font\n");
    exit(1);
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

static muniETA eta;
void updateMuni() {
  muniInit();

  while (true) {
    eta = muniRun();
    usleep(3 * 60 * 1000000);
  }

  muniCleanup();
}

void updateBrightness() {
  brightnessInit(13);
  unsigned char b;

  while (true) {
    b = brightnessGet();

    //printf("Updating brightness to %d\n", b);
    red = rgb_matrix::Color(b, 0, 0);

    sleep(5);
  }
}


void renderClock(int x, int y) {
  static char buffer[BUFFER_SIZE];
  static time_t now;
  static struct tm * timeinfo;

  // update var
  time(&now);
  // convert to string info
  timeinfo = localtime(&now);

  // clock format
  strftime(buffer,BUFFER_SIZE,"%H:%M",timeinfo);
  rgb_matrix::DrawText(matrix, font6x10, x, y + font6x10.baseline(), red, &blank, buffer);

  // date format
  strftime(buffer,BUFFER_SIZE,"%a%e",timeinfo);
  rgb_matrix::DrawText(matrix, font6x10, x + 35, y + font6x10.baseline(), red, &blank, buffer);
}

void renderMuni(int x, int y) {
  static char buffer[BUFFER_SIZE];
  static time_t now;
  unsigned int i;
  time(&now);
  //N
  for (i = 0; i < eta.N.size(); i++) {
    if (now < eta.N[i]) {
      break;
    }
  }
  // TODO erase text when blank
  if (eta.N.size() > i) {
    if ((eta.N.size() > i+1) && (eta.N[i+1] - now < (60 * 60))) {
      snprintf(buffer, BUFFER_SIZE, "N%ld %ld          ", (eta.N[i] - now) / 60, (eta.N[i+1] - now) / 60);
    }else {
      snprintf(buffer, BUFFER_SIZE, "N%ld             ", (eta.N[i] - now) / 60);
    }
    rgb_matrix::DrawText(matrix, font4x6, x, y + font4x6.baseline(), red, &blank, buffer);
  }
  // NX
  for (i = 0; i < eta.NX.size(); i++) {
    if (now < eta.NX[i]) {
      break;
    }
  }
  if (eta.NX.size() > i) {
    if ((eta.NX.size() > i+1) && (eta.NX[i+1] - now < (60 * 60))) {
      snprintf(buffer, BUFFER_SIZE, "NX%ld %ld ", (eta.NX[i] - now) / 60, (eta.NX[i+1] - now) / 60);
    }else {
      snprintf(buffer, BUFFER_SIZE, "NX%ld ", (eta.NX[i] - now) / 60);
    }
  rgb_matrix::DrawText(matrix, font4x6, x + 20, y + font4x6.baseline(), red, &blank, buffer); 
  }
}

void renderWeather(int x, int y) {
  if (weatherBuffer) {
    rgb_matrix::DrawText(matrix, font6x10, x, y + font6x10.baseline(), red, &blank, weatherBuffer);
  }
}

void run() {
  std::thread muniThread(updateMuni);
  std::thread weatherThread(updateWeather);
  if (!debug) {
    std::thread brightnessThread(updateBrightness);
    brightnessThread.detach();
  }

  matrix->Clear();
  while (true) {

    renderClock(0, -1);
    renderWeather(0, 8);
    renderMuni(16, 10);

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
    printf("%dx%d=%d\n",  matrix->width(), matrix->height(), matrix->width() * matrix->height());
  }

  showLoading(0);
  initFonts();

  showLoading(4);
  run();

  delete matrix;
  return 0;
}
