# EpiGimp Test Suite

A comprehensive unit test suite for the EpiGimp paint application using Google Test framework.

## Quick Start

### Prerequisites
- CMake 3.14+
- C++17 compatible compiler
- Internet connection (for Google Test download)

### Build and Run Tests
```bash
# Navigate to project directory
cd /path/to/my_GIMP

# Create and navigate to build directory
mkdir -p build
cd build

# Configure project with CMake
cmake ..

# Build project and tests
make -j$(nproc)

# Run all tests (excluding Raylib integration tests)
./EpiGimpTests --gtest_filter=-BasicTest*
```

## Test Organization

### Test Files Structure
```
tests/
├── test_simple.cpp              # Basic utility functions (8 tests)
├── test_history.cpp             # Basic HistoryManager tests (1 test)
├── test_history_comprehensive.cpp  # Comprehensive HistoryManager tests (12 tests)
├── test_file_utils.cpp          # File system operations (11 tests)
├── test_canvas_utils.cpp        # Graphics and canvas utilities (11 tests)
└── test_basic.cpp              # Raylib integration tests (disabled)
```

### Test Categories

#### Core Command System (13 tests total)
- **Command Pattern**: Tests for ICommand interface implementation
- **History Management**: Undo/redo functionality validation
- **Stack Operations**: Command history limits and management

#### File Operations (11 tests)
- **File I/O**: Create, read, write, delete operations
- **Path Utilities**: Extension extraction, basename operations
- **Image Detection**: File type validation for supported formats
- **Directory Operations**: Folder creation, listing, navigation

#### Graphics Utilities (11 tests)
- **Color Operations**: Creation, interpolation, blending
- **Vector Math**: 2D vector operations and calculations
- **Geometry**: Rectangle operations, collision detection
- **Drawing Algorithms**: Line drawing, coordinate transformations

#### Basic Utilities (8 tests)
- **String Operations**: Manipulation and validation
- **Math Functions**: Basic mathematical operations
- **Container Operations**: STL container usage patterns
- **System Utilities**: File system and path operations

## Test Execution Options

### Run All Tests
```bash
./EpiGimpTests --gtest_filter=-BasicTest*
```

### Run Specific Test Suite
```bash
# History Manager tests only
./EpiGimpTests --gtest_filter=HistoryManagerTest*

# File utility tests only  
./EpiGimpTests --gtest_filter=FileUtilsTest*

# Canvas utility tests only
./EpiGimpTests --gtest_filter=CanvasUtilsTest*
```

### Run Single Test
```bash
# Run a specific test
./EpiGimpTests --gtest_filter=HistoryManagerTest.ExecuteCommand
```

### Verbose Output
```bash
# Show detailed test output
./EpiGimpTests --gtest_filter=-BasicTest* --gtest_verbose
```

### Test Result Formatting
```bash
# Generate XML output for CI systems
./EpiGimpTests --gtest_filter=-BasicTest* --gtest_output=xml:test_results.xml

# Generate JSON output
./EpiGimpTests --gtest_filter=-BasicTest* --gtest_output=json:test_results.json
```

## Current Test Status

### ✅ Passing Tests: 43/46 (93.5% success rate)

**Fully Passing Test Suites:**
- SimpleFunctionTest (3/3)
- SimpleTest (4/4)  
- UtilityTest (2/2)
- ColorTest (1/1)
- MathTest (1/1)
- FileSystemTest (1/1)

**Mostly Passing Test Suites:**
- HistoryManagerTest (11/12) ⚠️ 1 failing
- CanvasUtilsTest (10/11) ⚠️ 1 failing
- FileUtilsTest (10/11) ⚠️ 1 failing

### ⚠️ Known Issues

#### 1. HistoryManagerTest.MaxHistorySize
- **Issue**: Expected 5 commands in history, got 3
- **Status**: Investigation needed on HistoryManager implementation

#### 2. CanvasUtilsTest.ColorBlending  
- **Issue**: Alpha blending calculation mismatch
- **Status**: Blending algorithm needs review

#### 3. FileUtilsTest.FileExtensionExtraction
- **Issue**: Hidden files treated as extensions
- **Status**: Edge case handling needed

### 🚫 Disabled Tests

