//Canvas selection functionality
#include "../../include/UI/Canvas.hpp"
#include <iostream>
#include <cmath>
#include "raymath.h"

namespace EpiGimp {

void Canvas::handleSelection()
{
    if (currentTool_ != DrawingTool::Select) return;
    
    const Vector2 mousePos = GetMousePosition();
    
    // Check if mouse is within canvas bounds
    if (!CheckCollisionPointRec(mousePos, bounds_)) return;
    
    // Handle selection input
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // Check if clicking on a resize handle first
        if (hasSelection_) {
            ResizeHandle handle = getResizeHandleAt(mousePos);
            if (handle != ResizeHandle::None) {
                if (isTransformMode_) {
                    // Start transforming content
                    isTransformingContent_ = true;
                    resizeHandle_ = handle;
                    resizeStartPos_ = mousePos;
                    resizeStartRect_ = contentTransformRect_;
                    std::cout << "Started transforming content with handle " << static_cast<int>(handle) << std::endl;
                } else {
                    // Start resizing selection
                    isResizingSelection_ = true;
                    resizeHandle_ = handle;
                    resizeStartPos_ = mousePos;
                    resizeStartRect_ = selectionRect_;
                    std::cout << "Started resizing selection with handle " << static_cast<int>(handle) << std::endl;
                }
                return;
            }
        }
        
        // Start new selection
        selectionStart_ = mousePos;
        selectionEnd_ = mousePos;
        isSelecting_ = true;
        hasSelection_ = false; // Clear existing selection while dragging
        
        std::cout << "Started selection at (" << mousePos.x << "," << mousePos.y << ")" << std::endl;
    }
    
    if (isTransformingContent_ && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        // Update content transform
        updateContentTransform(mousePos);
    }
    else if (isResizingSelection_ && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        // Update selection resize
        updateSelectionResize(mousePos);
    }
    else if (isSelecting_ && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        // Update selection end point
        selectionEnd_ = mousePos;
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (isTransformingContent_) {
            // Finish transforming content
            isTransformingContent_ = false;
            resizeHandle_ = ResizeHandle::None;
            std::cout << "Finished transforming content: (" << contentTransformRect_.x << "," << contentTransformRect_.y 
                      << ") " << contentTransformRect_.width << "x" << contentTransformRect_.height << std::endl;
        }
        else if (isResizingSelection_) {
            // Finish resizing
            isResizingSelection_ = false;
            resizeHandle_ = ResizeHandle::None;
            std::cout << "Finished resizing selection: (" << selectionRect_.x << "," << selectionRect_.y 
                      << ") " << selectionRect_.width << "x" << selectionRect_.height << std::endl;
        }
        else if (isSelecting_) {
            // Finish selection
            isSelecting_ = false;
            
            // Convert screen coordinates to image coordinates
            const Vector2 imageStart = screenToImageCoords(selectionStart_);
            const Vector2 imageEnd = screenToImageCoords(selectionEnd_);
            
            // Create normalized rectangle
            selectionRect_ = normalizeRect(imageStart, imageEnd);
            
            // Only create selection if it has meaningful size
            if (selectionRect_.width > 1.0f && selectionRect_.height > 1.0f) {
                hasSelection_ = true;
                std::cout << "Selection created: (" << selectionRect_.x << "," << selectionRect_.y 
                          << ") " << selectionRect_.width << "x" << selectionRect_.height << std::endl;
            } else {
                hasSelection_ = false;
                std::cout << "Selection too small, ignored" << std::endl;
            }
        }
    }
}

