#include <cstdlib>
#include "virtualcanvas.h"
#include <iostream>

using namespace rgb_matrix;
using namespace std;

VirtualCanvas::VirtualCanvas(int rows, int cols): rows(rows), cols(cols) {
    buffer = (char*)malloc(rows * cols);
}

void VirtualCanvas::SetPixel(int x, int y,
                                     uint8_t red, uint8_t green, uint8_t blue){
    const auto index = y * cols + x;
    if (index < rows * cols) {
        buffer[index] = red;
    }
}
int VirtualCanvas::width() const { return cols; }
int VirtualCanvas::height() const { return rows; }

void VirtualCanvas::Show(){
    for(auto y = 0; y < rows; y++) {
        for(auto x = 0; x < cols; x++){
            const auto index = y * cols + x;
            if(buffer[index] == 0) {
                cout << " ";
            } else{
                cout << "\033[0;31m*\033[0m";
            }
        }
        cout << "\n";
    }
}

void VirtualCanvas::Clear(){
    Fill(0, 0, 0);
}

void VirtualCanvas::Fill(uint8_t red, uint8_t green, uint8_t blue){
    for(auto i = 0; i < rows * cols; i++) {
        buffer[i] = red;
    }
}
