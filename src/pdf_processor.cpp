#include "pdf_processor.h"
#include "color_inverter.h"
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <cairo.h>
#include <fstream>
#include <filesystem>
#include <cairo-pdf.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <future>
#include <thread>

// Calculating Memory Usage
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>

void printMemoryUsage(const std::string& checkpoint) {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    
    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
    SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
    
    std::cout << "[" << checkpoint << "] Memory Usage:" << std::endl;
    std::cout << "  Physical Memory: " << (physMemUsedByMe / 1024 / 1024) << " MB" << std::endl;
    std::cout << "  Virtual Memory: " << (virtualMemUsedByMe / 1024 / 1024) << " MB" << std::endl;
    std::cout << "  Peak Working Set: " << (pmc.PeakWorkingSetSize / 1024 / 1024) << " MB" << std::endl;
}
#endif
//

PdfProcessor::PdfProcessor() : document_(nullptr), pageCount_(0) {
}

PdfProcessor::~PdfProcessor() {
}

bool PdfProcessor::loadPdf(const std::string& inputPath) {
    std::filesystem::path filePath(inputPath);
    std::filesystem::path absolutePath = std::filesystem::absolute(filePath);
    std::string absolutePathStr = absolutePath.string();
    
    if (!std::filesystem::exists(absolutePath)) {
        std::cerr << "Error: File does not exist: " << absolutePathStr << "\n";
        return false;
    }

    try {
        const std::string& pathRef = absolutePathStr;
        document_ = std::unique_ptr<poppler::document>(
            poppler::document::load_from_file(pathRef.c_str())
        );
    } catch (const std::exception& e) {
        std::cerr << "Error: Exception while loading PDF: " << e.what() << "\n";
        return false;
    }
    
    if (!document_) {
        std::cerr << "Error: Could not load PDF document" << "\n";
        return false;
    }
    
    if (document_->is_locked()) {
        std::cerr << "Error: PDF is password protected" << "\n";
        return false;
    }
    
    pageCount_ = document_->pages();
    std::cout << "Loaded PDF with " << pageCount_ << " pages" << "\n";
    
    // Display the DPI that will be used
    int dpi = calculateOptimalDPI();
    std::cout << "Using DPI: " << dpi << " (threshold: " << PAGE_THRESHOLD << " pages)" << "\n";
    
    return true;
}

bool PdfProcessor::convertToDarkMode(const std::string& outputPath, const ColorScheme& scheme) {
    if (!document_) {
        std::cerr << "Error: No PDF loaded" << "\n";
        return false;
    }
    
    std::unique_ptr<poppler::page> firstPage(document_->create_page(0));
    if (!firstPage) {
        std::cerr << "Error: Could not access first page" << "\n";
        return false;
    }
    
    poppler::rectf pageRect = firstPage->page_rect();
    double pageWidth = pageRect.width();
    double pageHeight = pageRect.height();
    
    std::cout << "Page dimensions: " << pageWidth << "x" << pageHeight << "\n";
    
    cairo_surface_t* pdfSurface = cairo_pdf_surface_create(
        outputPath.c_str(), pageWidth, pageHeight
    );
    
    if (cairo_surface_status(pdfSurface) != CAIRO_STATUS_SUCCESS) {
        std::cerr << "Error: Could not create output PDF surface" << "\n";
        cairo_surface_destroy(pdfSurface);
        return false;
    }
    
    cairo_t* pdfContext = cairo_create(pdfSurface);
    
    poppler::page_renderer renderer;
    renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
    renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);
    renderer.set_render_hint(poppler::page_renderer::text_hinting, true);

    // Process all pages asynchronously with the selected scheme
    std::vector<std::future<cairo_surface_t*>> futures;
    
    for (int i = 0; i < pageCount_; ++i) {
        futures.push_back(std::async(std::launch::async, [this, i, &renderer, &scheme]() {
            std::unique_ptr<poppler::page> page(document_->create_page(i));
            if (!page) {
                throw std::runtime_error("Could not access page " + std::to_string(i));
            }
            return processPage(renderer, page.get(), i, scheme);
        }));
    }
     
    for (int i = 0; i < pageCount_; ++i) 
    {        
        try
        {
            cairo_save(pdfContext);

            auto processedPage = futures[i].get();

            int surfaceWidth = cairo_image_surface_get_width(processedPage);
            int surfaceHeight = cairo_image_surface_get_height(processedPage);
            
            double scaleX = pageWidth / surfaceWidth;
            double scaleY = pageHeight / surfaceHeight;
            cairo_scale(pdfContext, scaleX, scaleY);
            
            cairo_set_source_surface(pdfContext, processedPage, 0, 0);
            cairo_paint(pdfContext);
            
            cairo_restore(pdfContext);
            cairo_surface_destroy(processedPage);
            cairo_show_page(pdfContext);

            if(i%10==0)
            {
                printMemoryUsage("After page " + std::to_string(i));
            }

            std::cout << "Processed page " << (i + 1) << "/" << pageCount_ << "\n";

        }
        catch (const std::exception& e)
        {
            std::cerr << "Error processing page " << i << ": " << e.what() << "\n";
        }
    }
    
    cairo_destroy(pdfContext);
    cairo_surface_destroy(pdfSurface);
    
    return true;
}

int PdfProcessor::calculateOptimalDPI() const {
    if (pageCount_ > PAGE_THRESHOLD) {
        return LOW_DPI;   // Use 90 DPI for large PDFs
    } else {
        return HIGH_DPI;  // Use 130 DPI for smaller PDFs
    }
}

cairo_surface_t* PdfProcessor::processPage(poppler::page_renderer& renderer, poppler::page* page, int pageIndex, const ColorScheme& scheme) {
    if (!page) {
        return nullptr;
    }
    
    int dpi = calculateOptimalDPI();
    poppler::image pageImage = renderer.render_page(page, dpi, dpi);
    
    if (!pageImage.is_valid()) {
        std::cerr << "Warning: Could not render page " << pageIndex << "\n";
        return nullptr;
    }
    
    int width = pageImage.width();
    int height = pageImage.height();
    
    cairo_surface_t* imageSurface = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, width, height
    );
    
    unsigned char* imageData = cairo_image_surface_get_data(imageSurface);
    const char* pageData = pageImage.const_data();
    int srcStride = pageImage.bytes_per_row();
    int dstStride = cairo_image_surface_get_stride(imageSurface);
    
    for (int y = 0; y < height; ++y) {
        memcpy(imageData + y * dstStride, 
               pageData + y * srcStride, 
               width * 4);
    }
    
    cairo_surface_mark_dirty(imageSurface);
    
    ColorInverter::invertColors(imageSurface,scheme);
    
    return imageSurface;
}
