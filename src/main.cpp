#include <iostream>
#include <string>
#include "pdf_processor.h"

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <input.pdf> <output.pdf>" << std::endl;
    std::cout << "Converts a PDF to dark mode by inverting colors." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string inputPath = argv[1];
    std::string outputPath = argv[2];
    
    std::cout << "DarkPdf - PDF Dark Mode Converter" << std::endl;
    std::cout << "Input: " << inputPath << std::endl;
    std::cout << "Output: " << outputPath << std::endl;
    std::cout << std::endl;
    
    PdfProcessor processor;
    
    // Load the PDF
    std::cout << "Loading PDF..." << std::endl;
    if (!processor.loadPdf(inputPath)) {
        std::cerr << "Error: Failed to load PDF file: " << inputPath << std::endl;
        return 1;
    }
    
    // Convert to dark mode
    std::cout << "Converting to dark mode..." << std::endl;
    if (!processor.convertToDarkMode(outputPath)) {
        std::cerr << "Error: Failed to convert PDF to dark mode" << std::endl;
        return 1;
    }
    
    std::cout << "Success! Dark mode PDF saved to: " << outputPath << std::endl;
    return 0;
}
