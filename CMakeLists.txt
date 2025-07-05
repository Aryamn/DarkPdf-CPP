cmake_minimum_required(VERSION 3.12)
project(DarkPdf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Find CLI11 using vcpkg
find_package(CLI11 CONFIG REQUIRED)

# Find Google Test
find_package(GTest CONFIG REQUIRED)


# Simple vcpkg paths for manual libraries
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

# Find CLI11 - remove manual finding since we use find_package
# CLI11 is header-only and handled by find_package above

# Create executable
add_executable(darkpdf
    src/main.cpp
    src/pdf_processor.cpp
    src/color_inverter.cpp
    src/color_scheme.cpp
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
    CLI11::CLI11
    user32
    gdi32
    msimg32
)


message(STATUS "Build configured for ${CMAKE_BUILD_TYPE}")

# Create a library for testable components
add_library(darkpdf_lib STATIC
    src/color_scheme.cpp
    src/color_inverter.cpp
    src/pdf_processor.cpp
)

target_include_directories(darkpdf_lib PUBLIC 
    src
    ${POPPLER_INCLUDE_DIR}
    ${CAIRO_INCLUDE_DIR}
)

target_link_libraries(darkpdf_lib PUBLIC
    ${POPPLER_CPP_LIBRARY}
    ${POPPLER_LIBRARY}
    ${CAIRO_LIBRARY}
    CLI11::CLI11
    user32
    gdi32
    msimg32
)

# Update main executable to use the library
target_link_libraries(darkpdf PRIVATE darkpdf_lib)

# Add tests
add_subdirectory(tests)