//Modern RAII wrappers for raylib resources
#ifndef RAYLIB_WRAPPERS_HPP
#define RAYLIB_WRAPPERS_HPP

#include <memory>
#include <optional>
#include <string>
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
        if (!isValid()) return false;
        return ExportImage(*image_, path.c_str());
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

} // namespace EpiGimp

#endif // RAYLIB_WRAPPERS_HPP