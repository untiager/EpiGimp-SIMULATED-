#include <gtest/gtest.h>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <string>
#include <cmath>

// Simple tests without raylib dependency
class SimpleTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Simple setup
    }
    
    void TearDown() override {
        // Simple cleanup
    }
};

TEST_F(SimpleTest, BasicStringOperations) {
    std::string test = "hello world";
    EXPECT_EQ(test.length(), 11);
    EXPECT_TRUE(test.find("world") != std::string::npos);
    
    // Test that starts_with equivalent works (C++17 compatible)
    std::string prefix = "hello";
    bool starts_with = test.substr(0, prefix.length()) == prefix;
    EXPECT_TRUE(starts_with);
}

TEST_F(SimpleTest, BasicMathOperations) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_FLOAT_EQ(3.14159f, 3.14159f);
    EXPECT_GT(10, 5);
    EXPECT_LT(3, 7);
}

TEST_F(SimpleTest, BasicContainerOperations) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    EXPECT_EQ(numbers.size(), 5);
    EXPECT_EQ(numbers[0], 1);
    EXPECT_EQ(numbers[4], 5);
    
    // Test find
    auto it = std::find(numbers.begin(), numbers.end(), 3);
    EXPECT_NE(it, numbers.end());
    EXPECT_EQ(*it, 3);
}

TEST_F(SimpleTest, BasicFileSystemOperations) {
    namespace fs = std::filesystem;
    
    // Test basic filesystem functionality  
    fs::path currentPath = fs::current_path();
    EXPECT_FALSE(currentPath.empty());
    
    // Test temp directory
    fs::path tempDir = fs::temp_directory_path();
    EXPECT_TRUE(fs::exists(tempDir));
    EXPECT_TRUE(fs::is_directory(tempDir));
}

// Test basic utility functions
TEST(UtilityTest, StringUtilities) {
    // Test string manipulation functions that might be used in the app
    std::string filename = "image.png";
    
    // Check extension
    size_t dot_pos = filename.find_last_of('.');
    std::string extension = (dot_pos != std::string::npos) ? filename.substr(dot_pos + 1) : "";
    EXPECT_EQ(extension, "png");
    
    // Check filename without extension
    std::string name_only = (dot_pos != std::string::npos) ? filename.substr(0, dot_pos) : filename;
    EXPECT_EQ(name_only, "image");
}

TEST(UtilityTest, PathUtilities) {
    namespace fs = std::filesystem;
    
    // Test path operations
    fs::path test_path = "test/path/file.txt";
    EXPECT_EQ(test_path.filename(), "file.txt");
    EXPECT_EQ(test_path.extension(), ".txt");
    EXPECT_EQ(test_path.stem(), "file");
}

// Test color validation (without raylib dependency)
TEST(ColorTest, ColorValidation) {
    struct TestColor {
        unsigned char r, g, b, a;
    };
    
    TestColor red = {255, 0, 0, 255};
    TestColor transparent = {0, 0, 0, 0};
    
    EXPECT_EQ(red.r, 255);
    EXPECT_EQ(red.g, 0);
    EXPECT_EQ(red.b, 0);
    EXPECT_EQ(red.a, 255);
    
    EXPECT_EQ(transparent.r, 0);
    EXPECT_EQ(transparent.g, 0);
    EXPECT_EQ(transparent.b, 0);
    EXPECT_EQ(transparent.a, 0);
}

// Test basic math operations for graphics
TEST(MathTest, Vector2Operations) {
    struct TestVector2 {
        float x, y;
    };
    
    TestVector2 v1 = {10.0f, 20.0f};
    TestVector2 v2 = {5.0f, 10.0f};
    
    EXPECT_FLOAT_EQ(v1.x, 10.0f);
    EXPECT_FLOAT_EQ(v1.y, 20.0f);
    EXPECT_FLOAT_EQ(v2.x, 5.0f);
    EXPECT_FLOAT_EQ(v2.y, 10.0f);
    
    // Test distance calculation
    float dx = v1.x - v2.x;
    float dy = v1.y - v2.y;
    float distance = sqrt(dx * dx + dy * dy);
    EXPECT_FLOAT_EQ(distance, sqrt(125.0f)); // sqrt((10-5)² + (20-10)²) = sqrt(25+100) = sqrt(125)
}