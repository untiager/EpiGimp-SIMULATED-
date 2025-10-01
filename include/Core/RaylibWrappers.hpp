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

class TextureResource {
private:
    std::unique_ptr<Texture2D, void(*)(Texture2D*)> texture_;

public:
    TextureResource() : texture_(nullptr, [](Texture2D*){}) {}
    
    explicit TextureResource(Texture2D tex) 
        : texture_(new Texture2D(tex), [](Texture2D* t) {
            if (t && t->id != 0)
                UnloadTexture(*t);
            delete t;
        }) {}

    static std::optional<TextureResource> fromImage(const Image& image) {
        Texture2D tex = LoadTextureFromImage(image);
        if (tex.id == 0)
            return std::nullopt;
        return TextureResource(tex);
    }

    static std::optional<TextureResource> fromFile(const std::string& path) {
        Texture2D tex = LoadTexture(path.c_str());
        if (tex.id == 0)
            return std::nullopt;
        return TextureResource(tex);
    }

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

class ImageResource {
private:
    std::unique_ptr<Image, void(*)(Image*)> image_;

public:
    ImageResource() : image_(nullptr, [](Image*){}) {}
    
    explicit ImageResource(Image img) 
        : image_(new Image(img), [](Image* i) {
            if (i && i->data)
                UnloadImage(*i);
            delete i;
        }) {}

    static std::optional<ImageResource> fromFile(const std::string& path) {
        Image img = LoadImage(path.c_str());
        if (!img.data)
            return std::nullopt;
        return ImageResource(img);
    }

    static std::optional<ImageResource> fromTexture(const Texture2D& texture) {
        Image img = LoadImageFromTexture(texture);
        if (!img.data)
            return std::nullopt;
        return ImageResource(img);
    }

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

    void resize(int width, int height);
    bool exportToFile(const std::string& path) const;
    bool exportToFile(const std::string& path, std::string& actualPath) const;

private:
    std::string validateAndFixExtension(const std::string& path) const;
};

class WindowResource {
private:
    bool initialized_;

public:
    WindowResource(int width, int height, const std::string& title);
    ~WindowResource();

    WindowResource(const WindowResource&) = delete;
    WindowResource& operator=(const WindowResource&) = delete;
    WindowResource(WindowResource&&) = delete;
    WindowResource& operator=(WindowResource&&) = delete;

    bool isInitialized() const { return initialized_; }
    explicit operator bool() const { return initialized_; }

    bool shouldClose() const;
    int getWidth() const;
    int getHeight() const;
};

class RenderTextureResource {
private:
    std::unique_ptr<RenderTexture2D, void(*)(RenderTexture2D*)> renderTexture_;

public:
    RenderTextureResource() : renderTexture_(nullptr, [](RenderTexture2D*){}) {}
    explicit RenderTextureResource(int width, int height);

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

    void beginDrawing() const;
    void endDrawing() const;
    void clear(Color color = BLANK) const;
};

} // namespace EpiGimp

#endif // RAYLIB_WRAPPERS_HPP