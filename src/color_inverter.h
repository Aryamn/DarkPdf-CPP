#pragma once
#include <cairo.h>
#include "color_scheme.h"

// using static because this class should be singleton
class ColorInverter {
public:
    static void invertColors(cairo_surface_t* surface, const ColorScheme& scheme);
    
private:
    static void invertPixel(unsigned char* pixel, const ColorScheme& scheme);
    static RGB mapToScheme(const RGB& original, const ColorScheme& scheme);
    static double calculateLuminosity(const RGB& color);
    static RGB interpolateColors(const RGB& color1, const RGB& color2, double factor);
    
    static double getLuminance(unsigned char r, unsigned char g, unsigned char b);
    static double getSaturation(unsigned char r, unsigned char g, unsigned char b);

    static void rgbToHsv(unsigned char r, unsigned char g, unsigned char b, 
                        double& h, double& s, double& v);
    static void hsvToRgb(double h, double s, double v, 
                        unsigned char& r, unsigned char& g, unsigned char& b);
};
