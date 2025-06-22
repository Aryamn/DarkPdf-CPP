#pragma once
#include <string>
#include <memory>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <cairo.h>

namespace poppler {
    class document;
    class page;
}

// can make this class as a singletion since, only one singletion should be there
class PdfProcessor {
public:
    PdfProcessor();
    ~PdfProcessor();
    
    bool loadPdf(const std::string& inputPath);
    bool convertToDarkMode(const std::string& outputPath);
    
private:
    std::unique_ptr<poppler::document> document_;
    int pageCount_;
    static const int DEFAULT_DPI = 150;
    cairo_surface_t* processPage(poppler::page_renderer& renderer ,poppler::page* page, int pageIndex);
    bool saveImageAsPNG(const poppler::image& image, const std::string& filename);    
};
