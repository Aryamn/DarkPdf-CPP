#pragma once
#include <cairo.h>
#include <map>
#include "color_scheme.h"

// using static because this class should be singleton
class ColorInverter {
public:
    static void invertColors(cairo_surface_t* surface, const ColorScheme& scheme);
    
    // Public methods for testing
    static RGB mapToScheme(const RGB& original, const ColorScheme& scheme);
    static double calculateLuminosity(const RGB& color);
    static RGB interpolateColors(const RGB& color1, const RGB& color2, double factor);
    
private:
    static inline std::map<RGB,std::tuple<double, double, double>> rgbToHsvCache; // Cache for RGB to HSV conversions
    static inline std::map<std::tuple<double, double, double>,RGB> hsvToRgbCache; // Cache for HSV to RGB conversions
    static void invertPixel(unsigned char* pixel, const ColorScheme& scheme);
    
    static double getLuminance(unsigned char r, unsigned char g, unsigned char b);
    static double getSaturation(unsigned char r, unsigned char g, unsigned char b);

    static void rgbToHsv(unsigned char r, unsigned char g, unsigned char b, 
                        double& h, double& s, double& v);
    static void hsvToRgb(double h, double s, double v, 
                        unsigned char& r, unsigned char& g, unsigned char& b);
};
