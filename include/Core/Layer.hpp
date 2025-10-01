#ifndef LAYER_HPP
#define LAYER_HPP

#include <string>
#include <memory>
#include "raylib.h"
#include "../Core/RaylibWrappers.hpp"

namespace EpiGimp {

/**
 * @brief Blend modes for layer composition
 */
enum class BlendMode {
    Normal,
    Multiply,
    Screen,
    Overlay,
    SoftLight,
    HardLight
};

/**
 * @brief Represents a drawing layer with texture, visibility, and blend properties
 */
class Layer {
private:
    std::string name_;
    std::optional<RenderTextureResource> texture_;
    bool visible_;
    float opacity_;
    BlendMode blendMode_;
    int width_;
    int height_;

public:
    explicit Layer(const std::string& name, int width, int height);
    ~Layer() = default;

    Layer(const Layer&) = delete;
    Layer& operator=(const Layer&) = delete;
    Layer(Layer&&) = default;
    Layer& operator=(Layer&&) = default;

    const std::string& getName() const { return name_; }
    bool isVisible() const { return visible_; }
    float getOpacity() const { return opacity_; }
    BlendMode getBlendMode() const { return blendMode_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    bool hasTexture() const { return texture_.has_value(); }
    const RenderTextureResource& getTexture() const;

    void setName(const std::string& name) { name_ = name; }
    void setVisible(bool visible) { visible_ = visible; }
    void setOpacity(float opacity);
    void setBlendMode(BlendMode mode) { blendMode_ = mode; }

    void beginDrawing();
    void endDrawing();
    void clear(Color color = BLANK);
    Image copyImage() const;
    bool restoreImage(const Image& image);
    
    void resize(int width, int height);
    void initializeTexture();
};

} // namespace EpiGimp

#endif // LAYER_HPP