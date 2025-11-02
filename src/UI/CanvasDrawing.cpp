//Canvas drawing operations functionality
#include "../../include/UI/Canvas.hpp"
#include "../../include/Commands/DrawCommand.hpp"
#include "../../include/Core/HistoryManager.hpp"
#include <iostream>
#include <cmath>

namespace EpiGimp {

void Canvas::initializeDrawingTexture()
{
    if (selectedLayerIndex_ >= 0 && selectedLayerIndex_ < static_cast<int>(drawingLayers_.size()) && currentTexture_) {
        const int width = (*currentTexture_)->width;
        const int height = (*currentTexture_)->height;
        DrawingLayer& layer = drawingLayers_[selectedLayerIndex_];
        layer.texture = RenderTextureResource(width, height);
        
        layer.texture->clear(Color{0, 0, 0, 0});
        
        std::cout << "Drawing texture initialized for layer: " << layer.name << " (" << width << "x" << height << ")" << std::endl;
    }
}

void Canvas::drawStroke(Vector2 from, Vector2 to)
{
    if (!hasDrawingTexture()) return;
    
    DrawingLayer& layer = drawingLayers_[selectedLayerIndex_];
    if (!layer.visible) return;
    
    const Rectangle imageRect = calculateImageDestRect();
    
    // Convert screen coordinates to texture coordinates (0-1 range first)
    float normalizedFromX = (from.x - imageRect.x) / imageRect.width;
    float normalizedFromY = (from.y - imageRect.y) / imageRect.height;
    float normalizedToX = (to.x - imageRect.x) / imageRect.width;
    float normalizedToY = (to.y - imageRect.y) / imageRect.height;

    if (canvasFlippedHorizontal_) {
        normalizedFromX = 1.0f - normalizedFromX;
        normalizedToX = 1.0f - normalizedToX;
    }
    if (canvasFlippedVertical_) {
        normalizedFromY = 1.0f - normalizedFromY;
        normalizedToY = 1.0f - normalizedToY;
    }
    
    // Apply layer-specific flip transformations as well
    if (layer.flippedHorizontal) {
        normalizedFromX = 1.0f - normalizedFromX;
        normalizedToX = 1.0f - normalizedToX;
    }
    if (layer.flippedVertical) {
        normalizedFromY = 1.0f - normalizedFromY;
        normalizedToY = 1.0f - normalizedToY;
    }
    
    // Convert normalized coordinates to texture pixel coordinates
    const Vector2 imageFrom = {
        normalizedFromX * (*currentTexture_)->width,
        normalizedFromY * (*currentTexture_)->height
    };
    
    const Vector2 imageTo = {
        normalizedToX * (*currentTexture_)->width,
        normalizedToY * (*currentTexture_)->height
    };
    
    std::cout << "Drawing stroke from (" << imageFrom.x << "," << imageFrom.y 
              << ") to (" << imageTo.x << "," << imageTo.y << ") on layer: " << layer.name << std::endl;
    
    layer.texture->beginDrawing();
    
    // Calculate mirrored positions if mirror mode is enabled
    Vector2 mirroredFrom = imageFrom;
    Vector2 mirroredTo = imageTo;
    
    if (mirrorModeEnabled_) {
        // Mirror horizontally across the center of the canvas
        const float centerX = (*currentTexture_)->width / 2.0f;
        mirroredFrom.x = 2.0f * centerX - imageFrom.x;
        mirroredTo.x = 2.0f * centerX - imageTo.x;
        
        std::cout << "Mirror mode enabled - also drawing from (" << mirroredFrom.x << "," << mirroredFrom.y 
                  << ") to (" << mirroredTo.x << "," << mirroredTo.y << ")" << std::endl;
    }
    
    // Lambda function to draw a stroke (used for both original and mirrored)
    auto drawSingleStroke = [&](const Vector2& from, const Vector2& to) {
        switch (currentTool_) {
            case DrawingTool::Crayon:
            case DrawingTool::Mirror:
                DrawLineEx(from, to, 3.0f, drawingColor_);
                break;
                
            case DrawingTool::Brush:
            {
                // Brush tool: larger, softer strokes with multiple passes for smoother effect
                DrawLineEx(from, to, 8.0f, drawingColor_);
                // Add some transparency for softer effect
                Color softerColor = drawingColor_;
                softerColor.a = 128; // Half transparency
                DrawLineEx(from, to, 12.0f, softerColor);
                break;
            }
            
            case DrawingTool::Airbrush:
            {
                // Airbrush tool: spray pattern with random particles
                const float sprayRadius = 15.0f;
                const int particleCount = 20; // Number of particles per stroke segment
                
                // Calculate distance between from and to points
                float distance = sqrtf((to.x - from.x) * (to.x - from.x) + (to.y - from.y) * (to.y - from.y));
                
                // Interpolate along the line and spray particles
                int steps = static_cast<int>(distance / 2.0f) + 1;
                for (int step = 0; step <= steps; ++step) {
                    float t = steps > 0 ? static_cast<float>(step) / static_cast<float>(steps) : 0.0f;
                    Vector2 pos = {
                        from.x + (to.x - from.x) * t,
                        from.y + (to.y - from.y) * t
                    };
                    
                    // Spray particles around the position
                    for (int i = 0; i < particleCount; ++i) {
                        // Random angle and distance for spray effect
                        float angle = static_cast<float>(GetRandomValue(0, 360)) * DEG2RAD;
                        float dist = static_cast<float>(GetRandomValue(0, static_cast<int>(sprayRadius * 100))) / 100.0f;
                        
                        Vector2 particlePos = {
                            pos.x + cosf(angle) * dist,
                            pos.y + sinf(angle) * dist
                        };
                        
                        // Draw particle with some transparency based on distance from center
                        Color particleColor = drawingColor_;
                        float distRatio = dist / sprayRadius;
                        particleColor.a = static_cast<unsigned char>(drawingColor_.a * (1.0f - distRatio * 0.5f));
                        
                        DrawCircle(static_cast<int>(particlePos.x), static_cast<int>(particlePos.y), 1.0f, particleColor);
                    }
                }
                break;
            }
            
            case DrawingTool::Ink:
            {
                // Ink tool: variable thickness based on drawing speed (calligraphy effect)
                // Calculate distance/speed
                float distance = sqrtf((to.x - from.x) * (to.x - from.x) + (to.y - from.y) * (to.y - from.y));
                
                // Variable thickness based on speed: slower = thicker, faster = thinner
                const float minThickness = 1.0f;
                const float maxThickness = 8.0f;
                const float speedThreshold = 20.0f; // Distance threshold for thickness variation
                
                // Calculate thickness: inversely proportional to speed
                float thickness = maxThickness - (distance / speedThreshold) * (maxThickness - minThickness);
                thickness = fmaxf(minThickness, fminf(maxThickness, thickness));
                
                // Draw main line with calculated thickness
                DrawLineEx(from, to, thickness, drawingColor_);
                
                // Add a darker edge for ink effect
                Color edgeColor = drawingColor_;
                edgeColor.r = static_cast<unsigned char>(edgeColor.r * 0.7f);
                edgeColor.g = static_cast<unsigned char>(edgeColor.g * 0.7f);
                edgeColor.b = static_cast<unsigned char>(edgeColor.b * 0.7f);
                DrawLineEx(from, to, thickness * 0.5f, edgeColor);
                
                break;
            }
            
            case DrawingTool::Blur:
            {
                // Blur tool needs special handling - it modifies existing pixels
                // Don't draw anything here, actual blur will be applied after endDrawing()
                break;
            }
            
            case DrawingTool::Burn:
            {
                // Burn tool needs special handling - it modifies existing pixels
                // Don't draw anything here, actual burn will be applied after endDrawing()
                break;
            }
            
            case DrawingTool::Dodge:
            {
                // Dodge tool needs special handling - it modifies existing pixels
                // Don't draw anything here, actual dodge will be applied after endDrawing()
                break;
            }
            
            case DrawingTool::Select:
                // Selection tool doesn't draw strokes
                break;
            
            case DrawingTool::None:
            default:
                // Fallback to basic line
                DrawLineEx(from, to, 1.0f, drawingColor_);
                break;
        }
    };
    
    // Draw the original stroke
    drawSingleStroke(imageFrom, imageTo);
    
    // Draw the mirrored stroke if mirror mode is enabled
    if (mirrorModeEnabled_) {
        drawSingleStroke(mirroredFrom, mirroredTo);
    }
    
    layer.texture->endDrawing();
    
    // Apply blur effect if using blur tool
    if (currentTool_ == DrawingTool::Blur) {
        applyBlurToLayer(layer, imageFrom, imageTo);
    }
    
    // Apply burn effect if using burn tool
    if (currentTool_ == DrawingTool::Burn) {
        applyBurnToLayer(layer, imageFrom, imageTo);
    }
    
    // Apply dodge effect if using dodge tool
    if (currentTool_ == DrawingTool::Dodge) {
        applyDodgeToLayer(layer, imageFrom, imageTo);
    }
    
    std::cout << "Stroke drawn successfully with tool: " << static_cast<int>(currentTool_) << std::endl;
}

void Canvas::handleDrawing()
{
    if (!hasImage()) return;
    
    // Skip drawing logic for selection tool and eyedropper tool
    if (currentTool_ == DrawingTool::Select || currentTool_ == DrawingTool::Eyedropper) return;
    
    // Mirror tool automatically enables mirror mode
    if (currentTool_ == DrawingTool::Mirror) {
        mirrorModeEnabled_ = true;
    }
    
    const Vector2 mousePos = GetMousePosition();
    
    const Rectangle imageRect = calculateImageDestRect();
    
    if (!CheckCollisionPointRec(mousePos, imageRect))
        return;
    
    static bool isDrawingLeft = false;
    static bool isDrawingRight = false;
    static Vector2 lastMousePos = {0, 0};
    static std::unique_ptr<DrawCommand> currentDrawCommand = nullptr;
    static Color currentStrokeColor = BLACK;
    
    // Handle left mouse button (primary color)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        std::cout << "Left mouse pressed at (" << mousePos.x << "," << mousePos.y << "), tool=" << static_cast<int>(currentTool_) 
                  << ", primary color=RGB(" << static_cast<int>(primaryColor_.r) << "," 
                  << static_cast<int>(primaryColor_.g) << "," << static_cast<int>(primaryColor_.b) << ")" << std::endl;
        if (currentTool_ != DrawingTool::None) {
            isDrawingLeft = true;
            lastMousePos = mousePos;
            currentStrokeColor = primaryColor_;
            
            // Create a new draw command if history manager is available
            if (historyManager_) {
                currentDrawCommand = createDrawCommand(this, "Primary Color Stroke");
                std::cout << "Started primary color stroke, captured before state" << std::endl;
            }
        } else {
            std::cout << "Drawing tool is NONE - cannot draw" << std::endl;
        }
    }
    
