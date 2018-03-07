#include <unistd.h>
#include <cmath>
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
rgb_matrix::FrameCanvas *canvas;

unsigned char brightness = 128;
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
  weatherCleanup();
}

bandwidth bw;
void updateBandwidth() {
  bandwidthInit();

  while (true) {
    bw = bandwidthRun();
    //printf("D:%u U:%u\n", bw.down, bw.up);
    sleep(1);
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
    // get time sample
    if (a < 10) {
      // bright
      brightness = 128;
    }else if (a < 100) {
      // medium
      brightness = 64;
    }else {
      // dim
      brightness = 32;
    }

    red = rgb_matrix::Color(brightness, 0, 0);

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
  rgb_matrix::DrawText(canvas, mainFont, x, y + mainFont.baseline(), red, NULL, buffer);

  // date format
  strftime(buffer,BUFFER_SIZE,"%a%e",timeinfo);
  rgb_matrix::DrawText(canvas, mainFont, x + 35, y + mainFont.baseline(), red, NULL, buffer);
}

void renderMuni(int x, int y) {
  static char buffer[BUFFER_SIZE];
  static time_t now;
  static const int displayNum = 3;
  unsigned int i;
  time(&now);

  // skip past times and 0 min
  for (i = 0; i < eta.size(); i++) {
    // add 30 secconds to now to reduce 0min etas
    if (now < (eta[i].eta - 30)) {
      break;
    }
  }

  for (int j = 0; (j < displayNum) && (j + i) < eta.size();  j++) {
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

void renderWeather(int x, int y) {
  weatherFont.DrawGlyph(canvas, x, y + weatherFont.baseline(), red, NULL, weatherCode);
}

// TODO could make bandwith iterate over each bar column (thickness)
void renderBandwidth(int x, int y) {
  //converting to megabits
  //u_int dm = (bw.down / 1000000) * 8);

  // scale
  u_int d = log2(bw.down * 8)-7; // convert to bps

  // draw bandwith on a scape of 0-32 (bars 8 high by 4 wide)
  for (u_int w = 0; w < 4; w++) {
    for (u_int h = 0; h < 8; h++) {
      if (d > 0) {
        canvas->SetPixel(x+w, y+8-h, brightness, 0, 0);
        d--;
      }
    }
  }

  // scale
  u_int u = log2(bw.up * 8)-7; // convert to bps

  // draw bandwith on a scape of 0-32 (bars 8 high by 4 wide)
  for (u_int w = 0; w < 4; w++) {
    for (u_int h = 0; h < 8; h++) {
      if (u > 0) {
        canvas->SetPixel(x+w+5, y+8-h, brightness, 0, 0);
        u--;
      }
    }
  }
}

void updateRecieved(char * out) {
  strncpy(updateString, out, SERVER_BUFFER_SIZE);
  updateString[SERVER_BUFFER_SIZE-1] = 0;
  newUpdate = 10;
}

void renderUpdate() {
  size_t len = strlen(updateString);
  int mw = matrix->width();
  int dw = updateFont.CharacterWidth('A') * len;
  if (len <= 7) {
    // center text
    int c = (mw - dw) / 2;
    rgb_matrix::DrawText(canvas, updateFont, c, 1 + updateFont.baseline(), red, NULL, updateString);
    canvas = matrix->SwapOnVSync(canvas);
    canvas->Clear();
    sleep(newUpdate);
  } else {
    // scroll text
    unsigned int slept = 0;
    static const int stepDuration = (0.2 * 1000000 / 8);
    while (slept < (newUpdate * 1000000)) {
      for (int i=0;  i < mw+dw; i++) {
        rgb_matrix::DrawText(canvas, updateFont, mw-i, 1 + updateFont.baseline(), red, NULL, updateString);
        canvas = matrix->SwapOnVSync(canvas);
        canvas->Clear();
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

  canvas = matrix->CreateFrameCanvas();
  while (true) {
    if (newUpdate) {
      renderUpdate();
      newUpdate = 0;
    }
    renderClock(0, -1);
    renderWeather(0, 8);
    renderMuni(9, 8);
    renderBandwidth(54, 8); // shift left 2
    canvas = matrix->SwapOnVSync(canvas);
    canvas->Clear();

    usleep(0.3 * 1000000);
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
  } else {
    // TODO make virtul canvas extend RGBMatrix (or other calss with CreateFrameCanvas and SwapOnVSync)
    //matrix = new VirtualCanvas(16, 64);
  }
  printf("Matrix %dx%d=%d\n",  matrix->width(), matrix->height(), matrix->width() * matrix->height());

  showLoading(0);
  initFonts();

  showLoading(4);
  run();

  delete matrix;
  return 0;
}
