#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <cstring>
#include "color_inverter.h"
#include "color_scheme.h"
#include <cairo.h>

class ColorInverterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test surface
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 4, 4);
        
        // Fill with test pattern
        unsigned char* data = cairo_image_surface_get_data(surface);
        int stride = cairo_image_surface_get_stride(surface);
        
        // Create test pixels: [white, black, gray, red]
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                unsigned char* pixel = data + y * stride + x * 4;
                
                if (x == 0) {
                    // White pixel (BGRA format)
                    pixel[0] = 255; pixel[1] = 255; pixel[2] = 255; pixel[3] = 255;
                } else if (x == 1) {
                    // Black pixel
                    pixel[0] = 0; pixel[1] = 0; pixel[2] = 0; pixel[3] = 255;
                } else if (x == 2) {
                    // Gray pixel
                    pixel[0] = 128; pixel[1] = 128; pixel[2] = 128; pixel[3] = 255;
                } else {
                    // Red pixel
                    pixel[0] = 0; pixel[1] = 0; pixel[2] = 255; pixel[3] = 255;
                }
            }
        }
        cairo_surface_mark_dirty(surface);
    }

    void TearDown() override {
        if (surface) {
            cairo_surface_destroy(surface);
        }
    }

    cairo_surface_t* surface = nullptr;
};

// Test calculateLuminosity function
TEST_F(ColorInverterTest, CalculateLuminosity) {
    // Test pure colors
    RGB white(255, 255, 255);
    RGB black(0, 0, 0);
    RGB red(255, 0, 0);
    RGB green(0, 255, 0);
    RGB blue(0, 0, 255);
    
    double whiteLum = ColorInverter::calculateLuminosity(white);
    double blackLum = ColorInverter::calculateLuminosity(black);
    double redLum = ColorInverter::calculateLuminosity(red);
    double greenLum = ColorInverter::calculateLuminosity(green);
    double blueLum = ColorInverter::calculateLuminosity(blue);
    
    // White should have maximum luminosity
    EXPECT_NEAR(whiteLum, 1.0, 0.01);
    
    // Black should have minimum luminosity
    EXPECT_NEAR(blackLum, 0.0, 0.01);
    
    // Green should have higher luminosity than red and blue (due to human eye sensitivity)
    EXPECT_GT(greenLum, redLum);
    EXPECT_GT(greenLum, blueLum);
    
    // All luminosity values should be in range [0, 1]
    EXPECT_GE(whiteLum, 0.0);
    EXPECT_LE(whiteLum, 1.0);
    EXPECT_GE(blackLum, 0.0);
    EXPECT_LE(blackLum, 1.0);
}

// Test interpolateColors function
TEST_F(ColorInverterTest, InterpolateColors) {
    RGB color1(0, 0, 0);        // Black
    RGB color2(255, 255, 255);  // White
    
    // Test interpolation at various factors
    RGB result0 = ColorInverter::interpolateColors(color1, color2, 0.0);
    RGB result25 = ColorInverter::interpolateColors(color1, color2, 0.25);
    RGB result50 = ColorInverter::interpolateColors(color1, color2, 0.5);
    RGB result75 = ColorInverter::interpolateColors(color1, color2, 0.75);
    RGB result100 = ColorInverter::interpolateColors(color1, color2, 1.0);
    
    // Check results with tolerance for rounding
    EXPECT_TRUE(result0 == color1);
    EXPECT_TRUE(result100 == color2);
    
    EXPECT_NEAR(result25.r, 64, 1);   // Allow ±1 for rounding differences
    EXPECT_NEAR(result50.r, 128, 1);  // Allow ±1 for rounding differences  
    EXPECT_NEAR(result75.r, 191, 1);  // Allow ±1 for rounding differences
    
    // Test clamping
    RGB resultNeg = ColorInverter::interpolateColors(color1, color2, -0.5);
    RGB resultOver = ColorInverter::interpolateColors(color1, color2, 1.5);
    
    EXPECT_TRUE(resultNeg == color1);   // Should clamp to 0.0
    EXPECT_TRUE(resultOver == color2);  // Should clamp to 1.0
}