    // Handle right mouse button (secondary color)
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        std::cout << "Right mouse pressed at (" << mousePos.x << "," << mousePos.y << "), tool=" << static_cast<int>(currentTool_) 
                  << ", secondary color=RGB(" << static_cast<int>(secondaryColor_.r) << "," 
                  << static_cast<int>(secondaryColor_.g) << "," << static_cast<int>(secondaryColor_.b) << ")" << std::endl;
        if (currentTool_ != DrawingTool::None) {
            isDrawingRight = true;
            lastMousePos = mousePos;
            currentStrokeColor = secondaryColor_;
            
            // Create a new draw command if history manager is available
            if (historyManager_) {
                currentDrawCommand = createDrawCommand(this, "Secondary Color Stroke");
                std::cout << "Started secondary color stroke, captured before state" << std::endl;
            }
        } else {
            std::cout << "Drawing tool is NONE - cannot draw" << std::endl;
        }
    }
    
    // Handle drawing while mouse is held down
    if ((IsMouseButtonDown(MOUSE_BUTTON_LEFT) && isDrawingLeft) || 
        (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && isDrawingRight)) {
        if (currentTool_ != DrawingTool::None) {
            std::cout << "Drawing from (" << lastMousePos.x << "," << lastMousePos.y << ") to (" << mousePos.x << "," << mousePos.y << ")" 
                      << " with color RGB(" << static_cast<int>(currentStrokeColor.r) << "," 
                      << static_cast<int>(currentStrokeColor.g) << "," << static_cast<int>(currentStrokeColor.b) << ")" << std::endl;
            // Temporarily set drawing color for the stroke
            Color originalColor = drawingColor_;
            drawingColor_ = currentStrokeColor;
            drawStroke(lastMousePos, mousePos);
            drawingColor_ = originalColor; // Restore original color
            lastMousePos = mousePos;
        }
    }
    
    // Handle mouse button release
    if ((IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && isDrawingLeft) || 
        (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && isDrawingRight)) {
        if ((isDrawingLeft || isDrawingRight) && currentDrawCommand && historyManager_) {
            std::cout << "Mouse released, capturing after state" << std::endl;
            // Capture the after state and execute the command
            currentDrawCommand->captureAfterState();
            
            if (historyManager_->executeCommand(std::move(currentDrawCommand))) {
                std::cout << "Drawing stroke completed and added to history" << std::endl;
            } else {
                std::cout << "Failed to add drawing stroke to history" << std::endl;
            }
            
            currentDrawCommand = nullptr;
        }
        isDrawingLeft = false;
        isDrawingRight = false;
    }
}

