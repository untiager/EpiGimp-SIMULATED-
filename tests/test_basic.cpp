#include <gtest/gtest.h>
#include <raylib.h>
#include <filesystem>
#include <vector>
#include <algorithm>
#include "test_globals.hpp"

// Test that basic raylib functionality works
class BasicTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Raylib is already initialized by GlobalTestEnvironment
    }
};

// Test fixture for file system operations
class FileSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup if needed
    }
    
    void TearDown() override {
        // Cleanup if needed
    }
};

TEST_F(BasicTest, RaylibInitialization) {
    EXPECT_TRUE(IsWindowReady());
    EXPECT_FALSE(WindowShouldClose());
}

TEST_F(BasicTest, BasicColorOperations) {
    Color red = {255, 0, 0, 255};
    Color blank = {0, 0, 0, 0};
    
    EXPECT_EQ(red.r, (unsigned char)255);
    EXPECT_EQ(red.g, (unsigned char)0);
    EXPECT_EQ(red.b, (unsigned char)0);
    EXPECT_EQ(red.a, (unsigned char)255);
    
    EXPECT_EQ(blank.r, (unsigned char)0);
    EXPECT_EQ(blank.g, (unsigned char)0);
    EXPECT_EQ(blank.b, (unsigned char)0);
    EXPECT_EQ(blank.a, (unsigned char)0);
}

TEST_F(BasicTest, BasicMathOperations) {
    Vector2 v1 = {10.0f, 20.0f};
    Vector2 v2 = {5.0f, 10.0f};
    
    EXPECT_FLOAT_EQ(v1.x, 10.0f);
    EXPECT_FLOAT_EQ(v1.y, 20.0f);
    EXPECT_FLOAT_EQ(v2.x, 5.0f);
    EXPECT_FLOAT_EQ(v2.y, 10.0f);
}

TEST_F(FileSystemTest, BasicFileOperations) {
    namespace fs = std::filesystem;
    
    // Test basic filesystem functionality  
    fs::path currentPath = fs::current_path();
    EXPECT_FALSE(currentPath.empty());
    
    // Test temp directory
    fs::path tempDir = fs::temp_directory_path();
    EXPECT_TRUE(fs::exists(tempDir));
    EXPECT_TRUE(fs::is_directory(tempDir));
}

// Simple function tests without raylib dependency
TEST(SimpleFunctionTest, BasicStringOperations) {
    std::string test = "hello world";
    EXPECT_EQ(test.length(), 11);
    EXPECT_TRUE(test.find("world") != std::string::npos);
    
    // Test that starts_with equivalent works (C++17 compatible)
    std::string prefix = "hello";
    bool starts_with = test.substr(0, prefix.length()) == prefix;
    EXPECT_TRUE(starts_with);
}

TEST(SimpleFunctionTest, BasicMathOperations) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_FLOAT_EQ(3.14159f, 3.14159f);
    EXPECT_GT(10, 5);
    EXPECT_LT(3, 7);
}

TEST(SimpleFunctionTest, BasicContainerOperations) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    EXPECT_EQ(numbers.size(), 5);
    EXPECT_EQ(numbers[0], 1);
    EXPECT_EQ(numbers[4], 5);
    
    // Test find
    auto it = std::find(numbers.begin(), numbers.end(), 3);
    EXPECT_NE(it, numbers.end());
    EXPECT_EQ(*it, 3);
}