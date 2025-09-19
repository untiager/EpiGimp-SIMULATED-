//Canvas implementation
#include "../../include/UI/Canvas.hpp"
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace EpiGimp {

Canvas::Canvas(Rectangle bounds, EventDispatcher* dispatcher)
    : bounds_(bounds), zoomLevel_(1.0f), panOffset_{0, 0}, eventDispatcher_(dispatcher),
      currentTool_(DrawingTool::None), isDrawing_(false), lastMousePos_{0, 0}
{
    
    if (!dispatcher)
        throw std::invalid_argument("EventDispatcher cannot be null");
    
    std::cout << "Canvas initialized with bounds: " 
              << bounds.x << ", " << bounds.y << ", " 
              << bounds.width << ", " << bounds.height << std::endl;
}

void Canvas::update(float /*deltaTime*/)
{
    handleInput();
    handleDrawing();
}

void Canvas::draw() const
{
    DrawRectangleRec(bounds_, WHITE);
    DrawRectangleLinesEx(bounds_, 1, DARKGRAY);
    
    BeginScissorMode(static_cast<int>(bounds_.x), static_cast<int>(bounds_.y), 
                     static_cast<int>(bounds_.width), static_cast<int>(bounds_.height));
    
    if (hasImage()) {
        drawImage();
    } else {
        drawPlaceholder();
    }
    
    EndScissorMode();
}

void Canvas::loadImage(const std::string& filePath)
{
    auto texture = createTextureFromFile(filePath);
    if (!texture) {
        eventDispatcher_->emit<ErrorEvent>("Failed to load image: " + filePath);
        return;
    }
    
    currentTexture_ = std::move(texture);
    currentImagePath_ = filePath;
    resetViewTransform();
    
    // Initialize drawing layer for the new image
    initializeDrawingLayer();
    
    eventDispatcher_->emit<ImageLoadedEvent>(filePath);
    std::cout << "Image loaded successfully: " << filePath << std::endl;
}

bool Canvas::saveImage(const std::string& filePath)
{
    if (!hasImage()) {
        eventDispatcher_->emit<ErrorEvent>("No image to save");
        return false;
    }
    
    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
    
    // If we have drawings, composite them with the original image
    if (drawingLayer_ && drawingLayer_->isValid()) {
        // Get original image
        auto originalImage = ImageResource::fromTexture(**currentTexture_);
        if (!originalImage) {
            eventDispatcher_->emit<ErrorEvent>("Failed to create image from texture");
            return false;
        }
        
        // Get drawing layer as image (this handles the Y-flip automatically)
        auto drawingImage = ImageResource::fromTexture((**drawingLayer_).texture);
        if (!drawingImage) {
            eventDispatcher_->emit<ErrorEvent>("Failed to create drawing image");
            return false;
        }
        
        // Flip the drawing image vertically to correct the render texture flip
        ImageFlipVertical(drawingImage->getMutable());
        
        // Create a new image and composite them
        Image composite = GenImageColor(originalImage->get()->width, originalImage->get()->height, WHITE);
        
        // Draw original image onto composite
        ImageDraw(&composite, *originalImage->get(), 
                 Rectangle{0, 0, static_cast<float>(originalImage->get()->width), static_cast<float>(originalImage->get()->height)},
                 Rectangle{0, 0, static_cast<float>(composite.width), static_cast<float>(composite.height)}, WHITE);
        
        // Draw the drawing layer on top
        ImageDraw(&composite, *drawingImage->get(),
                 Rectangle{0, 0, static_cast<float>(drawingImage->get()->width), static_cast<float>(drawingImage->get()->height)},
                 Rectangle{0, 0, static_cast<float>(composite.width), static_cast<float>(composite.height)}, WHITE);
        
        // Create ImageResource from the composite
        ImageResource compositeRes(composite);
        
        std::string actualPath;
        const bool success = compositeRes.exportToFile(filePath, actualPath);
        
        // Inform user if filename was auto-corrected
        if (success && actualPath != filePath) {
            std::cout << "Note: File extension was auto-corrected to: " << actualPath << std::endl;
        }
        
        eventDispatcher_->emit<ImageSavedEvent>(actualPath, success);
        
        if (success) {
            std::cout << "Image saved successfully with drawings: " << actualPath << std::endl;
        } else {
            eventDispatcher_->emit<ErrorEvent>("Failed to save image: " + filePath);
        }
        
        return success;
    }
    
    // No drawings, save original image
    auto imageRes = ImageResource::fromTexture(**currentTexture_);
    if (!imageRes) {
        eventDispatcher_->emit<ErrorEvent>("Failed to create image from texture");
        return false;
    }
    
    std::string actualPath;
    const bool success = imageRes->exportToFile(filePath, actualPath);
    
    // Inform user if filename was auto-corrected
    if (success && actualPath != filePath) {
        std::cout << "Note: File extension was auto-corrected to: " << actualPath << std::endl;
    }
    
    eventDispatcher_->emit<ImageSavedEvent>(actualPath, success);
    
    if (success) {
        std::cout << "Image saved successfully: " << actualPath << std::endl;
    } else {
        eventDispatcher_->emit<ErrorEvent>("Failed to save image: " + filePath);
    }
    
    return success;
}

bool Canvas::hasImage() const
{
    return currentTexture_.has_value() && currentTexture_->isValid();
}

void Canvas::setZoom(float zoom)
{
    zoomLevel_ = std::clamp(zoom, MIN_ZOOM, MAX_ZOOM);
}

void Canvas::setPan(Vector2 offset)
{
    panOffset_ = offset;
}

void Canvas::handleInput()
{
    handleZoom();
    handlePanning();
}

void Canvas::handleZoom()
{
    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        const Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, bounds_)) {
            const float newZoom = zoomLevel_ + wheel * ZOOM_STEP;
            setZoom(newZoom);
        }
    }
}