void Canvas::drawSelection() const
{
    if (!hasImage()) return;
    
    Rectangle screenRect;
    
    if (isSelecting_) {
        // Draw temporary selection rectangle while dragging
        screenRect = normalizeRect(selectionStart_, selectionEnd_);
    } else if (hasSelection_) {
        // Draw finalized selection rectangle
        const Vector2 topLeft = imageToScreenCoords(Vector2{selectionRect_.x, selectionRect_.y});
        const Vector2 bottomRight = imageToScreenCoords(Vector2{selectionRect_.x + selectionRect_.width, 
                                                                selectionRect_.y + selectionRect_.height});
        screenRect = normalizeRect(topLeft, bottomRight);
    } else {
        return;
    }
    
    // Draw selection rectangle with marching ants effect
    const float dashLength = 4.0f;
    const float animOffset = selectionAnimTime_ * 8.0f; // Speed of animation
    
    // Create marching ants pattern with different colors for transform mode
    Color lineColor = isSelecting_ ? BLUE : (isTransformMode_ ? RED : BLACK);
    Color bgColor = isSelecting_ ? LIGHTGRAY : (isTransformMode_ ? YELLOW : WHITE);
    
    // Draw background outline
    DrawRectangleLinesEx(screenRect, 1.0f, bgColor);
    
    // Draw marching ants by drawing dashed lines
    drawMarchingAnts(screenRect, lineColor, dashLength, animOffset);
    
    // Draw resize handles for existing selections
    if (!isSelecting_) {
        drawResizeHandles();
        
        // If in transform mode, also draw the content transform rectangle
        if (isTransformMode_ && selectionContent_.has_value()) {
            const Vector2 topLeft = imageToScreenCoords(Vector2{contentTransformRect_.x, contentTransformRect_.y});
            const Vector2 bottomRight = imageToScreenCoords(Vector2{contentTransformRect_.x + contentTransformRect_.width, 
                                                                    contentTransformRect_.y + contentTransformRect_.height});
            Rectangle transformScreenRect = normalizeRect(topLeft, bottomRight);
            
            // Draw content transform outline in bright green
            DrawRectangleLinesEx(transformScreenRect, 2.0f, GREEN);
        }
    }
}

void Canvas::drawMarchingAnts(Rectangle rect, Color color, float dashLength, float offset) const
{
    // Top edge
    drawDashedLine(Vector2{rect.x, rect.y}, Vector2{rect.x + rect.width, rect.y}, 
                   color, dashLength, offset);
    
    // Right edge
    drawDashedLine(Vector2{rect.x + rect.width, rect.y}, Vector2{rect.x + rect.width, rect.y + rect.height}, 
                   color, dashLength, offset);
    
    // Bottom edge
    drawDashedLine(Vector2{rect.x + rect.width, rect.y + rect.height}, Vector2{rect.x, rect.y + rect.height}, 
                   color, dashLength, offset);
    
    // Left edge
    drawDashedLine(Vector2{rect.x, rect.y + rect.height}, Vector2{rect.x, rect.y}, 
                   color, dashLength, offset);
}

void Canvas::drawDashedLine(Vector2 start, Vector2 end, Color color, float dashLength, float offset) const
{
    const Vector2 dir = Vector2Subtract(end, start);
    const float totalLength = Vector2Length(dir);
    
    if (totalLength < 0.1f) return;
    
    const Vector2 normalizedDir = Vector2Scale(dir, 1.0f / totalLength);
    
    float currentPos = fmod(offset, dashLength * 2.0f);
    
    while (currentPos < totalLength) {
        const float segmentStart = currentPos;
        const float segmentEnd = std::min(currentPos + dashLength, totalLength);
        
        if (static_cast<int>(currentPos / dashLength) % 2 == 0) { // Only draw on even segments
            const Vector2 lineStart = Vector2Add(start, Vector2Scale(normalizedDir, segmentStart));
            const Vector2 lineEnd = Vector2Add(start, Vector2Scale(normalizedDir, segmentEnd));
            DrawLineV(lineStart, lineEnd, color);
        }
        
        currentPos += dashLength;
    }
}

