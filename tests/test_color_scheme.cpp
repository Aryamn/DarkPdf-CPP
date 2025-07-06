#include <gtest/gtest.h>
#include "color_scheme.h"

class ColorSchemeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code for each test
    }

    void TearDown() override {
        // Cleanup code for each test
    }
};

// Test RGB structure
TEST_F(ColorSchemeTest, RGBConstructor) {
    RGB color(255, 128, 64);
    EXPECT_EQ(color.r, 255);
    EXPECT_EQ(color.g, 128);
    EXPECT_EQ(color.b, 64);
}

TEST_F(ColorSchemeTest, RGBEquality) {
    RGB color1(255, 128, 64);
    RGB color2(255, 128, 64);
    RGB color3(255, 128, 65);
    
    EXPECT_TRUE(color1 == color2);
    EXPECT_FALSE(color1 == color3);
}

// Test ColorScheme structure
TEST_F(ColorSchemeTest, ColorSchemeConstructor) {
    RGB bg(0, 0, 0);
    RGB text(255, 255, 255);
    ColorScheme scheme("Test Scheme", "test", bg, text, "Test description");
    
    EXPECT_EQ(scheme.name, "Test Scheme");
    EXPECT_EQ(scheme.identifier, "test");
    EXPECT_TRUE(scheme.backgroundColor == bg);
    EXPECT_TRUE(scheme.textColor == text);
    EXPECT_EQ(scheme.description, "Test description");
}

// Test ColorSchemeManager
TEST_F(ColorSchemeTest, GetAvailableSchemes) {
    auto schemes = ColorSchemeManager::getAvailableSchemes();
    
    // Should have exactly 4 unique schemes (no aliases)
    EXPECT_EQ(schemes.size(), 4);
    
    // Check that we have the expected schemes
    std::vector<std::string> expectedIdentifiers = {"classic", "sepia", "blue-filter", "oled"};
    
    for (const auto& expectedId : expectedIdentifiers) {
        bool found = false;
        for (const auto& scheme : schemes) {
            if (scheme.identifier == expectedId) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Expected scheme '" << expectedId << "' not found";
    }
}

TEST_F(ColorSchemeTest, GetDefaultScheme) {
    auto defaultScheme = ColorSchemeManager::getDefaultScheme();
    
    EXPECT_EQ(defaultScheme.identifier, "classic");
    EXPECT_EQ(defaultScheme.name, "Classic Dark");
    
    // Check colors
    RGB expectedBg(0, 0, 0);        // Black
    RGB expectedText(255, 255, 255); // White
    
    EXPECT_TRUE(defaultScheme.backgroundColor == expectedBg);
    EXPECT_TRUE(defaultScheme.textColor == expectedText);
}

TEST_F(ColorSchemeTest, GetSchemeByName) {
    // Test valid scheme names
    auto classicScheme = ColorSchemeManager::getSchemeByName("classic");
    EXPECT_EQ(classicScheme.identifier, "classic");
    
    auto sepiaScheme = ColorSchemeManager::getSchemeByName("sepia");
    EXPECT_EQ(sepiaScheme.identifier, "sepia");
    
    // Test case insensitive
    auto classicScheme2 = ColorSchemeManager::getSchemeByName("CLASSIC");
    EXPECT_EQ(classicScheme2.identifier, "classic");
    
    // Test alias
    auto defaultScheme = ColorSchemeManager::getSchemeByName("default");
    EXPECT_EQ(defaultScheme.identifier, "default"); // Should resolve to default
}

TEST_F(ColorSchemeTest, GetSchemeByNameInvalid) {
    // Test invalid scheme name - should return default
    auto invalidScheme = ColorSchemeManager::getSchemeByName("nonexistent");
    EXPECT_EQ(invalidScheme.identifier, "classic"); // Should fallback to default
}

TEST_F(ColorSchemeTest, IsValidScheme) {
    // Test valid schemes
    EXPECT_TRUE(ColorSchemeManager::isValidScheme("classic"));
    EXPECT_TRUE(ColorSchemeManager::isValidScheme("sepia"));
    EXPECT_TRUE(ColorSchemeManager::isValidScheme("blue-filter"));
    EXPECT_TRUE(ColorSchemeManager::isValidScheme("oled"));
    EXPECT_TRUE(ColorSchemeManager::isValidScheme("default")); // alias
    
    // Test case insensitive
    EXPECT_TRUE(ColorSchemeManager::isValidScheme("CLASSIC"));
    EXPECT_TRUE(ColorSchemeManager::isValidScheme("Sepia"));
    
    // Test invalid schemes
    EXPECT_FALSE(ColorSchemeManager::isValidScheme("nonexistent"));
    EXPECT_FALSE(ColorSchemeManager::isValidScheme(""));
}

// Test specific color schemes
TEST_F(ColorSchemeTest, ClassicDarkScheme) {
    auto scheme = ColorSchemeManager::getSchemeByName("classic");
    
    EXPECT_EQ(scheme.name, "Classic Dark");
    EXPECT_EQ(scheme.identifier, "classic");
    
    // Check RGB values
    RGB expectedBg(0, 0, 0);        // Pure Black
    RGB expectedText(255, 255, 255); // Pure White
    
    EXPECT_TRUE(scheme.backgroundColor == expectedBg);
    EXPECT_TRUE(scheme.textColor == expectedText);
}

TEST_F(ColorSchemeTest, SepiaDarkScheme) {
    auto scheme = ColorSchemeManager::getSchemeByName("sepia");
    
    EXPECT_EQ(scheme.name, "Sepia Dark");
    EXPECT_EQ(scheme.identifier, "sepia");
    
    // Check RGB values
    RGB expectedBg(43, 27, 23);     // Dark Brown
    RGB expectedText(244, 228, 188); // Warm Cream
    
    EXPECT_TRUE(scheme.backgroundColor == expectedBg);
    EXPECT_TRUE(scheme.textColor == expectedText);
}

TEST_F(ColorSchemeTest, BlueLightFilterScheme) {
    auto scheme = ColorSchemeManager::getSchemeByName("blue-filter");
    
    EXPECT_EQ(scheme.name, "Blue Light Filter");
    EXPECT_EQ(scheme.identifier, "blue-filter");
    
    // Check RGB values
    RGB expectedBg(30, 35, 40);     // Dark Blue-Gray
    RGB expectedText(240, 230, 210); // Warm White
    
    EXPECT_TRUE(scheme.backgroundColor == expectedBg);
    EXPECT_TRUE(scheme.textColor == expectedText);
}

TEST_F(ColorSchemeTest, OLEDBlackScheme) {
    auto scheme = ColorSchemeManager::getSchemeByName("oled");
    
    EXPECT_EQ(scheme.name, "OLED Black");
    EXPECT_EQ(scheme.identifier, "oled");
    
    // Check RGB values
    RGB expectedBg(0, 0, 0);        // Pure Black
    RGB expectedText(248, 248, 248); // Slightly Off-White
    
    EXPECT_TRUE(scheme.backgroundColor == expectedBg);
    EXPECT_TRUE(scheme.textColor == expectedText);
}
