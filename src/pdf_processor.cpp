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

PdfProcessor::PdfProcessor() : document_(nullptr), pageCount_(0) {
}

PdfProcessor::~PdfProcessor() {
}

bool PdfProcessor::loadPdf(const std::string& inputPath) {
    std::filesystem::path filePath(inputPath);
    std::filesystem::path absolutePath = std::filesystem::absolute(filePath);
    std::string absolutePathStr = absolutePath.string();
    
    if (!std::filesystem::exists(absolutePath)) {
        std::cerr << "Error: File does not exist: " << absolutePathStr << std::endl;
        return false;
    }

    try {
        const std::string& pathRef = absolutePathStr;
        document_ = std::unique_ptr<poppler::document>(
            poppler::document::load_from_file(pathRef.c_str())
        );
    } catch (const std::exception& e) {
        std::cerr << "Error: Exception while loading PDF: " << e.what() << std::endl;
        return false;
    }
    
    if (!document_) {
        std::cerr << "Error: Could not load PDF document" << std::endl;
        return false;
    }
    
    if (document_->is_locked()) {
        std::cerr << "Error: PDF is password protected" << std::endl;
        return false;
    }
    
    pageCount_ = document_->pages();
    std::cout << "Loaded PDF with " << pageCount_ << " pages" << std::endl;
    
    return true;
}

bool PdfProcessor::convertToDarkMode(const std::string& outputPath) {
    if (!document_) {
        std::cerr << "Error: No PDF loaded" << std::endl;
        return false;
    }
    
    std::unique_ptr<poppler::page> firstPage(document_->create_page(0));
    if (!firstPage) {
        std::cerr << "Error: Could not access first page" << std::endl;
        return false;
    }
    
    poppler::rectf pageRect = firstPage->page_rect();
    double pageWidth = pageRect.width();
    double pageHeight = pageRect.height();
    
    std::cout << "Page dimensions: " << pageWidth << "x" << pageHeight << std::endl;
    
    cairo_surface_t* pdfSurface = cairo_pdf_surface_create(
        outputPath.c_str(), pageWidth, pageHeight
    );
    
    if (cairo_surface_status(pdfSurface) != CAIRO_STATUS_SUCCESS) {
        std::cerr << "Error: Could not create output PDF surface" << std::endl;
        cairo_surface_destroy(pdfSurface);
        return false;
    }
    
    cairo_t* pdfContext = cairo_create(pdfSurface);
    
    poppler::page_renderer renderer;
    renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
    renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);
    
    for (int i = 0; i < pageCount_; ++i) {
        std::cout << "Processing page " << (i + 1) << "/" << pageCount_ << std::endl;
        
        std::unique_ptr<poppler::page> page(document_->create_page(i));
        if (!page) {
            std::cerr << "Warning: Could not access page " << i << std::endl;
            continue;
        }
        
        cairo_surface_t* processedSurface = processPage(renderer, page.get(), i);
        if (!processedSurface) {
            std::cerr << "Warning: Failed to process page " << i << std::endl;
            continue;
        }
                
        cairo_save(pdfContext);

        int surfaceWidth = cairo_image_surface_get_width(processedSurface);
        int surfaceHeight = cairo_image_surface_get_height(processedSurface);
        
        double scaleX = pageWidth / surfaceWidth;
        double scaleY = pageHeight / surfaceHeight;
        cairo_scale(pdfContext, scaleX, scaleY);
        
        cairo_set_source_surface(pdfContext, processedSurface, 0, 0);
        cairo_paint(pdfContext);
        
        cairo_restore(pdfContext);
        cairo_surface_destroy(processedSurface);
        cairo_show_page(pdfContext);
    }
    
    // Cleanup
    cairo_destroy(pdfContext);
    cairo_surface_destroy(pdfSurface);
    
    std::cout << "Conversion completed successfully!" << std::endl;
    return true;
}

cairo_surface_t* PdfProcessor::processPage(poppler::page_renderer& renderer, poppler::page* page, int pageIndex) {
    if (!page) {
        return nullptr;
    }
    
    // Render page to image
    poppler::image pageImage = renderer.render_page(page, DEFAULT_DPI, DEFAULT_DPI); // 150 DPI
    
    if (!pageImage.is_valid()) {
        std::cerr << "Warning: Could not render page " << pageIndex << std::endl;
        return nullptr;
    }
    
    int width = pageImage.width();
    int height = pageImage.height();
    
    cairo_surface_t* imageSurface = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, width, height
    );
    
    unsigned char* imageData = cairo_image_surface_get_data(imageSurface);
    const char* pageData = pageImage.const_data();
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (pageImage.format() == poppler::image::format_argb32) {
                int srcOffset = y * pageImage.bytes_per_row() + x * 4;
                int dstOffset = (y * width + x) * 4;
                
                imageData[dstOffset + 0] = pageData[srcOffset + 0]; 
                imageData[dstOffset + 1] = pageData[srcOffset + 1];
                imageData[dstOffset + 2] = pageData[srcOffset + 2];
                imageData[dstOffset + 3] = pageData[srcOffset + 3];
            }
        }
    }
    
    cairo_surface_mark_dirty(imageSurface);
    
    // Applied color inversion
    ColorInverter::invertColors(imageSurface);
    
    return imageSurface;
}
