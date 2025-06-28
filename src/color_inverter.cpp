#include "color_inverter.h"
#include <algorithm>
#include <cmath>

// Color inversion constants
namespace ColorConstants {
    // Color detection thresholds
    constexpr double SATURATION_THRESHOLD = 0.1;          // Threshold for detecting colored vs grayscale content
    constexpr double LUMINANCE_THRESHOLD = 200.0;         // Threshold for detecting dark vs light content
    
    // Color adjustment factors
    constexpr double BRIGHTNESS_BOOST_BASE = 0.7;         // Base brightness for colored text in dark mode
    constexpr double BRIGHTNESS_BOOST_FACTOR = 0.3;       // Factor for original brightness contribution
    constexpr double SATURATION_REDUCTION = 0.8;          // Factor to reduce saturation for colored text
    
    // RGB and color space constants
    constexpr double RGB_LUMINANCE_R = 0.299;             // Red component weight for luminance calculation
    constexpr double RGB_LUMINANCE_G = 0.587;             // Green component weight for luminance calculation
    constexpr double RGB_LUMINANCE_B = 0.114;             // Blue component weight for luminance calculation
    constexpr double RGB_SCALE = 255.0;                   // RGB scale factor
    constexpr unsigned char RGB_MAX = 255;                // Maximum RGB value
    
    // HSV color space constants
    constexpr double HUE_CIRCLE_DEGREES = 360.0;          // Degrees in a color circle
    constexpr double HUE_SECTOR_SIZE = 60.0;              // Size of each hue sector in degrees
    constexpr double HSV_LIGHTNESS_FACTOR = 2.0;          // Factor for HSV lightness calculation
    constexpr double HSV_SATURATION_DIVISOR = 2.0;        // Divisor for HSV saturation adjustment
    constexpr double HSV_HUE_MODULO = 2.0;                // Modulo factor for hue calculation
    
    // Pixel format constants
    constexpr int PIXEL_STRIDE = 4;                       // Bytes per pixel in BGRA format
    constexpr int ALPHA_CHANNEL_OFFSET = 3;               // Alpha channel position in BGRA
    constexpr unsigned char TRANSPARENT_ALPHA = 0;        // Fully transparent alpha value
    
    // HSV hue range boundaries
    constexpr double HUE_RANGE_1_MIN = 0.0;               // First hue range minimum
    constexpr double HUE_RANGE_1_MAX = 60.0;              // First hue range maximum
    constexpr double HUE_RANGE_2_MIN = 60.0;              // Second hue range minimum
    constexpr double HUE_RANGE_2_MAX = 120.0;             // Second hue range maximum
    constexpr double HUE_RANGE_3_MIN = 120.0;             // Third hue range minimum
    constexpr double HUE_RANGE_3_MAX = 180.0;             // Third hue range maximum
    constexpr double HUE_RANGE_4_MIN = 180.0;             // Fourth hue range minimum
    constexpr double HUE_RANGE_4_MAX = 240.0;             // Fourth hue range maximum
    constexpr double HUE_RANGE_5_MIN = 240.0;             // Fifth hue range minimum
    constexpr double HUE_RANGE_5_MAX = 300.0;             // Fifth hue range maximum
}

void ColorInverter::invertColors(cairo_surface_t* surface) {
    if (!surface) return;
    
    cairo_surface_flush(surface);
    
    unsigned char* data = cairo_image_surface_get_data(surface);
    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    int stride = cairo_image_surface_get_stride(surface);
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char* pixel = data + y * stride + x * ColorConstants::PIXEL_STRIDE;
            invertPixel(pixel);
        }
    }
    
    cairo_surface_mark_dirty(surface);
}

void ColorInverter::invertPixel(unsigned char* pixel) {
    // Cairo uses BGRA format
    unsigned char b = pixel[0];
    unsigned char g = pixel[1];
    unsigned char r = pixel[2];
    unsigned char a = pixel[ColorConstants::ALPHA_CHANNEL_OFFSET];

    // Skip transparent pixels
    if (a == ColorConstants::TRANSPARENT_ALPHA) return;

    double luminance = getLuminance(r, g, b);
    double saturation = getSaturation(r, g, b);

    if(saturation > ColorConstants::SATURATION_THRESHOLD && luminance < ColorConstants::LUMINANCE_THRESHOLD) 
    {
        double h, s, v;
        rgbToHsv(r, g, b, h, s, v);

        v = ColorConstants::BRIGHTNESS_BOOST_BASE + (v * ColorConstants::BRIGHTNESS_BOOST_FACTOR);  // Make colored text brighter
        s = s * ColorConstants::SATURATION_REDUCTION;

        unsigned char newR, newG, newB;
        hsvToRgb(h, s, v, newR, newG, newB);

        pixel[0] = newB;
        pixel[1] = newG;
        pixel[2] = newR;
    }
    else
    {
        pixel[0] = ColorConstants::RGB_MAX - b;
        pixel[1] = ColorConstants::RGB_MAX - g;
        pixel[2] = ColorConstants::RGB_MAX - r;
    }
    
    pixel[3] = a;
}