void Canvas::onColorChanged(const ColorChangedEvent& event)
{
    // Keep for backward compatibility - set as primary color
    primaryColor_ = event.selectedColor;
    drawingColor_ = event.selectedColor;
    std::cout << "Canvas: Drawing color changed to RGB(" 
              << static_cast<int>(drawingColor_.r) << ", "
              << static_cast<int>(drawingColor_.g) << ", "
              << static_cast<int>(drawingColor_.b) << ", "
              << static_cast<int>(drawingColor_.a) << ")" << std::endl;
}

void Canvas::onPrimaryColorChanged(const PrimaryColorChangedEvent& event)
{
    primaryColor_ = event.primaryColor;
    drawingColor_ = primaryColor_; // Update drawing color for compatibility
    std::cout << "Canvas: Primary color changed to RGB(" 
              << static_cast<int>(primaryColor_.r) << ", "
              << static_cast<int>(primaryColor_.g) << ", "
              << static_cast<int>(primaryColor_.b) << ", "
              << static_cast<int>(primaryColor_.a) << ")" << std::endl;
}

void Canvas::onSecondaryColorChanged(const SecondaryColorChangedEvent& event)
{
    secondaryColor_ = event.secondaryColor;
    std::cout << "Canvas: Secondary color changed to RGB(" 
              << static_cast<int>(secondaryColor_.r) << ", "
              << static_cast<int>(secondaryColor_.g) << ", "
              << static_cast<int>(secondaryColor_.b) << ", "
              << static_cast<int>(secondaryColor_.a) << ")" << std::endl;
}

