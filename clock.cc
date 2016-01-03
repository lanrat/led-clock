#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <cstdio>
#include <thread>
#include "led-matrix.h"
#include "graphics.h"
#include "muni.h"
#include "weather.h"
#include "brightness.h"
#include "virtualcanvas.h"
#include "server.h"
#include "bandwidth.h"

rgb_matrix::RGBMatrix *matrix;
rgb_matrix::Font mainFont;
rgb_matrix::Font updateFont;
rgb_matrix::Font weatherFont;

unsigned char brightness = 255;
auto red = rgb_matrix::Color(brightness, 0, 0);
static bool debug = false;
char updateString[SERVER_BUFFER_SIZE];
static unsigned int newUpdate = 0;

#define BUFFER_SIZE 80 

static void initFonts() {
  if (!mainFont.LoadFont("matrix/fonts/6x10.bdf")) {
    fprintf(stderr, "Couldn't load font\n");
    exit(1);
  }
  if (!updateFont.LoadFont("matrix/fonts/9x15B.bdf")) {
    fprintf(stderr, "Couldn't load font\n");
    exit(1);
  }
  if (!weatherFont.LoadFont("weather.bdf")) {
    fprintf(stderr, "Couldn't load font\n");
    exit(1);
  }
}

static int weatherCode = 255;
void updateWeather(){
  weatherInit();
  while (true) {
    weatherCode = weatherRun();
    usleep(10 * 60 * 1000000);
  }
}

bandwidth bw;
void updateBandwidth() {
  bandwidthInit();

  while (true) {
    bw = bandwidthRun();
    //printf("D:%u U:%u\n", bw.down, bw.up);
    sleep(2);
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
  double a;

  while (true) {
    a = brightnessGet();
    if (a < 30) {
      brightness = 200;
    }else if (a < 100) {
      brightness = 128;
    }else {
      brightness = 64;
    }

    red = rgb_matrix::Color(brightness, 0, 0);

    sleep(5);
  }
}


void renderClock(rgb_matrix::FrameCanvas * canvas, int x, int y) {
  static char buffer[BUFFER_SIZE];
  static time_t now;
  static struct tm * timeinfo;

  // update var
  time(&now);
  // convert to string info
  timeinfo = localtime(&now);

  // clock format
  strftime(buffer,BUFFER_SIZE,"%H:%M",timeinfo);
  rgb_matrix::DrawText(canvas, mainFont, x, y + mainFont.baseline(), red, NULL, buffer);

  // date format
  strftime(buffer,BUFFER_SIZE,"%a%e",timeinfo);
  rgb_matrix::DrawText(canvas, mainFont, x + 35, y + mainFont.baseline(), red, NULL, buffer);
}

void renderMuni(rgb_matrix::FrameCanvas * canvas, int x, int y) {
  static char buffer[BUFFER_SIZE];
  static time_t now;
  unsigned int i;
  time(&now);

  // skip past times and 0 min
  for (i = 0; i < eta.size(); i++) {
    // add 30 secconds to now to reduce 0min etas
    if (now < (eta[i].eta - 30)) {
      break;
    }
  }

  for (int j = 0; (j < 3) && (j + i) < eta.size();  j++) {
    // draw symbol
    int h = mainFont.height();
    for (int k = 0; k < h; k++) {
      if (k < eta[i+j].route) {
        canvas->SetPixel(x, y+k+1, brightness/2, 0, 0);
      }
    }
    // draw eta
    snprintf(buffer, BUFFER_SIZE, "%ld", (eta[i+j].eta - now) / 60);
    x = x + 3 + rgb_matrix::DrawText(canvas, mainFont, x+1, y + mainFont.baseline(), red, NULL, buffer);
  }
}

void renderWeather(rgb_matrix::FrameCanvas * canvas, int x, int y) {
  weatherFont.DrawGlyph(canvas, x, y + weatherFont.baseline(), red, NULL, weatherCode);
}

// TODO more work here
// TODO use log scale?
void renderBandwidth(rgb_matrix::FrameCanvas * canvas, int x, int y) {
  //down
  u_int dm = bw.down / 1000000;
  for (u_int i = 0; i < 8; i ++) {
    if (dm > i) {
      canvas->SetPixel(x+1, y+8-i, brightness, 0, 0);
      canvas->SetPixel(x+2, y+8-i, brightness, 0, 0);
    }
  }

  // up
  u_int um = bw.up / 1000000;
  for (u_int i = 0; i < 8; i ++) {
    if (um > i) {
      canvas->SetPixel(x+4, y+8-i, brightness, 0, 0);
      canvas->SetPixel(x+5, y+8-i, brightness, 0, 0);
    }
  }
}

void updateRecieved(char * out) {
  strncpy(updateString, out, SERVER_BUFFER_SIZE);
  updateString[SERVER_BUFFER_SIZE-1] = 0;
  newUpdate = 10;
}

void renderUpdate() {
  rgb_matrix::FrameCanvas *canvas;
  size_t len = strlen(updateString);
  int mw = matrix->width();
  int dw = updateFont.CharacterWidth('A') * len;
  if (len <= 7) {
    // center text
    int c = (mw - dw) / 2;
    canvas = matrix->CreateFrameCanvas();
    rgb_matrix::DrawText(canvas, updateFont, c, 1 + updateFont.baseline(), red, NULL, updateString);
    matrix->SwapOnVSync(canvas);
    sleep(newUpdate);
  } else {
    // scroll text
    unsigned int slept = 0;
    static const int stepDuration = (0.2 * 1000000 / 8);
    while (slept < (newUpdate * 1000000)) {
      for (int i=0;  i < mw+dw; i++) {
        canvas = matrix->CreateFrameCanvas();
        rgb_matrix::DrawText(canvas, updateFont, mw-i, 1 + updateFont.baseline(), red, NULL, updateString);
        matrix->SwapOnVSync(canvas);
        usleep(stepDuration);
        slept = slept + stepDuration;
      }
    }
  }
}

void run() {
  std::thread muniThread(updateMuni);
  std::thread weatherThread(updateWeather);
  std::thread bandwidthThread(updateBandwidth);
  std::thread serverThread(setupServer, updateRecieved);
  if (!debug) {
    std::thread brightnessThread(updateBrightness);
    brightnessThread.detach();
  }

  rgb_matrix::FrameCanvas *canvas;
  while (true) {
    
    if (newUpdate) {
      renderUpdate();
      newUpdate = 0;
    }

    canvas = matrix->CreateFrameCanvas();
    renderClock(canvas, 0, -1);
    renderWeather(canvas, 0, 8);
    renderMuni(canvas, 9, 8);
    renderBandwidth(canvas, 56, 8);
    matrix->SwapOnVSync(canvas);

    usleep(0.5 * 1000000);
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
    // TODO make virtul canvas extend RGBMatrix (or other calss with CreateFrameCanvas and SwapOnVSync)
    //matrix = new VirtualCanvas(16, 64);
    printf("%dx%d=%d\n",  matrix->width(), matrix->height(), matrix->width() * matrix->height());
  }

  showLoading(0);
  initFonts();

  showLoading(4);
  run();

  delete matrix;
  return 0;
}
