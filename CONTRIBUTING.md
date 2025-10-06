### 📝 Contribution Guidelines

#### Code Style
- **Language**: Modern C++17 features encouraged
- **Formatting**: Follow existing code style (4 spaces, no tabs)
- **File Size**: Keep source files under 200 lines for maintainability
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
- **Modular Design**: Split large files into focused components (follow existing pattern)

### 🎯 Areas We Need Help

#### 🔴 High Priority
- **Enhanced Drawing Tools**: Configurable brush sizes and colors, additional tool types
- **Color System**: Color picker and palette implementation  
- **Layer Operations**: Advanced layer blend modes and effects
- **Performance Optimization**: Rendering optimization for large canvases

#### 🟡 Medium Priority
- **Filters**: Basic image filters (blur, sharpen, brightness/contrast)
- **Advanced Drawing Tools**: Eraser, text tool, shape tools, gradients
- **Cross-platform**: Windows and macOS support testing
- **Plugin System**: Framework for extensible tools and filters

#### 🟢 Good First Issues
- **UI Improvements**: Better button styles, icons, tooltips
- **File Format Support**: Add WEBP, TIFF, or other formats
- **Keyboard Shortcuts**: Implement more hotkeys and accessibility
- **Settings**: Configuration file support for user preferences
- **Drawing Enhancements**: Add more colors, brush patterns, or effects
- **Documentation**: Improve code comments and user guides

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
- [ ] **All existing tests pass** (`./EpiGimpTests` runs successfully - 111/111 tests expected)
- [ ] **New functionality includes unit tests** (we have comprehensive Google Test framework)

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

### 🌍 Community Guidelines

- Be respectful and inclusive
- Help newcomers get started
- Provide constructive feedback
- Follow the code of conduct

Thank you for contributing to EpiGimp! Every contribution, no matter how small, helps make this project better. 🚀