ResizeHandle Canvas::getResizeHandleAt(Vector2 mousePos) const
{
    if (!hasSelection_) return ResizeHandle::None;
    
    const float handleSize = 8.0f; // Size of resize handles in pixels
    const Rectangle selectionScreenRect = {
        imageToScreenCoords({selectionRect_.x, selectionRect_.y}).x,
        imageToScreenCoords({selectionRect_.x, selectionRect_.y}).y,
        selectionRect_.width * zoomLevel_,
        selectionRect_.height * zoomLevel_
    };
    
    // Define handle positions
    const Vector2 topLeft = {selectionScreenRect.x, selectionScreenRect.y};
    const Vector2 topRight = {selectionScreenRect.x + selectionScreenRect.width, selectionScreenRect.y};
    const Vector2 bottomLeft = {selectionScreenRect.x, selectionScreenRect.y + selectionScreenRect.height};
    const Vector2 bottomRight = {selectionScreenRect.x + selectionScreenRect.width, selectionScreenRect.y + selectionScreenRect.height};
    const Vector2 topCenter = {selectionScreenRect.x + selectionScreenRect.width * 0.5f, selectionScreenRect.y};
    const Vector2 rightCenter = {selectionScreenRect.x + selectionScreenRect.width, selectionScreenRect.y + selectionScreenRect.height * 0.5f};
    const Vector2 bottomCenter = {selectionScreenRect.x + selectionScreenRect.width * 0.5f, selectionScreenRect.y + selectionScreenRect.height};
    const Vector2 leftCenter = {selectionScreenRect.x, selectionScreenRect.y + selectionScreenRect.height * 0.5f};
    
    // Check each handle (corners first for priority)
    if (CheckCollisionPointRec(mousePos, {topLeft.x - handleSize*0.5f, topLeft.y - handleSize*0.5f, handleSize, handleSize}))
        return ResizeHandle::TopLeft;
    if (CheckCollisionPointRec(mousePos, {topRight.x - handleSize*0.5f, topRight.y - handleSize*0.5f, handleSize, handleSize}))
        return ResizeHandle::TopRight;
    if (CheckCollisionPointRec(mousePos, {bottomLeft.x - handleSize*0.5f, bottomLeft.y - handleSize*0.5f, handleSize, handleSize}))
        return ResizeHandle::BottomLeft;
    if (CheckCollisionPointRec(mousePos, {bottomRight.x - handleSize*0.5f, bottomRight.y - handleSize*0.5f, handleSize, handleSize}))
        return ResizeHandle::BottomRight;
    
    // Check edge handles
    if (CheckCollisionPointRec(mousePos, {topCenter.x - handleSize*0.5f, topCenter.y - handleSize*0.5f, handleSize, handleSize}))
        return ResizeHandle::Top;
    if (CheckCollisionPointRec(mousePos, {rightCenter.x - handleSize*0.5f, rightCenter.y - handleSize*0.5f, handleSize, handleSize}))
        return ResizeHandle::Right;
    if (CheckCollisionPointRec(mousePos, {bottomCenter.x - handleSize*0.5f, bottomCenter.y - handleSize*0.5f, handleSize, handleSize}))
        return ResizeHandle::Bottom;
    if (CheckCollisionPointRec(mousePos, {leftCenter.x - handleSize*0.5f, leftCenter.y - handleSize*0.5f, handleSize, handleSize}))
        return ResizeHandle::Left;
    
    return ResizeHandle::None;
}

Rectangle Canvas::getResizeHandleRect(ResizeHandle handle) const
{
    if (!hasSelection_) return {0, 0, 0, 0};
    
    const float handleSize = 8.0f;
    const Rectangle selectionScreenRect = {
        imageToScreenCoords({selectionRect_.x, selectionRect_.y}).x,
        imageToScreenCoords({selectionRect_.x, selectionRect_.y}).y,
        selectionRect_.width * zoomLevel_,
        selectionRect_.height * zoomLevel_
    };
    
    Vector2 handlePos = {0, 0};
    
    switch (handle) {
        case ResizeHandle::TopLeft:
            handlePos = {selectionScreenRect.x, selectionScreenRect.y};
            break;
        case ResizeHandle::TopRight:
            handlePos = {selectionScreenRect.x + selectionScreenRect.width, selectionScreenRect.y};
            break;
        case ResizeHandle::BottomLeft:
            handlePos = {selectionScreenRect.x, selectionScreenRect.y + selectionScreenRect.height};
            break;
        case ResizeHandle::BottomRight:
            handlePos = {selectionScreenRect.x + selectionScreenRect.width, selectionScreenRect.y + selectionScreenRect.height};
            break;
        case ResizeHandle::Top:
            handlePos = {selectionScreenRect.x + selectionScreenRect.width * 0.5f, selectionScreenRect.y};
            break;
        case ResizeHandle::Right:
            handlePos = {selectionScreenRect.x + selectionScreenRect.width, selectionScreenRect.y + selectionScreenRect.height * 0.5f};
            break;
        case ResizeHandle::Bottom:
            handlePos = {selectionScreenRect.x + selectionScreenRect.width * 0.5f, selectionScreenRect.y + selectionScreenRect.height};
            break;
        case ResizeHandle::Left:
            handlePos = {selectionScreenRect.x, selectionScreenRect.y + selectionScreenRect.height * 0.5f};
            break;
        default:
            return {0, 0, 0, 0};
    }
    
    return {handlePos.x - handleSize*0.5f, handlePos.y - handleSize*0.5f, handleSize, handleSize};
}