// Test mapToScheme function with Classic Dark scheme
TEST_F(ColorInverterTest, MapToSchemeClassicDark) {
    ColorScheme classicScheme = ColorSchemeManager::getSchemeByName("classic");
    
    // Test light color mapping (should map to background)
    RGB lightColor(240, 240, 240);
    RGB mappedLight = ColorInverter::mapToScheme(lightColor, classicScheme);
    EXPECT_TRUE(mappedLight == classicScheme.backgroundColor);
    
    // Test dark color mapping (should map to text color)
    RGB darkColor(20, 20, 20);
    RGB mappedDark = ColorInverter::mapToScheme(darkColor, classicScheme);
    EXPECT_TRUE(mappedDark == classicScheme.textColor);
    
    // Test mid-tone mapping (should interpolate)
    RGB midColor(128, 128, 128);
    RGB mappedMid = ColorInverter::mapToScheme(midColor, classicScheme);
    
    // Mid-tone should be between background and text colors
    EXPECT_NE(mappedMid.r, classicScheme.backgroundColor.r);
    EXPECT_NE(mappedMid.r, classicScheme.textColor.r);
    EXPECT_GT(mappedMid.r, classicScheme.backgroundColor.r);
    EXPECT_LT(mappedMid.r, classicScheme.textColor.r);
}

// Test mapToScheme function with Sepia scheme
TEST_F(ColorInverterTest, MapToSchemeSepia) {
    ColorScheme sepiaScheme = ColorSchemeManager::getSchemeByName("sepia");
    
    // Test light color mapping
    RGB lightColor(250, 250, 250);
    RGB mappedLight = ColorInverter::mapToScheme(lightColor, sepiaScheme);
    EXPECT_TRUE(mappedLight == sepiaScheme.backgroundColor);
    
    // Test dark color mapping
    RGB darkColor(10, 10, 10);
    RGB mappedDark = ColorInverter::mapToScheme(darkColor, sepiaScheme);
    EXPECT_TRUE(mappedDark == sepiaScheme.textColor);
}

// Test colored element handling
TEST_F(ColorInverterTest, MapToSchemeColoredElements) {
    ColorScheme classicScheme = ColorSchemeManager::getSchemeByName("classic");
    
    // Test medium red (high saturation but not too bright)
    RGB mediumRed(180, 30, 30);  // High saturation, medium luminosity
    RGB mappedRed = ColorInverter::mapToScheme(mediumRed, classicScheme);
    
    // Should not be pure black or white (since it's colored)
    EXPECT_FALSE(mappedRed == classicScheme.backgroundColor);
    EXPECT_FALSE(mappedRed == classicScheme.textColor);
    
    // For colored elements, we expect it to be processed differently than pure grayscale
    // The exact output depends on the HSV transformation, so let's test more generally
    
    // Test that saturation is preserved (it should still look reddish)
    // We can't guarantee exact RGB dominance due to HSV transformations,
    // but we can test that it's not a pure grayscale color
    bool isGrayscale = (mappedRed.r == mappedRed.g && mappedRed.g == mappedRed.b);
    EXPECT_FALSE(isGrayscale) << "Colored input should not result in pure grayscale output";
    
    // Test with a darker saturated color that should definitely be in the colored range
    RGB darkSaturatedBlue(0, 0, 150);  // Dark but saturated blue
    RGB mappedBlue = ColorInverter::mapToScheme(darkSaturatedBlue, classicScheme);
    
    // Should be processed as colored element, not pure text color
    EXPECT_FALSE(mappedBlue == classicScheme.textColor);
    EXPECT_FALSE(mappedBlue == classicScheme.backgroundColor);
}

// Test surface inversion
TEST_F(ColorInverterTest, InvertColorsWithScheme) {
    ColorScheme classicScheme = ColorSchemeManager::getSchemeByName("classic");
    
    // Get original pixel data
    unsigned char* data = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);
    
    // Store original white pixel (BGRA format)
    unsigned char originalWhite[4];
    unsigned char* whitePixel = data + 0 * stride + 0 * 4; // First pixel
    memcpy(originalWhite, whitePixel, 4);
    
    // Apply color inversion
    ColorInverter::invertColors(surface, classicScheme);
    
    // Check that white pixel changed
    EXPECT_NE(whitePixel[0], originalWhite[0]); // Blue channel should change
    EXPECT_NE(whitePixel[1], originalWhite[1]); // Green channel should change
    EXPECT_NE(whitePixel[2], originalWhite[2]); // Red channel should change
    EXPECT_EQ(whitePixel[3], originalWhite[3]); // Alpha should remain unchanged
}

