#include "color_scheme.h"
#include <iostream>
#include <algorithm>

void ColorSchemeManager::ensureInitialized() {
    if (!initialized) {
        schemes = initializeSchemes();
        initialized = true;
    }
}

std::vector<ColorScheme> ColorSchemeManager::initializeSchemes() {
    std::vector<ColorScheme> schemeList;
    
    // 1. Classic Dark (Default)
    schemeList.emplace_back(
        "Classic Dark",
        "classic",
        RGB(0, 0, 0),           
        RGB(255, 255, 255), 
        "Standard dark mode with maximum contrast"
    );
    
    // Add "default" as alias for classic
    schemeList.emplace_back(
        "Classic Dark",
        "default",
        RGB(0, 0, 0),           
        RGB(255, 255, 255), 
        "Standard dark mode with maximum contrast (default)"
    );
    
    // 2. Sepia Dark
    schemeList.emplace_back(
        "Sepia Dark",
        "sepia",
        RGB(43, 27, 23), 
        RGB(244, 228, 188),
        "Reduced eye strain with warm tones for long reading"
    );
    
    // 3. Blue Light Filter
    schemeList.emplace_back(
        "Blue Light Filter",
        "blue-filter",
        RGB(30, 35, 40),
        RGB(240, 230, 210),
        "Night reading mode that reduces blue light exposure"
    );
    
    // 4. OLED Black
    schemeList.emplace_back(
        "OLED Black",
        "oled",
        RGB(0, 0, 0),           
        RGB(248, 248, 248), 
        "Optimized for OLED displays and battery saving"
    );
    
    return schemeList;
}

std::vector<ColorScheme> ColorSchemeManager::getAvailableSchemes() {
    ensureInitialized();
    
    std::vector<ColorScheme> uniqueSchemes;
    std::vector<std::string> seenNames;
    
    for (const auto& scheme : schemes) {
        if (std::find(seenNames.begin(), seenNames.end(), scheme.name) == seenNames.end()) {
            uniqueSchemes.push_back(scheme);
            seenNames.push_back(scheme.name);
        }
    }
    
    return uniqueSchemes;
}

ColorScheme ColorSchemeManager::getSchemeByName(const std::string& name) {
    ensureInitialized();
    
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    for (const auto& scheme : schemes) {
        std::string lowerIdentifier = scheme.identifier;
        std::transform(lowerIdentifier.begin(), lowerIdentifier.end(), lowerIdentifier.begin(), ::tolower);
        
        if (lowerIdentifier == lowerName) {
            return scheme;
        }
    }
    
    return getDefaultScheme();
}

ColorScheme ColorSchemeManager::getDefaultScheme() {
    ensureInitialized();
    
    for (const auto& scheme : schemes) {
        if (scheme.identifier == "classic") {
            return scheme;
        }
    }
    
    // Fallback (should never happen)
    return ColorScheme("Classic Dark", "classic", RGB(0, 0, 0), RGB(255, 255, 255), 
                      "Standard dark mode with maximum contrast");
}

bool ColorSchemeManager::isValidScheme(const std::string& name) {
    ensureInitialized();
    
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    for (const auto& scheme : schemes) {
        std::string lowerIdentifier = scheme.identifier;
        std::transform(lowerIdentifier.begin(), lowerIdentifier.end(), lowerIdentifier.begin(), ::tolower);
        
        if (lowerIdentifier == lowerName) {
            return true;
        }
    }
    
    return false;
}

void ColorSchemeManager::printAvailableSchemes() {
    std::cout << "Available color schemes:\n\n";
    
    auto uniqueSchemes = getAvailableSchemes();
    
    for (const auto& scheme : uniqueSchemes) {
        std::cout << "  " << scheme.identifier << "\n";
        std::cout << "    Name: " << scheme.name << "\n";
        std::cout << "    Description: " << scheme.description << "\n";
        std::cout << "    Background: RGB(" << static_cast<int>(scheme.backgroundColor.r) 
                  << ", " << static_cast<int>(scheme.backgroundColor.g) 
                  << ", " << static_cast<int>(scheme.backgroundColor.b) << ")\n";
        std::cout << "    Text: RGB(" << static_cast<int>(scheme.textColor.r) 
                  << ", " << static_cast<int>(scheme.textColor.g) 
                  << ", " << static_cast<int>(scheme.textColor.b) << ")\n\n";
    }
    
    std::cout << "Usage: darkpdf.exe input.pdf output.pdf --scheme <scheme_name>\n";
    std::cout << "Example: darkpdf.exe document.pdf output.pdf --scheme sepia\n";
}