void Canvas::drawResizeHandles() const
{
    if (!hasSelection_) return;
    
    const Color handleColor = WHITE;
    const Color handleBorder = BLACK;
    
    // Draw all resize handles
    for (int i = 0; i < 8; i++) {
        ResizeHandle handle = static_cast<ResizeHandle>(i);
        Rectangle handleRect = getResizeHandleRect(handle);
        
        if (handleRect.width > 0) {
            DrawRectangleRec(handleRect, handleColor);
            DrawRectangleLinesEx(handleRect, 1.0f, handleBorder);
        }
    }
}

void Canvas::updateSelectionResize(Vector2 mousePos)
{
    if (!isResizingSelection_ || resizeHandle_ == ResizeHandle::None) return;
    
    // Calculate mouse movement since resize started
    const Vector2 mouseDelta = Vector2Subtract(mousePos, resizeStartPos_);
    const Vector2 imageDelta = Vector2Scale(mouseDelta, 1.0f / zoomLevel_); // Convert screen movement to image movement
    
    // Start with the original rectangle
    Rectangle newRect = resizeStartRect_;
    
    // Apply resize based on which handle is being dragged
    switch (resizeHandle_) {
        case ResizeHandle::TopLeft:
            newRect.x += imageDelta.x;
            newRect.y += imageDelta.y;
            newRect.width -= imageDelta.x;
            newRect.height -= imageDelta.y;
            break;
        case ResizeHandle::TopRight:
            newRect.y += imageDelta.y;
            newRect.width += imageDelta.x;
            newRect.height -= imageDelta.y;
            break;
        case ResizeHandle::BottomLeft:
            newRect.x += imageDelta.x;
            newRect.width -= imageDelta.x;
            newRect.height += imageDelta.y;
            break;
        case ResizeHandle::BottomRight:
            newRect.width += imageDelta.x;
            newRect.height += imageDelta.y;
            break;
        case ResizeHandle::Top:
            newRect.y += imageDelta.y;
            newRect.height -= imageDelta.y;
            break;
        case ResizeHandle::Right:
            newRect.width += imageDelta.x;
            break;
        case ResizeHandle::Bottom:
            newRect.height += imageDelta.y;
            break;
        case ResizeHandle::Left:
            newRect.x += imageDelta.x;
            newRect.width -= imageDelta.x;
            break;
        default:
            return;
    }
    
    // Ensure minimum size
    const float minSize = 5.0f;
    if (newRect.width < minSize) {
        if (resizeHandle_ == ResizeHandle::TopLeft || resizeHandle_ == ResizeHandle::BottomLeft || resizeHandle_ == ResizeHandle::Left) {
            newRect.x = newRect.x + newRect.width - minSize;
        }
        newRect.width = minSize;
    }
    if (newRect.height < minSize) {
        if (resizeHandle_ == ResizeHandle::TopLeft || resizeHandle_ == ResizeHandle::TopRight || resizeHandle_ == ResizeHandle::Top) {
            newRect.y = newRect.y + newRect.height - minSize;
        }
        newRect.height = minSize;
    }
    
    // Update the selection rectangle
    selectionRect_ = newRect;
}

} // namespace EpiGimp