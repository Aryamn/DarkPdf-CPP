#pragma once
#include <cairo.h>

// using static because this class should be singleton
class ColorInverter {
public:
    static void invertColors(cairo_surface_t* surface);
    static void invertPixel(unsigned char* pixel);
    
private:
    static double getLuminance(unsigned char r, unsigned char g, unsigned char b);
    static double getSaturation(unsigned char r, unsigned char g, unsigned char b);

    static void rgbToHsv(unsigned char r, unsigned char g, unsigned char b, 
                        double& h, double& s, double& v);
    static void hsvToRgb(double h, double s, double v, 
                        unsigned char& r, unsigned char& g, unsigned char& b);
};
