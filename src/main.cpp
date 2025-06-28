#include <iostream>
#include <string>
#include "pdf_processor.h"

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <input.pdf> <output.pdf>" << "\n";
    std::cout << "Converts a PDF to dark mode by inverting colors." << "\n";
}

void useFastIO()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
}

int main(int argc, char* argv[]) {
    
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string inputPath = argv[1];
    std::string outputPath = argv[2];
    
    std::cout << "DarkPdf - PDF Dark Mode Converter" << "\n";
    std::cout << "Input: " << inputPath << "\n";
    std::cout << "Output: " << outputPath << "\n";
    std::cout << "\n";
    
    PdfProcessor processor;
    
    std::cout << "Loading PDF..." << "\n";
    if (!processor.loadPdf(inputPath)) {
        std::cerr << "Error: Failed to load PDF file: " << inputPath << "\n";
        return 1;
    }
    
    std::cout << "Converting to dark mode..." << "\n";
    if (!processor.convertToDarkMode(outputPath)) {
        std::cerr << "Error: Failed to convert PDF to dark mode" << "\n";
        return 1;
    }
    
    std::cout << "Success! Dark mode PDF saved to: " << outputPath << "\n";
    return 0;
}