void Canvas::applyBlurToLayer(DrawingLayer& layer, Vector2 from, Vector2 to)
{
    const float blurRadius = 10.0f;
    const int blurKernelSize = 2;
    
    // Load the layer texture as an image
    Image layerImage = LoadImageFromTexture((**layer.texture).texture);
    Image originalImage = ImageCopy(layerImage);
    
    std::cout << "Blur from (" << from.x << "," << from.y << ") to (" << to.x << "," << to.y << ")" << std::endl;
    std::cout << "Image size: " << layerImage.width << "x" << layerImage.height << std::endl;
    
    // Raylib textures are flipped vertically when loaded as images
    // We need to flip the Y coordinates
    Vector2 flippedFrom = { from.x, layerImage.height - from.y };
    Vector2 flippedTo = { to.x, layerImage.height - to.y };
    
    std::cout << "Flipped from (" << flippedFrom.x << "," << flippedFrom.y << ") to (" << flippedTo.x << "," << flippedTo.y << ")" << std::endl;
    
    // Calculate distance for interpolation
    float distance = sqrtf((flippedTo.x - flippedFrom.x) * (flippedTo.x - flippedFrom.x) + (flippedTo.y - flippedFrom.y) * (flippedTo.y - flippedFrom.y));
    int steps = static_cast<int>(distance / 2.0f) + 1;
    
    for (int step = 0; step <= steps; ++step) {
        float t = steps > 0 ? static_cast<float>(step) / static_cast<float>(steps) : 0.0f;
        Vector2 pos = {
            flippedFrom.x + (flippedTo.x - flippedFrom.x) * t,
            flippedFrom.y + (flippedTo.y - flippedFrom.y) * t
        };
        
        int centerX = static_cast<int>(pos.x);
        int centerY = static_cast<int>(pos.y);
        int radius = static_cast<int>(blurRadius);
        
        // Apply blur in circular brush area
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx * dx + dy * dy > radius * radius) continue;
                
                int px = centerX + dx;
                int py = centerY + dy;
                
                if (px < 0 || py < 0 || px >= originalImage.width || py >= originalImage.height) continue;
                
                // Average surrounding pixels
                int sumR = 0, sumG = 0, sumB = 0, sumA = 0;
                int count = 0;
                
                for (int ky = -blurKernelSize; ky <= blurKernelSize; ky++) {
                    for (int kx = -blurKernelSize; kx <= blurKernelSize; kx++) {
                        int sx = px + kx;
                        int sy = py + ky;
                        
                        if (sx >= 0 && sy >= 0 && sx < originalImage.width && sy < originalImage.height) {
                            Color c = GetImageColor(originalImage, sx, sy);
                            sumR += c.r;
                            sumG += c.g;
                            sumB += c.b;
                            sumA += c.a;
                            count++;
                        }
                    }
                }
                
                if (count > 0) {
                    Color blurredColor = {
                        static_cast<unsigned char>(sumR / count),
                        static_cast<unsigned char>(sumG / count),
                        static_cast<unsigned char>(sumB / count),
                        static_cast<unsigned char>(sumA / count)
                    };
                    ImageDrawPixel(&layerImage, px, py, blurredColor);
                }
            }
        }
    }
    
    // Update the layer texture
    UpdateTexture((**layer.texture).texture, layerImage.data);
    
    UnloadImage(originalImage);
    UnloadImage(layerImage);
}