void Canvas::handlePanning()
{
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        const Vector2 mouseDelta = GetMouseDelta();
        panOffset_.x += mouseDelta.x;
        panOffset_.y += mouseDelta.y;
    }
    
    if (IsKeyDown(KEY_LEFT)) panOffset_.x += PAN_SPEED;
    if (IsKeyDown(KEY_RIGHT)) panOffset_.x -= PAN_SPEED;
    if (IsKeyDown(KEY_UP)) panOffset_.y += PAN_SPEED;
    if (IsKeyDown(KEY_DOWN)) panOffset_.y -= PAN_SPEED;
}

void Canvas::handleDrawing()
{
    if (currentTool_ == DrawingTool::None || !hasImage()) {
        isDrawing_ = false;
        return;
    }
    
    const Vector2 mousePos = GetMousePosition();
    const bool mouseInBounds = CheckCollisionPointRec(mousePos, bounds_);
    
    if (currentTool_ == DrawingTool::Crayon && mouseInBounds) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isDrawing_ = true;
            lastMousePos_ = mousePos;
            std::cout << "Started drawing at: " << mousePos.x << ", " << mousePos.y << std::endl;
        }
        
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && isDrawing_) {
            // Draw line from last position to current position
            drawStroke(lastMousePos_, mousePos);
            lastMousePos_ = mousePos;
        }
        
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            isDrawing_ = false;
            std::cout << "Finished drawing stroke" << std::endl;
        }
    }
}

void Canvas::drawImage() const
{
    if (!hasImage()) return;
    
    const auto destRect = calculateImageDestRect();
    const Rectangle sourceRect = {0, 0, static_cast<float>((*currentTexture_)->width), 
                                        static_cast<float>((*currentTexture_)->height)};
    
    DrawTexturePro(**currentTexture_, sourceRect, destRect, Vector2{0, 0}, 0.0f, WHITE);
    
    // Draw the drawing layer on top of the image
    if (drawingLayer_ && drawingLayer_->isValid()) {
        const Rectangle drawingSourceRect = {0, 0, static_cast<float>((**drawingLayer_).texture.width), 
                                           -static_cast<float>((**drawingLayer_).texture.height)}; // Flip Y for render texture
        DrawTexturePro((**drawingLayer_).texture, drawingSourceRect, destRect, Vector2{0, 0}, 0.0f, WHITE);
    }
}

