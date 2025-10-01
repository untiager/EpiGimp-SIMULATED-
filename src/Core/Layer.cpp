#include "../../include/Core/Layer.hpp"
#include <stdexcept>
#include <algorithm>

namespace EpiGimp {

Layer::Layer(const std::string& name, int width, int height)
    : name_(name)
    , visible_(true)
    , opacity_(1.0f)
    , blendMode_(BlendMode::Normal)
    , width_(width)
    , height_(height)
{
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("Layer dimensions must be positive");
    initializeTexture();
}

const RenderTextureResource& Layer::getTexture() const
{
    if (!texture_.has_value())
        throw std::runtime_error("Layer texture not initialized");
    return texture_.value();
}

void Layer::setOpacity(float opacity)
{
    opacity_ = std::clamp(opacity, 0.0f, 1.0f);
}

void Layer::beginDrawing()
{
    if (!texture_.has_value())
        initializeTexture();
    BeginTextureMode(**texture_);
}

void Layer::endDrawing()
{
    if (texture_.has_value())
        EndTextureMode();
}

void Layer::clear(Color color)
{
    if (!texture_.has_value())
        initializeTexture();
    
    beginDrawing();
    ClearBackground(color);
    endDrawing();
}

Image Layer::copyImage() const
{
    if (!texture_.has_value())
        return GenImageColor(width_, height_, BLANK);
    
    return LoadImageFromTexture((**texture_).texture);
}

bool Layer::restoreImage(const Image& image)
{
    if (!texture_.has_value())
        initializeTexture();
    
    try {
        beginDrawing();
        ClearBackground(BLANK);
        
        // Create temporary texture from image
        Texture2D tempTexture = LoadTextureFromImage(image);
        DrawTexture(tempTexture, 0, 0, WHITE);
        UnloadTexture(tempTexture);
        
        endDrawing();
        return true;
    } catch (...) {
        return false;
    }
}

void Layer::resize(int width, int height)
{
    if (width <= 0 || height <= 0)
        return;
    
    std::optional<Image> currentImage;
    if (texture_.has_value())
        currentImage = copyImage();
    
    width_ = width;
    height_ = height;
    texture_.reset();
    initializeTexture();
    
    if (currentImage.has_value()) {
        // Scale the image to new size
        ImageResize(&currentImage.value(), width, height);
        restoreImage(currentImage.value());
        UnloadImage(currentImage.value());
    }
}

void Layer::initializeTexture()
{
    texture_ = RenderTextureResource(width_, height_);
    if (texture_.has_value())
        clear(BLANK);
}

} // namespace EpiGimp