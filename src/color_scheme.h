#pragma once
#include <string>
#include <vector>

struct RGB {
    unsigned char r, g, b;
    
    RGB(unsigned char red, unsigned char green, unsigned char blue) 
        : r(red), g(green), b(blue) {}
    
    RGB() : r(0), g(0), b(0) {}
    
    bool operator==(const RGB& other) const {
        return r == other.r && g == other.g && b == other.b;
    }
};

struct ColorScheme {
    std::string name;
    std::string identifier;
    RGB backgroundColor;
    RGB textColor;
    std::string description;
    
    ColorScheme(const std::string& n, const std::string& id, 
                const RGB& bg, const RGB& text, const std::string& desc)
        : name(n), identifier(id), backgroundColor(bg), textColor(text), description(desc) {}
    
    ColorScheme() = default;
};

// Color scheme manager class
class ColorSchemeManager {
public:
    static std::vector<ColorScheme> getAvailableSchemes();
    static ColorScheme getSchemeByName(const std::string& name);
    static ColorScheme getDefaultScheme();
    static bool isValidScheme(const std::string& name);
    static void printAvailableSchemes();
    
private:
    static std::vector<ColorScheme> initializeSchemes();
    static std::vector<ColorScheme> schemes;
    static bool initialized;
    static void ensureInitialized();
};