void Canvas::drawPlaceholder() const
{
    constexpr const char* text = "No image loaded. Click 'Load Image' to get started.";
    constexpr int fontSize = 20;
    
    const int textWidth = MeasureText(text, fontSize);
    const int textX = static_cast<int>(bounds_.x + (bounds_.width - textWidth) / 2);
    const int textY = static_cast<int>(bounds_.y + bounds_.height / 2 - fontSize / 2);
    
    DrawText(text, textX, textY, fontSize, LIGHTGRAY);
}

Rectangle Canvas::calculateImageDestRect() const
{
    if (!hasImage()) return Rectangle{0, 0, 0, 0};
    
    const float imageWidth = (*currentTexture_)->width * zoomLevel_;
    const float imageHeight = (*currentTexture_)->height * zoomLevel_;
    
    const float imageX = bounds_.x + (bounds_.width - imageWidth) / 2 + panOffset_.x;
    const float imageY = bounds_.y + (bounds_.height - imageHeight) / 2 + panOffset_.y;
    
    return Rectangle{imageX, imageY, imageWidth, imageHeight};
}

Vector2 Canvas::getImageCenter() const
{
    return Vector2{bounds_.x + bounds_.width / 2, bounds_.y + bounds_.height / 2};
}

std::optional<TextureResource> Canvas::createTextureFromFile(const std::string& filePath)
{
    auto imageRes = ImageResource::fromFile(filePath);
    if (!imageRes) {
        return std::nullopt;
    }
    
    // Resize image if too large for canvas
    const auto maxWidth = static_cast<int>(bounds_.width);
    const auto maxHeight = static_cast<int>(bounds_.height);
    
    if (imageRes->get()->width > maxWidth || imageRes->get()->height > maxHeight) {
        const float widthRatio = static_cast<float>(maxWidth) / imageRes->get()->width;
        const float heightRatio = static_cast<float>(maxHeight) / imageRes->get()->height;
        const float scale = std::min(widthRatio, heightRatio);
        
        const int newWidth = static_cast<int>(imageRes->get()->width * scale);
        const int newHeight = static_cast<int>(imageRes->get()->height * scale);
        
        imageRes->resize(newWidth, newHeight);
        std::cout << "Image resized to: " << newWidth << "x" << newHeight << std::endl;
    }
    
    return TextureResource::fromImage(*imageRes->get());
}

void Canvas::resetViewTransform()
{
    zoomLevel_ = 1.0f;
    panOffset_ = Vector2{0, 0};
}

void Canvas::setDrawingTool(DrawingTool tool)
{
    currentTool_ = tool;
    std::cout << "Canvas drawing tool set to: " << static_cast<int>(tool) << std::endl;
}

void Canvas::initializeDrawingLayer()
{
    if (hasImage() && currentTexture_) {
        const int width = (*currentTexture_)->width;
        const int height = (*currentTexture_)->height;
        drawingLayer_ = RenderTextureResource(width, height);
        
        // Clear the drawing layer to transparent
        drawingLayer_->clear(BLANK);
        
        std::cout << "Drawing layer initialized: " << width << "x" << height << std::endl;
    }
}

void Canvas::drawStroke(Vector2 from, Vector2 to)
{
    if (!drawingLayer_ || !drawingLayer_->isValid()) {
        return;
    }
    
    // Convert screen coordinates to image coordinates
    const Rectangle imageRect = calculateImageDestRect();
    
    // Transform screen coordinates to image texture coordinates
    const Vector2 imageFrom = {
        (from.x - imageRect.x) / imageRect.width * (*currentTexture_)->width,
        (from.y - imageRect.y) / imageRect.height * (*currentTexture_)->height
    };
    
    const Vector2 imageTo = {
        (to.x - imageRect.x) / imageRect.width * (*currentTexture_)->width,
        (to.y - imageRect.y) / imageRect.height * (*currentTexture_)->height
    };
    
    // Draw to the render texture
    drawingLayer_->beginDrawing();
    DrawLineEx(imageFrom, imageTo, 3.0f, RED); // TODO: Make brush size and color configurable
    drawingLayer_->endDrawing();
}

} // namespace EpiGimp