void Canvas::applyBurnToLayer(DrawingLayer& layer, Vector2 from, Vector2 to)
{
    const float burnRadius = 15.0f; // Radius of burn brush
    const float burnAmount = 15.0f; // How much to darken per pass (0-255 scale)
    
    // Load the layer texture as an image
    Image layerImage = LoadImageFromTexture((**layer.texture).texture);
    
    std::cout << "Burn from (" << from.x << "," << from.y << ") to (" << to.x << "," << to.y << ")" << std::endl;
    
    // Flip Y coordinates (same as blur tool)
    Vector2 flippedFrom = { from.x, layerImage.height - from.y };
    Vector2 flippedTo = { to.x, layerImage.height - to.y };
    
    // Calculate distance for interpolation
    float distance = sqrtf((flippedTo.x - flippedFrom.x) * (flippedTo.x - flippedFrom.x) + (flippedTo.y - flippedFrom.y) * (flippedTo.y - flippedFrom.y));
    int steps = static_cast<int>(distance / 2.0f) + 1;
    
    for (int step = 0; step <= steps; ++step) {
        float t = steps > 0 ? static_cast<float>(step) / static_cast<float>(steps) : 0.0f;
        Vector2 pos = {
            flippedFrom.x + (flippedTo.x - flippedFrom.x) * t,
            flippedFrom.y + (flippedTo.y - flippedFrom.y) * t
        };
        
        int centerX = static_cast<int>(pos.x);
        int centerY = static_cast<int>(pos.y);
        int radius = static_cast<int>(burnRadius);
        
        // Apply burn in circular brush area
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                // Calculate distance from center for soft brush falloff
                float dist = sqrtf(static_cast<float>(dx * dx + dy * dy));
                if (dist > burnRadius) continue;
                
                int px = centerX + dx;
                int py = centerY + dy;
                
                if (px < 0 || py < 0 || px >= layerImage.width || py >= layerImage.height) continue;
                
                // Get current pixel color
                Color currentColor = GetImageColor(layerImage, px, py);
                
                // Skip fully transparent pixels
                if (currentColor.a == 0) continue;
                
                // Calculate falloff based on distance from center (softer edges)
                float falloff = 1.0f - (dist / burnRadius);
                float effectiveBurn = burnAmount * falloff;
                
                // Darken the pixel by subtracting from RGB values
                // This allows gradual darkening with multiple passes
                Color darkenedColor = {
                    static_cast<unsigned char>(currentColor.r > effectiveBurn ? currentColor.r - effectiveBurn : 0),
                    static_cast<unsigned char>(currentColor.g > effectiveBurn ? currentColor.g - effectiveBurn : 0),
                    static_cast<unsigned char>(currentColor.b > effectiveBurn ? currentColor.b - effectiveBurn : 0),
                    currentColor.a // Keep alpha unchanged
                };
                
                ImageDrawPixel(&layerImage, px, py, darkenedColor);
            }
        }
    }
    
    // Update the layer texture
    UpdateTexture((**layer.texture).texture, layerImage.data);
    
    UnloadImage(layerImage);
}

