#include "color_inverter.h"
#include <algorithm>
#include <cmath>

void ColorInverter::invertColors(cairo_surface_t* surface) {
    if (!surface) return;
    
    cairo_surface_flush(surface);
    
    unsigned char* data = cairo_image_surface_get_data(surface);
    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    int stride = cairo_image_surface_get_stride(surface);
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char* pixel = data + y * stride + x * 4;
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
    unsigned char a = pixel[3];

    // Skip transparent pixels
    if (a == 0) return;

    double luminance = getLuminance(r, g, b);
    double saturation = getSaturation(r, g, b);

    if(saturation > 0.1f && luminance < 200) 
    {
        double h, s, v;
        rgbToHsv(r, g, b, h, s, v);

        v = 0.7f + (v * 0.3f);  // Make colored text brighter
        s = s * 0.8f;

        unsigned char newR, newG, newB;
        hsvToRgb(h, s, v, newR, newG, newB);

        pixel[0] = newB;  // B
        pixel[1] = newG;  // G
        pixel[2] = newR;  // R
    }
    else
    {
        // Low saturation (grayscale-ish) - full inversion
        pixel[0] = 255 - b;  // B
        pixel[1] = 255 - g;  // G
        pixel[2] = 255 - r;  // R
    }
    
    // Alpha channel remains unchanged
    pixel[3] = a;
}

double ColorInverter::getLuminance(unsigned char r, unsigned char g, unsigned char b) {
    // Calculate perceived brightness using the standard formula
    double brightness = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0;
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
    double rf = r / 255.0;
    double gf = g / 255.0;
    double bf = b / 255.0;
    
    double maxVal = std::max({rf, gf, bf});
    double minVal = std::min({rf, gf, bf});
    double delta = maxVal - minVal;
    
    // Value (brightness)
    v = (maxVal + minVal)/2.0;
    
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
        h = 60 * (((gf - bf) / delta) + (gf < bf ? 6 : 0));
    } else if (maxVal == gf) {
        h = 60 * (((bf - rf) / delta) + 2);
    } else {
        h = 60 * (((rf - gf) / delta) + 4);
    }
    
    if (h < 0) h += 360;
    if (h >= 360) h -= 360;
}

void ColorInverter::hsvToRgb(double h, double s, double v, 
                            unsigned char& r, unsigned char& g, unsigned char& b) {
    double c = v * s;
    double x = c * (1 - std::abs(std::fmod(h / 60.0, 2) - 1));
    double m = v - c / 2;
    
    double rf, gf, bf;
    
    if (h >= 0 && h < 60) {
        rf = c; gf = x; bf = 0;
    } else if (h >= 60 && h < 120) {
        rf = x; gf = c; bf = 0;
    } else if (h >= 120 && h < 180) {
        rf = 0; gf = c; bf = x;
    } else if (h >= 180 && h < 240) {
        rf = 0; gf = x; bf = c;
    } else if (h >= 240 && h < 300) {
        rf = x; gf = 0; bf = c;
    } else {
        rf = c; gf = 0; bf = x;
    }
    
    r = static_cast<unsigned char>((rf + m) * 255);
    g = static_cast<unsigned char>((gf + m) * 255);
    b = static_cast<unsigned char>((bf + m) * 255);
}
