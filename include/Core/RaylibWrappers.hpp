//Modern RAII wrappers for raylib resources
#ifndef RAYLIB_WRAPPERS_HPP
#define RAYLIB_WRAPPERS_HPP

#include <memory>
#include <optional>
#include <string>
#include <filesystem>
#include <algorithm>
#include <vector>
#include "raylib.h"

namespace EpiGimp {

// RAII wrapper for raylib Texture2D
class TextureResource {
private:
    std::unique_ptr<Texture2D, void(*)(Texture2D*)> texture_;

public:
    TextureResource() : texture_(nullptr, [](Texture2D*){}) {}
    
    explicit TextureResource(Texture2D tex) 
        : texture_(new Texture2D(tex), [](Texture2D* t) {
            if (t && t->id != 0) {
                UnloadTexture(*t);
            }
            delete t;
        }) {}

    static std::optional<TextureResource> fromImage(const Image& image) {
        Texture2D tex = LoadTextureFromImage(image);
        if (tex.id == 0) {
            return std::nullopt;
        }
        return TextureResource(tex);
    }

    static std::optional<TextureResource> fromFile(const std::string& path) {
        Texture2D tex = LoadTexture(path.c_str());
        if (tex.id == 0) {
            return std::nullopt;
        }
        return TextureResource(tex);
    }

    // Non-copyable but moveable
    TextureResource(const TextureResource&) = delete;
    TextureResource& operator=(const TextureResource&) = delete;
    
    TextureResource(TextureResource&&) = default;
    TextureResource& operator=(TextureResource&&) = default;

    const Texture2D* get() const { return texture_.get(); }
    const Texture2D& operator*() const { return *texture_; }
    const Texture2D* operator->() const { return texture_.get(); }
    
    bool isValid() const { return texture_ && texture_->id != 0; }
    explicit operator bool() const { return isValid(); }
};

// RAII wrapper for raylib Image
class ImageResource {
private:
    std::unique_ptr<Image, void(*)(Image*)> image_;

public:
    ImageResource() : image_(nullptr, [](Image*){}) {}
    
    explicit ImageResource(Image img) 
        : image_(new Image(img), [](Image* i) {
            if (i && i->data) {
                UnloadImage(*i);
            }
            delete i;
        }) {}

    static std::optional<ImageResource> fromFile(const std::string& path) {
        Image img = LoadImage(path.c_str());
        if (!img.data) {
            return std::nullopt;
        }
        return ImageResource(img);
    }

    static std::optional<ImageResource> fromTexture(const Texture2D& texture) {
        Image img = LoadImageFromTexture(texture);
        if (!img.data) {
            return std::nullopt;
        }
        return ImageResource(img);
    }

    // Non-copyable but moveable
    ImageResource(const ImageResource&) = delete;
    ImageResource& operator=(const ImageResource&) = delete;
    
    ImageResource(ImageResource&&) = default;
    ImageResource& operator=(ImageResource&&) = default;

    const Image* get() const { return image_.get(); }
    Image* getMutable() { return image_.get(); }
    const Image& operator*() const { return *image_; }
    const Image* operator->() const { return image_.get(); }
    
    bool isValid() const { return image_ && image_->data; }
    explicit operator bool() const { return isValid(); }

    // Utility methods
    void resize(int width, int height) {
        if (isValid()) {
            ImageResize(getMutable(), width, height);
        }
    }

    bool exportToFile(const std::string& path) const {
        std::string actualPath;
        return exportToFile(path, actualPath);
    }
    
    bool exportToFile(const std::string& path, std::string& actualPath) const {
        if (!isValid()) return false;
        
        // Validate and potentially fix file extension
        actualPath = validateAndFixExtension(path);
        if (actualPath.empty()) {
            return false; // Invalid extension and couldn't fix
        }
        
        return ExportImage(*image_, actualPath.c_str());
    }

private:
    std::string validateAndFixExtension(const std::string& path) const {
        std::filesystem::path filePath(path);
        std::string extension = filePath.extension().string();
        
        // Convert to lowercase for comparison
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        // Supported extensions by Raylib's ExportImage
        std::vector<std::string> supportedExts = {".png", ".bmp", ".tga", ".jpg", ".jpeg"};
        
        // Check if extension is already valid
        if (std::find(supportedExts.begin(), supportedExts.end(), extension) != supportedExts.end()) {
            return path; // Extension is valid, return as-is
        }
        
        // If no extension or invalid extension, add .png as default
        if (extension.empty()) {
            return path + ".png";
        }
        
        // Invalid extension - replace with .png
        std::string newPath = filePath.replace_extension(".png").string();
        return newPath;
    }
};

// RAII wrapper for raylib Window
class WindowResource {
private:
    bool initialized_;

public:
    WindowResource(int width, int height, const std::string& title) 
        : initialized_(false) {
        InitWindow(width, height, title.c_str());
        if (IsWindowReady()) {
            initialized_ = true;
            SetTargetFPS(60);
        }
    }

    ~WindowResource() {
        if (initialized_) {
            CloseWindow();
        }
    }

    // Non-copyable and non-moveable
    WindowResource(const WindowResource&) = delete;
    WindowResource& operator=(const WindowResource&) = delete;
    WindowResource(WindowResource&&) = delete;
    WindowResource& operator=(WindowResource&&) = delete;

    bool isInitialized() const { return initialized_; }
    explicit operator bool() const { return initialized_; }

    // Window operations
    bool shouldClose() const { return WindowShouldClose(); }
    int getWidth() const { return GetScreenWidth(); }
    int getHeight() const { return GetScreenHeight(); }
};

// RAII wrapper for raylib RenderTexture2D
class RenderTextureResource {
private:
    std::unique_ptr<RenderTexture2D, void(*)(RenderTexture2D*)> renderTexture_;

public:
    RenderTextureResource() : renderTexture_(nullptr, [](RenderTexture2D*){}) {}
    
    explicit RenderTextureResource(int width, int height) 
        : renderTexture_(new RenderTexture2D(LoadRenderTexture(width, height)), [](RenderTexture2D* rt) {
            if (rt && rt->id > 0) {
                UnloadRenderTexture(*rt);
            }
            delete rt;
        }) {}

    // Non-copyable but moveable
    RenderTextureResource(const RenderTextureResource&) = delete;
    RenderTextureResource& operator=(const RenderTextureResource&) = delete;
    
    RenderTextureResource(RenderTextureResource&&) = default;
    RenderTextureResource& operator=(RenderTextureResource&&) = default;

    const RenderTexture2D* get() const { return renderTexture_.get(); }
    RenderTexture2D* getMutable() { return renderTexture_.get(); }
    const RenderTexture2D& operator*() const { return *renderTexture_; }
    const RenderTexture2D* operator->() const { return renderTexture_.get(); }
    
    bool isValid() const { return renderTexture_ && renderTexture_->id > 0; }
    explicit operator bool() const { return isValid(); }

    void beginDrawing() const {
        if (isValid()) {
            BeginTextureMode(*renderTexture_);
        }
    }

    void endDrawing() const {
        if (isValid()) {
            EndTextureMode();
        }
    }
    
    void clear(Color color = BLANK) const {
        if (isValid()) {
            beginDrawing();
            ClearBackground(color);
            endDrawing();
        }
    }
};

} // namespace EpiGimp

#endif // RAYLIB_WRAPPERS_HPP