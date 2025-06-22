cmake_minimum_required(VERSION 3.12)
project(DarkPdf)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Simple vcpkg paths
set(VCPKG_ROOT "C:/Users/aryamanjain/vcpkg")
set(VCPKG_INSTALLED_DIR "${VCPKG_ROOT}/installed/x64-windows")

message(STATUS "Using vcpkg: ${VCPKG_ROOT}")

# Find Poppler
find_path(POPPLER_INCLUDE_DIR
    NAMES poppler/cpp/poppler-document.h
    PATHS "${VCPKG_INSTALLED_DIR}/include"
)

find_library(POPPLER_LIBRARY NAMES poppler PATHS "${VCPKG_INSTALLED_DIR}/lib" "${VCPKG_INSTALLED_DIR}/debug/lib")
find_library(POPPLER_CPP_LIBRARY NAMES poppler-cpp PATHS "${VCPKG_INSTALLED_DIR}/lib" "${VCPKG_INSTALLED_DIR}/debug/lib")

# Find Cairo
find_path(CAIRO_INCLUDE_DIR
    NAMES cairo.h
    PATHS "${VCPKG_INSTALLED_DIR}/include/cairo"
)

find_library(CAIRO_LIBRARY NAMES cairo PATHS "${VCPKG_INSTALLED_DIR}/lib" "${VCPKG_INSTALLED_DIR}/debug/lib")

# Create executable
add_executable(darkpdf
    src/main.cpp
    src/pdf_processor.cpp
    src/color_inverter.cpp
)

# Include directories
target_include_directories(darkpdf PRIVATE 
    ${POPPLER_INCLUDE_DIR}
    ${CAIRO_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(darkpdf PRIVATE
    ${POPPLER_CPP_LIBRARY}
    ${POPPLER_LIBRARY}
    ${CAIRO_LIBRARY}
    user32
    gdi32
    msimg32
)


message(STATUS "Build configured for ${CMAKE_BUILD_TYPE}")