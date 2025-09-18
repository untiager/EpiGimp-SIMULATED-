# EpiGimp - Modern C++ Paint Interface

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/untiager/EpiGimp-SIMULATED-)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

A modern, lightweight paint application built with C++17 and Raylib. EpiGimp provides a clean, intuitive interface for basic image editing operations with a focus on performance and extensibility.

![EpiGimp Screenshot](docs/screenshot.png)

## 🎯 Project Status

**Current Version**: 1.0.0 (Active Development)

**Core Features Implemented**:
- ✅ Modern C++ architecture with RAII resource management
- ✅ Clean paint interface with toolbar and canvas
- ✅ Safe in-application file browser (no system crashes!)
- ✅ Load Image functionality (PNG, JPG, BMP, GIF, TGA, TIFF)
- ✅ Save Image functionality with custom filename/location
- ✅ Zoom and pan controls for image navigation
- ✅ Event-driven architecture for easy extensibility
- ✅ Zero compiler warnings (clean production code)

**Upcoming Features**:
- 🚧 Basic drawing tools (brush, pencil, eraser)
- 🚧 Color palette and picker
- 🚧 Layers support
- 🚧 Filters and effects
- 🚧 Undo/Redo system

## 🛠️ Technical Architecture

EpiGimp follows modern C++ best practices with a clean, modular architecture:

### Core Components
- **Application**: Main orchestrator managing all components
- **Canvas**: Image display and manipulation area with RAII resource management
- **Toolbar**: UI toolbar with extensible button system
- **FileBrowser**: Safe in-application file navigation (no dangerous system calls)

### Key Design Patterns
- **RAII**: Automatic resource cleanup for textures and images
- **Event System**: Type-safe event handling for component communication
- **Interface-Based Design**: Abstract interfaces for testability and modularity
- **Dependency Injection**: Clean separation of concerns

### Technology Stack
- **Language**: C++17
- **Graphics**: Raylib 5.5
- **Build System**: CMake 3.10+
- **Architecture**: Modern C++ with smart pointers and RAII

## 📋 Requirements

### System Requirements
- **OS**: Linux (Ubuntu/Arch/Fedora), Windows 10+, macOS 10.14+
- **RAM**: 256MB minimum
- **Graphics**: OpenGL 3.3 compatible graphics card

### Build Requirements
- **Compiler**: GCC 7+ / Clang 6+ / MSVC 2017+
- **CMake**: 3.10 or later
- **Raylib**: 5.0 or later

## 🚀 Installation

### 1. Install Dependencies

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake libraylib-dev git
```

#### Arch Linux
```bash
sudo pacman -S base-devel cmake raylib git
```

#### Fedora
```bash
sudo dnf install gcc-c++ cmake raylib-devel git
```

### 2. Clone and Build

```bash
# Clone the repository
git clone https://github.com/untiager/EpiGimp-SIMULATED-.git
cd EpiGimp-SIMULATED-

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
make

# Run the application
./EpiGimp
```

### 3. Optional: Install to System

```bash
sudo make install  # Installs to /usr/local/bin
```

## 🎮 Usage

### Basic Controls
- **Load Image**: Click "Load Image" button or use `Ctrl+O`
- **Save Image**: Click "Save Image" button or use `Ctrl+S`
- **Navigation**: 
  - Pan: Click and drag image
  - Zoom: Mouse wheel or `+`/`-` keys
  - Reset view: `Ctrl+0`
- **Exit**: `Escape` key or close window

### File Browser
- Navigate directories by clicking folder icons
- Use ".." to go up one directory level
- Hover over items for visual feedback
- ESC cancels any dialog

### Command Line
```bash
# Start with a specific image
./EpiGimp path/to/image.png

# Start empty
./EpiGimp
```

## 🏗️ Project Structure

```
EpiGimp/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── src/                    # Source code
│   ├── main.cpp           # Application entry point
│   ├── Core/              # Core application logic
│   │   └── Application.cpp
│   ├── UI/                # User interface components
│   │   ├── Canvas.cpp
│   │   └── Toolbar.cpp
│   └── Utils/             # Utility classes
│       └── FileBrowser.cpp
├── include/               # Header files
│   ├── Core/              # Core interfaces and classes
│   ├── UI/                # UI component headers
│   └── Utils/             # Utility headers
├── build/                 # Build output (generated)
└── png/                   # Test images
    └── test.png
```

## 🧪 Testing

Currently, testing is done manually. Automated testing framework coming soon!

### Manual Testing
```bash
# Build and run
make && ./EpiGimp

# Test with sample image
./EpiGimp png/test.png

