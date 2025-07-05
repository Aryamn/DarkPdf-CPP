#include <gtest/gtest.h>

// Test main entry point
// Google Test will automatically provide main() function
// This file can be used for global test setup if needed

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
