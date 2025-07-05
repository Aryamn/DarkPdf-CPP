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

    // Color scheme mapping thresholds
    constexpr double LIGHT_THRESHOLD = 0.7;               // Threshold for light colors (map to background)
    constexpr double DARK_THRESHOLD = 0.3;                // Threshold for dark colors (map to text)
}

void ColorInverter::invertColors(cairo_surface_t* surface, const ColorScheme& scheme) {
    if (!surface) return;
    
    cairo_surface_flush(surface);
    
    unsigned char* data = cairo_image_surface_get_data(surface);
    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    int stride = cairo_image_surface_get_stride(surface);
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char* pixel = data + y * stride + x * ColorConstants::PIXEL_STRIDE;
            invertPixel(pixel, scheme);
        }
    }
    
    cairo_surface_mark_dirty(surface);
}

void ColorInverter::invertPixel(unsigned char* pixel, const ColorScheme& scheme) {

    unsigned char b = pixel[0];
    unsigned char g = pixel[1];
    unsigned char r = pixel[2];
    unsigned char a = pixel[ColorConstants::ALPHA_CHANNEL_OFFSET];

    if (a == ColorConstants::TRANSPARENT_ALPHA) return;

    RGB originalColor(r, g, b);
    
    RGB mappedColor = mapToScheme(originalColor, scheme);
    
    pixel[0] = mappedColor.b;  // Blue
    pixel[1] = mappedColor.g;  // Green
    pixel[2] = mappedColor.r;  // Red
    pixel[3] = a;              // Alpha
}

double ColorInverter::getLuminance(unsigned char r, unsigned char g, unsigned char b) {
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
    
    // Value (brightness) - correct HSV formula
    v = maxVal;
    
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
    double x = c * (1 - std::abs(std::fmod(h / ColorConstants::HUE_SECTOR_SIZE, 2.0) - 1));
    double m = v - c;
    
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

RGB ColorInverter::mapToScheme(const RGB& original, const ColorScheme& scheme) {
    double luminosity = calculateLuminosity(original);
    double saturation = getSaturation(original.r, original.g, original.b);
    
    // Check if it's a colored element (high saturation)
    if (saturation > ColorConstants::SATURATION_THRESHOLD) {
        // For colored elements, preserve hue but adapt brightness for dark mode
        double h, s, v;

        if(rgbToHsvCache.find(original)== rgbToHsvCache.end()) {
            rgbToHsv(original.r, original.g, original.b, h, s, v);
            rgbToHsvCache[original] = std::make_tuple(h, s, v);
        } else {
            std::tie(h, s, v) = rgbToHsvCache[original];
        }
 
        // Map HSV value (brightness) to appropriate dark mode range
        double targetV;
        if (v > ColorConstants::LIGHT_THRESHOLD) {
            // Bright colored element -> make moderately bright for dark mode
            targetV = 0.5 + (v - ColorConstants::LIGHT_THRESHOLD) * 0.3;
        } else if (v < ColorConstants::DARK_THRESHOLD) {
            // Dark colored element -> make brighter for visibility
            targetV = 0.4 + v * 0.5;
        } else {
            // Mid-tone colored element -> adjust moderately
            targetV = 0.3 + v * 0.4;
        }
        
        targetV = std::max(0.2, std::min(0.8, targetV));
        
        // Reduce saturation slightly for better readability in dark mode
        double targetS = s * ColorConstants::SATURATION_REDUCTION;
        
        unsigned char newR, newG, newB;
        std::tuple<double,double,double>hsvTuple{h, targetS, targetV};

        if(hsvToRgbCache.find(hsvTuple) == hsvToRgbCache.end()) {
            hsvToRgb(h, targetS, targetV, newR, newG, newB);
            hsvToRgbCache[hsvTuple] = RGB(newR, newG, newB);
            
        }

       return hsvToRgbCache[hsvTuple];
    } else {
        // For grayscale elements, use luminosity-based mapping
        if (luminosity > ColorConstants::LIGHT_THRESHOLD) {
            // Light background colors -> scheme background
            return scheme.backgroundColor;
        } else if (luminosity < ColorConstants::DARK_THRESHOLD) {
            // Dark text colors -> scheme text color
            return scheme.textColor;
        } else {
            // Mid-tones -> interpolate between background and text
            double factor = (luminosity - ColorConstants::DARK_THRESHOLD) / 
                          (ColorConstants::LIGHT_THRESHOLD - ColorConstants::DARK_THRESHOLD);
            return interpolateColors(scheme.textColor, scheme.backgroundColor, factor);
        }
    }
}

double ColorInverter::calculateLuminosity(const RGB& color) {
    return (ColorConstants::RGB_LUMINANCE_R * color.r + 
            ColorConstants::RGB_LUMINANCE_G * color.g + 
            ColorConstants::RGB_LUMINANCE_B * color.b) / ColorConstants::RGB_SCALE;
}

RGB ColorInverter::interpolateColors(const RGB& color1, const RGB& color2, double factor) {
    factor = std::max(0.0, std::min(1.0, factor));
    
    unsigned char r = static_cast<unsigned char>(color1.r + factor * (color2.r - color1.r));
    unsigned char g = static_cast<unsigned char>(color1.g + factor * (color2.g - color1.g));
    unsigned char b = static_cast<unsigned char>(color1.b + factor * (color2.b - color1.b));
    
    return RGB(r, g, b);
}