void Canvas::applyDodgeToLayer(DrawingLayer& layer, Vector2 from, Vector2 to)
{
    const float dodgeRadius = 15.0f; // Radius of dodge brush
    const float dodgeAmount = 15.0f; // How much to lighten per pass (0-255 scale)
    
    // Load the layer texture as an image
    Image layerImage = LoadImageFromTexture((**layer.texture).texture);
    
    std::cout << "Dodge from (" << from.x << "," << from.y << ") to (" << to.x << "," << to.y << ")" << std::endl;
    
    // Flip Y coordinates (same as blur/burn tool)
    Vector2 flippedFrom = { from.x, layerImage.height - from.y };
    Vector2 flippedTo = { to.x, layerImage.height - to.y };
    
    // Calculate distance for interpolation
    float distance = sqrtf((flippedTo.x - flippedFrom.x) * (flippedTo.x - flippedFrom.x) + (flippedTo.y - flippedFrom.y) * (flippedTo.y - flippedFrom.y));
    int steps = static_cast<int>(distance / 2.0f) + 1;
    
    for (int step = 0; step <= steps; ++step) {
        float t = steps > 0 ? static_cast<float>(step) / static_cast<float>(steps) : 0.0f;
        Vector2 pos = {
            flippedFrom.x + (flippedTo.x - flippedFrom.x) * t,
            flippedFrom.y + (flippedTo.y - flippedFrom.y) * t
        };
        
        int centerX = static_cast<int>(pos.x);
        int centerY = static_cast<int>(pos.y);
        int radius = static_cast<int>(dodgeRadius);
        
        // Apply dodge in circular brush area
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                // Calculate distance from center for soft brush falloff
                float dist = sqrtf(static_cast<float>(dx * dx + dy * dy));
                if (dist > dodgeRadius) continue;
                
                int px = centerX + dx;
                int py = centerY + dy;
                
                if (px < 0 || py < 0 || px >= layerImage.width || py >= layerImage.height) continue;
                
                // Get current pixel color
                Color currentColor = GetImageColor(layerImage, px, py);
                
                // Skip fully transparent pixels
                if (currentColor.a == 0) continue;
                
                // Calculate falloff based on distance from center (softer edges)
                float falloff = 1.0f - (dist / dodgeRadius);
                float effectiveDodge = dodgeAmount * falloff;
                
                // Lighten the pixel by adding to RGB values
                // This allows gradual lightening with multiple passes
                Color lightenedColor = {
                    static_cast<unsigned char>(currentColor.r + effectiveDodge > 255 ? 255 : currentColor.r + effectiveDodge),
                    static_cast<unsigned char>(currentColor.g + effectiveDodge > 255 ? 255 : currentColor.g + effectiveDodge),
                    static_cast<unsigned char>(currentColor.b + effectiveDodge > 255 ? 255 : currentColor.b + effectiveDodge),
                    currentColor.a // Keep alpha unchanged
                };
                
                ImageDrawPixel(&layerImage, px, py, lightenedColor);
            }
        }
    }
    
    // Update the layer texture
    UpdateTexture((**layer.texture).texture, layerImage.data);
    
    UnloadImage(layerImage);
}

} // namespace EpiGimp