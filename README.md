# DarkPdf - PDF Dark Mode Converter

A C++ application that converts regular PDFs to dark mode by intelligently inverting colors while preserving readability.

## Features

- Converts PDF pages to dark mode by inverting colors
- Smart color inversion that preserves text contrast
- Maintains PDF structure and quality
- Cross-platform support (Windows, Linux, macOS)

## Dependencies

This project requires the following libraries:

### Windows (using vcpkg)
```powershell
# Install vcpkg if you haven't already
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install required packages
.\vcpkg install poppler:x64-windows cairo:x64-windows
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install libpoppler-cpp-dev libcairo2-dev cmake build-essential pkg-config
```

### macOS (using Homebrew)
```bash
brew install poppler cairo cmake pkg-config
```

## Building

### Windows with vcpkg
```powershell
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### Linux/macOS
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## Usage

```bash
./darkpdf input.pdf output.pdf
```

### Example
```bash
./darkpdf document.pdf document_dark.pdf
```

## How It Works

1. **PDF Loading**: Uses Poppler library to load and parse PDF files
2. **Page Rendering**: Renders each PDF page to a high-resolution image
3. **Color Inversion**: Applies intelligent color inversion:
   - Preserves hue for colored elements
   - Inverts brightness/luminosity
   - Maintains text readability
4. **PDF Generation**: Creates a new PDF with the processed pages

## Color Inversion Algorithm

The application uses a smart color inversion approach:

- **Grayscale content**: Full RGB inversion for maximum contrast
- **Colored content**: HSV-based inversion that preserves hue while inverting brightness
- **Text preservation**: Maintains high contrast between text and background

## Project Structure

```
DarkPdf/
├── src/
│   ├── main.cpp           # Main application entry point
│   ├── pdf_processor.cpp  # PDF loading and processing logic
│   ├── pdf_processor.h
│   ├── color_inverter.cpp # Color inversion algorithms
│   └── color_inverter.h
├── CMakeLists.txt         # Build configuration
└── README.md             # This file
```

## Limitations

- Currently processes all pages (no page range selection)
- Output quality depends on rendering DPI (currently set to 150)
- Password-protected PDFs are not supported
- Large PDFs may require significant memory

## Future Enhancements

- [ ] Page range selection
- [ ] Adjustable DPI settings
- [ ] Password-protected PDF support
- [ ] Batch processing
- [ ] Custom color schemes
- [ ] GUI interface
- [ ] Progress indication for large files

## Contributing

Feel free to submit issues and enhancement requests!

## License

This project is open source. Please check the licenses of the dependencies (Poppler, Cairo) for their respective terms.
