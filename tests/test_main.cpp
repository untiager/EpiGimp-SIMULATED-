#include <gtest/gtest.h>
#include "test_globals.hpp"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Register the global test environment
    ::testing::AddGlobalTestEnvironment(new EpiGimp::GlobalTestEnvironment);
    
    return RUN_ALL_TESTS();
}