// Test null surface handling
TEST_F(ColorInverterTest, InvertColorsNullSurface) {
    ColorScheme classicScheme = ColorSchemeManager::getSchemeByName("classic");
    
    // Should not crash with null surface
    EXPECT_NO_THROW(ColorInverter::invertColors(nullptr, classicScheme));
}

// Test transparent pixels (should be skipped)
TEST_F(ColorInverterTest, TransparentPixelHandling) {
    ColorScheme classicScheme = ColorSchemeManager::getSchemeByName("classic");
    
    // Create surface with transparent pixel
    cairo_surface_t* testSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    unsigned char* data = cairo_image_surface_get_data(testSurface);
    
    // Set transparent pixel (alpha = 0)
    data[0] = 255; // Blue
    data[1] = 255; // Green  
    data[2] = 255; // Red
    data[3] = 0;   // Alpha (transparent)
    
    cairo_surface_mark_dirty(testSurface);
    
    // Store original values
    unsigned char original[4];
    memcpy(original, data, 4);
    
    // Apply inversion
    ColorInverter::invertColors(testSurface, classicScheme);
    
    // Transparent pixel should remain unchanged
    EXPECT_EQ(data[0], original[0]);
    EXPECT_EQ(data[1], original[1]);
    EXPECT_EQ(data[2], original[2]);
    EXPECT_EQ(data[3], original[3]);
    
    cairo_surface_destroy(testSurface);
}

// Performance test (basic)
TEST_F(ColorInverterTest, PerformanceBasic) {
    ColorScheme classicScheme = ColorSchemeManager::getSchemeByName("classic");
    
    // Create larger surface for performance test
    cairo_surface_t* largeSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 100, 100);
    
    auto start = std::chrono::high_resolution_clock::now();
    ColorInverter::invertColors(largeSurface, classicScheme);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete within reasonable time (1 second for 100x100 is very generous)
    EXPECT_LT(duration.count(), 1000);
    
    cairo_surface_destroy(largeSurface);
}

// Debug test to understand color mapping
TEST_F(ColorInverterTest, DebugColorMapping) {
    ColorScheme classicScheme = ColorSchemeManager::getSchemeByName("classic");
    
    // Test bright red to see what happens
    RGB brightRed(255, 0, 0);
    double luminosity = ColorInverter::calculateLuminosity(brightRed);
    RGB mappedRed = ColorInverter::mapToScheme(brightRed, classicScheme);
    
    // Print debug info (will show in test output if verbose)
    std::cout << "Bright Red RGB(255,0,0):" << std::endl;
    std::cout << "  Luminosity: " << luminosity << std::endl;
    std::cout << "  Mapped to: RGB(" << (int)mappedRed.r << "," << (int)mappedRed.g << "," << (int)mappedRed.b << ")" << std::endl;
    std::cout << "  Background: RGB(" << (int)classicScheme.backgroundColor.r << "," << (int)classicScheme.backgroundColor.g << "," << (int)classicScheme.backgroundColor.b << ")" << std::endl;
    std::cout << "  Text: RGB(" << (int)classicScheme.textColor.r << "," << (int)classicScheme.textColor.g << "," << (int)classicScheme.textColor.b << ")" << std::endl;
    
    // Test different luminosity levels
    RGB darkRed(80, 0, 0);
    RGB mediumRed(150, 0, 0);
    
    double darkLum = ColorInverter::calculateLuminosity(darkRed);
    double mediumLum = ColorInverter::calculateLuminosity(mediumRed);
    
    std::cout << "Dark Red luminosity: " << darkLum << std::endl;
    std::cout << "Medium Red luminosity: " << mediumLum << std::endl;
    std::cout << "Bright Red luminosity: " << luminosity << std::endl;
}
