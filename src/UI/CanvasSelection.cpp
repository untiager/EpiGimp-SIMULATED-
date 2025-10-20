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
        // Start new selection
        selectionStart_ = mousePos;
        selectionEnd_ = mousePos;
        isSelecting_ = true;
        hasSelection_ = false; // Clear existing selection while dragging
        
        std::cout << "Started selection at (" << mousePos.x << "," << mousePos.y << ")" << std::endl;
    }
    
    if (isSelecting_ && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        // Update selection end point
        selectionEnd_ = mousePos;
    }
    
    if (isSelecting_ && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
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
    
    // Create marching ants pattern
    Color lineColor = isSelecting_ ? BLUE : BLACK;
    Color bgColor = isSelecting_ ? LIGHTGRAY : WHITE;
    
    // Draw background outline
    DrawRectangleLinesEx(screenRect, 1.0f, bgColor);
    
    // Draw marching ants by drawing dashed lines
    drawMarchingAnts(screenRect, lineColor, dashLength, animOffset);
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

} // namespace EpiGimp