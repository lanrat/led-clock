#include "led-matrix.h"

class VirtualCanvas : public rgb_matrix::Canvas {

    int rows;
    int cols;

    char* buffer;
    
    static void Run(VirtualCanvas* canvas, useconds_t wait);

public:
    VirtualCanvas(int rows, int cols, useconds_t wait = 1000000);

    virtual void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);

    // Clear screen to be all black.
    virtual void Clear();

    // Fill screen with given 24bpp color.
    virtual void Fill(uint8_t red, uint8_t green, uint8_t blue);

    virtual int width() const;
    virtual int height() const;

    void Show();
};
