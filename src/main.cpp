#include <iostream>
#include <string>
#include <vector>
#include <CLI/CLI.hpp>
#include "pdf_processor.h"
#include "color_scheme.h"

void useFastIO() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
}

int main(int argc, char* argv[]) {
    useFastIO();
    
    CLI::App app{"DarkPdf - PDF Dark Mode Converter"};
    app.set_version_flag("--version", "1.0.0");
    
    std::string inputFile;
    std::string outputFile;
    std::string colorScheme = "classic";
    bool listSchemes = false;
    
    app.add_option("input", inputFile, "Input PDF file")->required();
    app.add_option("output", outputFile, "Output PDF file")->required();
    
    auto schemeOpt = app.add_option("-s,--scheme", colorScheme, "Color scheme to use (default: classic)")
        ->default_val("classic");
    
    app.add_flag("--list-schemes", listSchemes, "List all available color schemes");
    
    schemeOpt->check([](const std::string& val) -> std::string {
        if (!ColorSchemeManager::isValidScheme(val)) {
            auto availableSchemes = ColorSchemeManager::getAvailableSchemes();
            std::string validOptions = "Valid options: ";
            for (size_t i = 0; i < availableSchemes.size(); ++i) {
                if (i > 0) validOptions += ", ";
                validOptions += availableSchemes[i].identifier;
            }
            return "Invalid color scheme '" + val + "'. " + validOptions;
        }
        return "";
    });
    
    app.footer("\nExamples:\n"
              "  darkpdf.exe input.pdf output.pdf\n"
              "  darkpdf.exe input.pdf output.pdf --scheme sepia\n"
              "  darkpdf.exe input.pdf output.pdf -s blue-filter\n"
              "  darkpdf.exe --list-schemes");
    
    if (argc >= 2 && std::string(argv[1]) == "--list-schemes") {
        ColorSchemeManager::printAvailableSchemes();
        return 0;
    }
    
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }
    
    if (listSchemes) {
        ColorSchemeManager::printAvailableSchemes();
        return 0;
    }
    
    ColorScheme selectedScheme = ColorSchemeManager::getSchemeByName(colorScheme);
    
    std::cout << "DarkPdf - PDF Dark Mode Converter v1.0.0\n";
    std::cout << "Input: " << inputFile << "\n";
    std::cout << "Output: " << outputFile << "\n";
    std::cout << "Color Scheme: " << selectedScheme.name << " (" << selectedScheme.identifier << ")\n";
    std::cout << "\n";
    
    PdfProcessor processor;
    
    std::cout << "Loading PDF...\n";
    if (!processor.loadPdf(inputFile)) {
        std::cerr << "Error: Failed to load PDF file: " << inputFile << "\n";
        return 1;
    }
    
    std::cout << "Converting to dark mode...\n";
    if (!processor.convertToDarkMode(outputFile, selectedScheme)) {
        std::cerr << "Error: Failed to convert PDF to dark mode\n";
        return 1;
    }
    
    std::cout << "Success! Dark mode PDF saved to: " << outputFile << "\n";
    return 0;
}
