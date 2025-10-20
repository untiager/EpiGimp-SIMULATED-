#include "../../include/UI/Canvas.hpp"
#include <iostream>
#include <algorithm>

namespace EpiGimp {

void Canvas::extractSelectionContent() {
    if (!hasSelection_ || selectedLayerIndex_ < 0 || selectedLayerIndex_ >= static_cast<int>(drawingLayers_.size())) {
        std::cout << "Cannot extract content: no selection or invalid layer" << std::endl;
        return;
    }

    const auto& layer = drawingLayers_[selectedLayerIndex_];
    if (!layer.texture.has_value() || !layer.texture->isValid()) {
        std::cout << "Cannot extract content: layer texture is invalid" << std::endl;
        return;
    }

    // Store original content rectangle
    contentOriginalRect_ = selectionRect_;
    contentTransformRect_ = selectionRect_;
    
    std::cout << "extractSelectionContent: originalRect=(" << contentOriginalRect_.x << "," << contentOriginalRect_.y 
              << "," << contentOriginalRect_.width << "," << contentOriginalRect_.height << ")" << std::endl;
    std::cout << "extractSelectionContent: transformRect=(" << contentTransformRect_.x << "," << contentTransformRect_.y 
              << "," << contentTransformRect_.width << "," << contentTransformRect_.height << ")" << std::endl;

    // Get the layer texture
    Texture2D layerTexture = layer.texture->get()->texture;
    
    // Create a RenderTextureResource to extract the selected content
    int width = static_cast<int>(selectionRect_.width);
    int height = static_cast<int>(selectionRect_.height);
    
    if (width <= 0 || height <= 0) {
        std::cout << "Invalid selection size for content extraction" << std::endl;
        return;
    }
    
    RenderTextureResource extractedTexture(width, height);
    if (!extractedTexture.isValid()) {
        std::cout << "Failed to create texture for content extraction" << std::endl;
        return;
    }

    extractedTexture.beginDrawing();
    extractedTexture.clear(BLANK);
    
    // Draw the selected portion of the layer
    Rectangle sourceRect = {
        selectionRect_.x,
        selectionRect_.y,
        selectionRect_.width,
        selectionRect_.height
    };
    
    Rectangle destRect = {
        0, 0,
        selectionRect_.width,
        selectionRect_.height
    };
    
    DrawTexturePro(layerTexture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    extractedTexture.endDrawing();
    
    // Store the extracted content
    selectionContent_ = std::move(extractedTexture);
    
    std::cout << "Extracted content from selection: " << selectionRect_.width << "x" << selectionRect_.height << std::endl;
}

void Canvas::updateContentTransform(Vector2 mousePos) {
    if (!selectionContent_.has_value() || resizeHandle_ == ResizeHandle::None) {
        return;
    }

    // Convert mouse delta from screen space to image space
    Vector2 mouseDelta = {
        mousePos.x - lastMousePos_.x,
        mousePos.y - lastMousePos_.y
    };
    
    // Scale the delta by the inverse of the zoom level to get image space delta
    Vector2 imageDelta = {
        mouseDelta.x / zoomLevel_,
        mouseDelta.y / zoomLevel_
    };

    float minSize = 10.0f; // Minimum size for transformed content

    switch (resizeHandle_) {
        case ResizeHandle::TopLeft:
            contentTransformRect_.x += imageDelta.x;
            contentTransformRect_.y += imageDelta.y;
            contentTransformRect_.width -= imageDelta.x;
            contentTransformRect_.height -= imageDelta.y;
            break;
            
        case ResizeHandle::TopRight:
            contentTransformRect_.y += imageDelta.y;
            contentTransformRect_.width += imageDelta.x;
            contentTransformRect_.height -= imageDelta.y;
            break;
            
        case ResizeHandle::BottomLeft:
            contentTransformRect_.x += imageDelta.x;
            contentTransformRect_.width -= imageDelta.x;
            contentTransformRect_.height += imageDelta.y;
            break;
            
        case ResizeHandle::BottomRight:
            contentTransformRect_.width += imageDelta.x;
            contentTransformRect_.height += imageDelta.y;
            break;
            
        case ResizeHandle::Top:
            contentTransformRect_.y += imageDelta.y;
            contentTransformRect_.height -= imageDelta.y;
            break;
            
        case ResizeHandle::Bottom:
            contentTransformRect_.height += imageDelta.y;
            break;
            
        case ResizeHandle::Left:
            contentTransformRect_.x += imageDelta.x;
            contentTransformRect_.width -= imageDelta.x;
            break;
            
        case ResizeHandle::Right:
            contentTransformRect_.width += imageDelta.x;
            break;
            
        default:
            break;
    }

    // Ensure minimum size
    if (contentTransformRect_.width < minSize) {
        contentTransformRect_.width = minSize;
    }
    if (contentTransformRect_.height < minSize) {
        contentTransformRect_.height = minSize;
    }

    lastMousePos_ = mousePos;
    
    std::cout << "Transforming content to: (" << contentTransformRect_.x << "," << contentTransformRect_.y 
              << ") " << contentTransformRect_.width << "x" << contentTransformRect_.height << std::endl;
}

void Canvas::applyTransformedContent() {
    if (!selectionContent_.has_value() || selectedLayerIndex_ < 0 || selectedLayerIndex_ >= static_cast<int>(drawingLayers_.size())) {
        std::cout << "Cannot apply transformed content: no content or invalid layer" << std::endl;
        return;
    }

    auto& layer = drawingLayers_[selectedLayerIndex_];
    if (!layer.texture.has_value() || !layer.texture->isValid()) {
        std::cout << "Cannot apply transformed content: layer texture is invalid" << std::endl;
        return;
    }

    const auto& contentTexture = selectionContent_.value();
    if (!contentTexture.isValid()) {
        std::cout << "Cannot apply transformed content: content texture is invalid" << std::endl;
        return;
    }

    // Get the layer texture
    Texture2D layerTexture = layer.texture->get()->texture;
    
    // Create a new render texture for the updated layer
    RenderTexture2D newLayerTexture = LoadRenderTexture(layerTexture.width, layerTexture.height);
    if (!newLayerTexture.id) {
        std::cout << "Failed to create render texture for applying transformed content" << std::endl;
        return;
    }

    BeginTextureMode(newLayerTexture);
    
    // First, draw the original layer
    DrawTexture(layerTexture, 0, 0, WHITE);
    
    // Clear the original selection area
    Rectangle clearRect = contentOriginalRect_;
    DrawRectangleRec(clearRect, BLANK);
    
    // Draw the transformed content
    Rectangle sourceRect = {0, 0, 
                           static_cast<float>(contentTexture.get()->texture.width), 
                           static_cast<float>(contentTexture.get()->texture.height)};
    DrawTexturePro(contentTexture.get()->texture, sourceRect, contentTransformRect_, {0, 0}, 0.0f, WHITE);
    
    EndTextureMode();
    
    // Update the layer with the new texture - create new RenderTextureResource from this texture
    layer.texture = RenderTextureResource(newLayerTexture.texture.width, newLayerTexture.texture.height);
    if (layer.texture->isValid()) {
        layer.texture->beginDrawing();
        layer.texture->clear(BLANK);
        DrawTexture(newLayerTexture.texture, 0, 0, WHITE);
        layer.texture->endDrawing();
    }
    
    // Clean up the temporary render texture
    UnloadRenderTexture(newLayerTexture);
    
    std::cout << "Applied transformed content to layer" << std::endl;
}

void Canvas::drawTransformPreview(Rectangle imageDestRect) const {
    if (!selectionContent_.has_value() || !isTransformMode_) {
        return;
    }
    
    const auto& contentTexture = selectionContent_.value();
    if (!contentTexture.isValid()) {
        return;
    }
    
    // Calculate the transform area in screen coordinates relative to the image
    Rectangle transformScreenRect;
    
    // Convert image coordinates to screen coordinates for the transform rectangle
    const float scaleX = imageDestRect.width / (currentTexture_ ? (*currentTexture_)->width : 800.0f);
    const float scaleY = imageDestRect.height / (currentTexture_ ? (*currentTexture_)->height : 600.0f);
    
    transformScreenRect.x = imageDestRect.x + (contentTransformRect_.x * scaleX);
    transformScreenRect.y = imageDestRect.y + (contentTransformRect_.y * scaleY);
    transformScreenRect.width = contentTransformRect_.width * scaleX;
    transformScreenRect.height = contentTransformRect_.height * scaleY;
    
    // Draw the scaled content texture
    Rectangle sourceRect = {0, 0, 
                           static_cast<float>(contentTexture.get()->texture.width), 
                           static_cast<float>(-contentTexture.get()->texture.height)}; // Negative height for Y-flip
    
    DrawTexturePro(contentTexture.get()->texture, sourceRect, transformScreenRect, {0, 0}, 0.0f, WHITE);
}

void Canvas::enterTransformMode() {
    if (!hasSelection_) {
        std::cout << "Cannot enter transform mode: no selection" << std::endl;
        return;
    }

    isTransformMode_ = true;
    extractSelectionContent();
    std::cout << "Entered transform mode" << std::endl;
}

void Canvas::exitTransformMode() {
    if (isTransformMode_) {
        if (isTransformingContent_) {
            // Apply any pending transformation
            applyTransformedContent();
            isTransformingContent_ = false;
            resizeHandle_ = ResizeHandle::None;
        }
        
        // Clean up content texture
        if (selectionContent_.has_value()) {
            selectionContent_.reset();
        }
        
        isTransformMode_ = false;
        std::cout << "Exited transform mode" << std::endl;
    }
}

} // namespace EpiGimp