double ColorInverter::getLuminance(unsigned char r, unsigned char g, unsigned char b) {
    // Calculate perceived brightness using the standard formula
    double brightness = (ColorConstants::RGB_LUMINANCE_R * r + ColorConstants::RGB_LUMINANCE_G * g + ColorConstants::RGB_LUMINANCE_B * b) / ColorConstants::RGB_SCALE;
    return brightness;
}

double ColorInverter::getSaturation(unsigned char r, unsigned char g, unsigned char b){
    double maxChannel  = std::max({r,g,b});
    double minChannel  = std::min({r,g,b});

    double saturation = (maxChannel-minChannel)/maxChannel;
    return saturation;
}

void ColorInverter::rgbToHsv(unsigned char r, unsigned char g, unsigned char b, 
                            double& h, double& s, double& v) {
    double rf = r / ColorConstants::RGB_SCALE;
    double gf = g / ColorConstants::RGB_SCALE;
    double bf = b / ColorConstants::RGB_SCALE;
    
    double maxVal = std::max({rf, gf, bf});
    double minVal = std::min({rf, gf, bf});
    double delta = maxVal - minVal;
    
    // Value (brightness)
    v = (maxVal + minVal) / ColorConstants::HSV_LIGHTNESS_FACTOR;
    
    // Saturation
    if (maxVal == 0) {
        s = 0;
    } else {
        s = delta / maxVal;
    }
    
    // Hue
    if (delta == 0) {
        h = 0;
    } else if (maxVal == rf) {
        h = ColorConstants::HUE_SECTOR_SIZE * (((gf - bf) / delta) + (gf < bf ? 6 : 0));
    } else if (maxVal == gf) {
        h = ColorConstants::HUE_SECTOR_SIZE * (((bf - rf) / delta) + 2);
    } else {
        h = ColorConstants::HUE_SECTOR_SIZE * (((rf - gf) / delta) + 4);
    }
    
    if (h < 0) h += ColorConstants::HUE_CIRCLE_DEGREES;
    if (h >= ColorConstants::HUE_CIRCLE_DEGREES) h -= ColorConstants::HUE_CIRCLE_DEGREES;
}

void ColorInverter::hsvToRgb(double h, double s, double v, 
                            unsigned char& r, unsigned char& g, unsigned char& b) {
    double c = v * s;
    double x = c * (1 - std::abs(std::fmod(h / ColorConstants::HUE_SECTOR_SIZE, ColorConstants::HSV_HUE_MODULO) - 1));
    double m = v - c / ColorConstants::HSV_SATURATION_DIVISOR;
    
    double rf, gf, bf;
    
    if (h >= ColorConstants::HUE_RANGE_1_MIN && h < ColorConstants::HUE_RANGE_1_MAX) {
        rf = c; gf = x; bf = 0;
    } else if (h >= ColorConstants::HUE_RANGE_2_MIN && h < ColorConstants::HUE_RANGE_2_MAX) {
        rf = x; gf = c; bf = 0;
    } else if (h >= ColorConstants::HUE_RANGE_3_MIN && h < ColorConstants::HUE_RANGE_3_MAX) {
        rf = 0; gf = c; bf = x;
    } else if (h >= ColorConstants::HUE_RANGE_4_MIN && h < ColorConstants::HUE_RANGE_4_MAX) {
        rf = 0; gf = x; bf = c;
    } else if (h >= ColorConstants::HUE_RANGE_5_MIN && h < ColorConstants::HUE_RANGE_5_MAX) {
        rf = x; gf = 0; bf = c;
    } else {
        rf = c; gf = 0; bf = x;
    }
    
    r = static_cast<unsigned char>((rf + m) * ColorConstants::RGB_MAX);
    g = static_cast<unsigned char>((gf + m) * ColorConstants::RGB_MAX);
    b = static_cast<unsigned char>((bf + m) * ColorConstants::RGB_MAX);
}
