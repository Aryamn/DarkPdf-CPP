cmake_minimum_required(VERSION 3.12)

# Create test executable
add_executable(darkpdf_tests
    test_color_scheme.cpp
    test_color_inverter.cpp
    test_main.cpp
)

# Link with Google Test and our library
target_link_libraries(darkpdf_tests PRIVATE
    darkpdf_lib
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
)

# Include test directories
target_include_directories(darkpdf_tests PRIVATE
    ../src
    ${POPPLER_INCLUDE_DIR}
    ${CAIRO_INCLUDE_DIR}
)

# Discover and register tests
include(GoogleTest)
gtest_discover_tests(darkpdf_tests)