# Test file operations
# 1. Click Load Image, navigate and select a file
# 2. Click Save Image, choose location and filename
# 3. Verify image loads and saves correctly
```

## 🐛 Known Issues

- File browser currently shows all directories in save mode (directories are for navigation only)
- No drawing tools yet - this is a planned feature
- Window resizing not fully optimized

## 📈 Performance

EpiGimp is designed for performance:
- **Memory**: Automatic resource cleanup prevents memory leaks
- **Rendering**: Hardware-accelerated via Raylib/OpenGL
- **Loading**: Efficient image loading with proper scaling
- **UI**: 60 FPS responsive interface

## 🤝 How to Contribute

We welcome contributions from developers of all skill levels! Here's how you can help make EpiGimp better:

### 🌟 Getting Started

1. **Fork the Repository**
   ```bash
   # Click "Fork" on GitHub, then:
   git clone https://github.com/YOUR_USERNAME/EpiGimp-SIMULATED-.git
   cd EpiGimp-SIMULATED-
   git remote add upstream https://github.com/untiager/EpiGimp-SIMULATED-.git
   ```

2. **Set Up Development Environment**
   ```bash
   # Install dependencies (see Installation section)
   # Build the project to ensure everything works
   mkdir build && cd build
   cmake .. && make
   ./EpiGimp  # Test that it runs
   ```

### 📝 Contribution Guidelines

#### Code Style
- **Language**: Modern C++17 features encouraged
- **Formatting**: Follow existing code style (4 spaces, no tabs)
- **Naming**: 
  - Classes: `PascalCase` (e.g., `FileBrowser`)
  - Functions/Variables: `camelCase` (e.g., `loadImage`)
  - Constants: `UPPER_SNAKE_CASE`
- **Comments**: Use `//` for single-line, `/* */` for multi-line
- **Headers**: Include guards using `#ifndef/#define/#endif`

#### Architecture Principles
- **RAII**: All resources should be automatically managed
- **Single Responsibility**: Each class should have one clear purpose
- **Interface Segregation**: Use abstract interfaces where appropriate
- **No Memory Leaks**: Use smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- **Exception Safety**: Handle errors gracefully

### 🎯 Areas We Need Help

#### 🔴 High Priority
- **Drawing Tools**: Implement brush, pencil, eraser tools
- **Color System**: Color picker and palette implementation
- **Unit Tests**: Add automated testing framework
- **Documentation**: Code documentation and tutorials

#### 🟡 Medium Priority
- **Layers**: Layer management system
- **Filters**: Basic image filters (blur, sharpen, etc.)
- **Undo/Redo**: Command pattern implementation
- **Cross-platform**: Windows and macOS support

#### 🟢 Good First Issues
- **UI Improvements**: Better button styles, icons
- **File Format Support**: Add more image formats
- **Keyboard Shortcuts**: Implement more hotkeys
- **Settings**: Configuration file support

### 🔄 Development Workflow

1. **Create Feature Branch**
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/bug-description
   ```

2. **Make Your Changes**
   - Write clean, documented code
   - Follow the existing architecture patterns
   - Test your changes thoroughly

3. **Commit Guidelines**
   ```bash
   # Use conventional commits format:
   git commit -m "feat: add brush tool implementation"
   git commit -m "fix: resolve file dialog crash on Windows"
   git commit -m "docs: update README with new features"
   ```

4. **Update and Push**
   ```bash
   git fetch upstream
   git rebase upstream/main
   git push origin feature/your-feature-name
   ```

5. **Create Pull Request**
   - Use a clear, descriptive title
   - Explain what your PR does and why
   - Reference any related issues
   - Include screenshots for UI changes

### 📋 Pull Request Checklist

- [ ] Code compiles without warnings
- [ ] Follows project code style
- [ ] Includes appropriate comments
- [ ] No memory leaks (tested with valgrind if possible)
- [ ] Maintains backward compatibility
- [ ] Updates documentation if needed
- [ ] Adds tests for new functionality (when testing framework exists)

### 🐛 Reporting Bugs

**Before Creating an Issue:**
- Check existing issues to avoid duplicates
- Test with the latest version
- Provide clear steps to reproduce

**Bug Report Template:**
```markdown
**Bug Description**
Clear description of what went wrong.

**Steps to Reproduce**
1. Step one
2. Step two
3. Expected vs actual behavior

**Environment**
- OS: [e.g., Ubuntu 22.04]
- Compiler: [e.g., GCC 11.2]
- Raylib Version: [e.g., 5.5]
- EpiGimp Version: [e.g., 1.0.0]

**Additional Context**
Screenshots, logs, or other helpful information.
```

### 💡 Feature Requests

We love new ideas! Before suggesting a feature:
- Check if it's already planned or requested
- Consider how it fits with the project goals
- Think about implementation complexity

### 🎉 Recognition

Contributors will be:
- Listed in the CONTRIBUTORS.md file
- Mentioned in release notes
- Given credit in the about dialog (planned feature)

### 📞 Getting Help

- **GitHub Issues**: For bugs and feature requests
- **Discussions**: For questions and general discussion
- **Discord**: [Coming soon] For real-time chat

### 🌍 Community Guidelines

- Be respectful and inclusive
- Help newcomers get started
- Provide constructive feedback
- Follow the code of conduct

Thank you for contributing to EpiGimp! Every contribution, no matter how small, helps make this project better. 🚀

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Raylib**: Amazing graphics library that makes C++ graphics accessible
- **Contributors**: Everyone who has contributed code, ideas, and feedback
- **Community**: Users who test and provide valuable feedback

---

**Made with ❤️ using Modern C++17**