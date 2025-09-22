#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

// File utility functions that might exist in the project
class FileUtilsTest : public ::testing::Test {
protected:
    fs::path testDir_;
    
    void SetUp() override {
        // Create a temporary directory for testing
        testDir_ = fs::temp_directory_path() / "epi_gimp_test";
        fs::create_directories(testDir_);
    }
    
    void TearDown() override {
        // Clean up test directory
        if (fs::exists(testDir_)) {
            fs::remove_all(testDir_);
        }
    }
    
    void createTestFile(const fs::path& path, const std::string& content) {
        std::ofstream file(path);
        file << content;
        file.close();
    }
    
    bool fileExists(const fs::path& path) {
        return fs::exists(path) && fs::is_regular_file(path);
    }
    
    std::string readFileContent(const fs::path& path) {
        std::ifstream file(path);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return content;
    }
    
    bool isImageFile(const std::string& filename) {
        std::vector<std::string> imageExtensions = {".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tiff"};
        
        size_t dotPos = filename.find_last_of('.');
        if (dotPos == std::string::npos) return false;
        
        std::string extension = filename.substr(dotPos);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        return std::find(imageExtensions.begin(), imageExtensions.end(), extension) != imageExtensions.end();
    }
    
    std::string getFileExtension(const std::string& filename) {
        size_t dotPos = filename.find_last_of('.');
        // If no dot found, or dot is at the beginning (hidden file), no extension
        if (dotPos == std::string::npos || dotPos == 0) return "";
        return filename.substr(dotPos);
    }
    
    std::string getBaseName(const std::string& filename) {
        size_t dotPos = filename.find_last_of('.');
        return (dotPos != std::string::npos) ? filename.substr(0, dotPos) : filename;
    }
};

TEST_F(FileUtilsTest, BasicFileOperations) {
    auto testFile = testDir_ / "test.txt";
    std::string testContent = "Hello, World!";
    
    // Create file
    createTestFile(testFile, testContent);
    
    // Check if file exists
    EXPECT_TRUE(fileExists(testFile));
    
    // Read file content
    std::string content = readFileContent(testFile);
    EXPECT_EQ(content, testContent);
    
    // Remove file
    fs::remove(testFile);
    EXPECT_FALSE(fileExists(testFile));
}

TEST_F(FileUtilsTest, DirectoryOperations) {
    auto subDir = testDir_ / "subdir";
    
    // Create subdirectory
    EXPECT_TRUE(fs::create_directory(subDir));
    EXPECT_TRUE(fs::exists(subDir));
    EXPECT_TRUE(fs::is_directory(subDir));
    
    // List directory contents
    std::vector<fs::path> entries;
    for (const auto& entry : fs::directory_iterator(testDir_)) {
        entries.push_back(entry.path());
    }
    
    EXPECT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0].filename(), "subdir");
}

TEST_F(FileUtilsTest, ImageFileDetection) {
    EXPECT_TRUE(isImageFile("test.png"));
    EXPECT_TRUE(isImageFile("test.PNG"));
    EXPECT_TRUE(isImageFile("test.jpg"));
    EXPECT_TRUE(isImageFile("test.jpeg"));
    EXPECT_TRUE(isImageFile("test.bmp"));
    EXPECT_TRUE(isImageFile("test.gif"));
    EXPECT_TRUE(isImageFile("test.tiff"));
    
    EXPECT_FALSE(isImageFile("test.txt"));
    EXPECT_FALSE(isImageFile("test.doc"));
    EXPECT_FALSE(isImageFile("test"));
    EXPECT_FALSE(isImageFile(""));
}

TEST_F(FileUtilsTest, FileExtensionExtraction) {
    EXPECT_EQ(getFileExtension("test.png"), ".png");
    EXPECT_EQ(getFileExtension("test.txt"), ".txt");
    EXPECT_EQ(getFileExtension("file.with.multiple.dots.jpg"), ".jpg");
    EXPECT_EQ(getFileExtension("noextension"), "");
    EXPECT_EQ(getFileExtension(""), "");
    EXPECT_EQ(getFileExtension(".hidden"), "");
}

TEST_F(FileUtilsTest, BaseNameExtraction) {
    EXPECT_EQ(getBaseName("test.png"), "test");
    EXPECT_EQ(getBaseName("test.txt"), "test");
    EXPECT_EQ(getBaseName("file.with.multiple.dots.jpg"), "file.with.multiple.dots");
    EXPECT_EQ(getBaseName("noextension"), "noextension");
    EXPECT_EQ(getBaseName(""), "");
}

TEST_F(FileUtilsTest, PathManipulation) {
    fs::path testPath = "/home/user/images/photo.png";
    
    EXPECT_EQ(testPath.filename(), "photo.png");
    EXPECT_EQ(testPath.stem(), "photo");
    EXPECT_EQ(testPath.extension(), ".png");
    EXPECT_EQ(testPath.parent_path(), "/home/user/images");
}

TEST_F(FileUtilsTest, SafeFileCreation) {
    auto testFile = testDir_ / "new_file.txt";
    
    // Ensure file doesn't exist initially
    EXPECT_FALSE(fileExists(testFile));
    
    // Create file safely
    createTestFile(testFile, "test content");
    
    EXPECT_TRUE(fileExists(testFile));
    EXPECT_EQ(readFileContent(testFile), "test content");
}

TEST_F(FileUtilsTest, FileSizeCalculation) {
    auto testFile = testDir_ / "size_test.txt";
    std::string content = "This is a test file for size calculation.";
    
    createTestFile(testFile, content);
    
    auto fileSize = fs::file_size(testFile);
    EXPECT_EQ(fileSize, content.length());
}

TEST_F(FileUtilsTest, FilePermissions) {
    auto testFile = testDir_ / "permissions_test.txt";
    createTestFile(testFile, "test");
    
    // Check if file is readable
    auto perms = fs::status(testFile).permissions();
    bool readable = (perms & fs::perms::owner_read) != fs::perms::none;
    bool writable = (perms & fs::perms::owner_write) != fs::perms::none;
    
    EXPECT_TRUE(readable);
    EXPECT_TRUE(writable);
}

// Test file backup functionality
TEST_F(FileUtilsTest, FileBackup) {
    auto originalFile = testDir_ / "original.txt";
    auto backupFile = testDir_ / "original.txt.bak";
    std::string content = "original content";
    
    createTestFile(originalFile, content);
    
    // Create backup
    fs::copy_file(originalFile, backupFile);
    
    EXPECT_TRUE(fileExists(backupFile));
    EXPECT_EQ(readFileContent(backupFile), content);
    
    // Modify original
    createTestFile(originalFile, "modified content");
    
    // Verify backup still has original content
    EXPECT_EQ(readFileContent(backupFile), content);
    EXPECT_EQ(readFileContent(originalFile), "modified content");
}

// Test for handling special characters in filenames
TEST_F(FileUtilsTest, SpecialCharactersInFilenames) {
    // Test with spaces and underscores
    auto spaceFile = testDir_ / "file with spaces.txt";
    auto underscoreFile = testDir_ / "file_with_underscores.txt";
    
    createTestFile(spaceFile, "spaces");
    createTestFile(underscoreFile, "underscores");
    
    EXPECT_TRUE(fileExists(spaceFile));
    EXPECT_TRUE(fileExists(underscoreFile));
    
    EXPECT_EQ(readFileContent(spaceFile), "spaces");
    EXPECT_EQ(readFileContent(underscoreFile), "underscores");
}