#### BasicTest Suite (Raylib Integration)
- **Issue**: Segmentation fault in headless mode
- **Reason**: OpenGL context issues in test environment
- **Status**: Alternative testing approach under consideration

## Development Workflow

### Adding New Tests

1. **Create test file** in `tests/` directory:
```cpp
#include <gtest/gtest.h>
// Your includes here

class YourTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup
    }
    
    void TearDown() override {
        // Test cleanup  
    }
};

TEST_F(YourTestFixture, TestName) {
    // Your test implementation
    EXPECT_EQ(expected, actual);
}
```

2. **Rebuild project**:
```bash
cd build
make -j$(nproc)
```

3. **Run new tests**:
```bash
./EpiGimpTests --gtest_filter=YourTestFixture*
```

### Test-Driven Development

1. **Write failing test first**
2. **Implement minimal code to pass**
3. **Refactor while keeping tests green**
4. **Repeat cycle**

### Mock Objects

The test suite includes mock implementations for testing:

```cpp
// Example: MockCommand for HistoryManager testing
class MockCommand : public ICommand {
    // Mock implementation with test helpers
    bool wasExecuted() const;
    bool wasUndone() const;
};
```

## Continuous Integration

### GitHub Actions Example
```yaml
name: Tests
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    - name: Build and Test
      run: |
        mkdir build && cd build
        cmake ..
        make -j$(nproc)
        ./EpiGimpTests --gtest_filter=-BasicTest* --gtest_output=xml:test_results.xml
```

### Test Reporting
- XML output compatible with Jenkins, GitHub Actions
- JSON output for custom reporting tools
- Console output with colored results

## Debugging Tests

### Running Under GDB
```bash
gdb ./EpiGimpTests
(gdb) run --gtest_filter=FailingTest*
```

### Memory Leak Detection
```bash
valgrind --leak-check=full ./EpiGimpTests --gtest_filter=-BasicTest*
```

### Code Coverage
```bash
# Build with coverage flags
cmake -DCMAKE_CXX_FLAGS="--coverage" ..
make -j$(nproc)

# Run tests and generate coverage report
./EpiGimpTests --gtest_filter=-BasicTest*
gcov ../src/*.cpp
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

## Best Practices

### Test Naming
- Use descriptive test names: `TEST(ComponentTest, ShouldDoSomethingWhenCondition)`
- Group related tests in test fixtures
- Use consistent naming patterns

### Test Organization
- One test file per component/module
- Separate unit tests from integration tests
- Keep tests independent and isolated

### Assertions
- Use appropriate assertion macros:
  - `EXPECT_EQ` for equality
  - `EXPECT_TRUE`/`EXPECT_FALSE` for booleans
  - `EXPECT_FLOAT_EQ` for floating point
  - `EXPECT_THROW` for exception testing

### Test Data
- Use minimal test data
- Prefer generated data over fixed files
- Clean up test artifacts in `TearDown()`

## Troubleshooting

### Common Build Issues

**CMake can't find Google Test:**
- Ensure internet connection for FetchContent
- Check CMake version >= 3.14

**Compilation errors:**
- Verify C++17 compiler support
- Check include paths in CMakeLists.txt

**Link errors:**
- Ensure EpiGimpLib target includes all necessary sources
- Check library dependencies

### Common Runtime Issues

**Segmentation faults:**
- Run under GDB for stack trace
- Check for null pointer dereferences
- Verify object lifetime management

**Test failures:**
- Check test expectations vs actual behavior
- Verify test setup/teardown
- Use `--gtest_verbose` for detailed output

**Permission errors:**
- Ensure write permissions for temp directories
- Check file system access rights

## Contributing

### Code Review Checklist
- [ ] Tests cover new functionality
- [ ] Tests follow naming conventions
- [ ] No test dependencies or ordering issues
- [ ] Mock objects used appropriately
- [ ] Test cleanup properly implemented
- [ ] Documentation updated

### Submitting Tests
1. Create tests for your changes
2. Ensure all tests pass locally
3. Update documentation if needed
4. Submit pull request with test coverage info

---

For more information, see [TEST_SUMMARY.md](TEST_SUMMARY.md) for detailed test coverage analysis.