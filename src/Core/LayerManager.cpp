#include "../../include/Core/LayerManager.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace EpiGimp {

LayerManager::LayerManager(int width, int height, EventDispatcher* dispatcher)
    : activeLayerIndex_(0)
    , eventDispatcher_(dispatcher)
    , canvasWidth_(width)
    , canvasHeight_(height)
{
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("Canvas dimensions must be positive");
    ensureDefaultLayer();
}

size_t LayerManager::createLayer(const std::string& name)
{
    auto layer = std::make_unique<Layer>(name, canvasWidth_, canvasHeight_);
    layers_.push_back(std::move(layer));
    size_t newIndex = layers_.size() - 1;
    
    notifyLayerCreated(newIndex);
    return newIndex;
}

bool LayerManager::deleteLayer(size_t index)
{
    if (!isValidIndex(index) || layers_.size() <= 1)
        return false; // Can't delete the last layer
    
    std::string layerName = layers_[index]->getName();
    layers_.erase(layers_.begin() + index);

    if (activeLayerIndex_ >= layers_.size()) {
        size_t oldIndex = activeLayerIndex_;
        activeLayerIndex_ = layers_.size() - 1;
        notifyActiveLayerChanged(oldIndex, activeLayerIndex_);
    } else if (activeLayerIndex_ > index) {
        size_t oldIndex = activeLayerIndex_;
        activeLayerIndex_--;
        notifyActiveLayerChanged(oldIndex, activeLayerIndex_);
    }
    
    notifyLayerDeleted(index, layerName);
    return true;
}

bool LayerManager::moveLayer(size_t fromIndex, size_t toIndex)
{
    if (!isValidIndex(fromIndex) || !isValidIndex(toIndex) || fromIndex == toIndex)
        return false;
    
    auto layer = std::move(layers_[fromIndex]);
    layers_.erase(layers_.begin() + fromIndex);
    layers_.insert(layers_.begin() + toIndex, std::move(layer));
    
    if (activeLayerIndex_ == fromIndex) {
        size_t oldIndex = activeLayerIndex_;
        activeLayerIndex_ = toIndex;
        notifyActiveLayerChanged(oldIndex, activeLayerIndex_);
    } else if (activeLayerIndex_ > fromIndex && activeLayerIndex_ <= toIndex) {
        size_t oldIndex = activeLayerIndex_;
        activeLayerIndex_--;
        notifyActiveLayerChanged(oldIndex, activeLayerIndex_);
    } else if (activeLayerIndex_ < fromIndex && activeLayerIndex_ >= toIndex) {
        size_t oldIndex = activeLayerIndex_;
        activeLayerIndex_++;
        notifyActiveLayerChanged(oldIndex, activeLayerIndex_);
    }
    
    notifyLayerReordered(fromIndex, toIndex);
    return true;
}

bool LayerManager::duplicateLayer(size_t index)
{
    if (!isValidIndex(index))
        return false;
    
    const Layer* sourceLayer = layers_[index].get();
    std::string newName = sourceLayer->getName() + " Copy";
    
    auto newLayer = std::make_unique<Layer>(newName, canvasWidth_, canvasHeight_);
    newLayer->setVisible(sourceLayer->isVisible());
    newLayer->setOpacity(sourceLayer->getOpacity());
    newLayer->setBlendMode(sourceLayer->getBlendMode());
    
    if (sourceLayer->hasTexture()) {
        Image sourceImage = sourceLayer->copyImage();
        newLayer->restoreImage(sourceImage);
        UnloadImage(sourceImage);
    }
    
    layers_.insert(layers_.begin() + index + 1, std::move(newLayer));
    
    if (activeLayerIndex_ > index) {
        size_t oldIndex = activeLayerIndex_;
        activeLayerIndex_++;
        notifyActiveLayerChanged(oldIndex, activeLayerIndex_);
    }
    
    notifyLayerCreated(index + 1);
    return true;
}

Layer* LayerManager::getLayer(size_t index)
{
    return isValidIndex(index) ? layers_[index].get() : nullptr;
}

const Layer* LayerManager::getLayer(size_t index) const
{
    return isValidIndex(index) ? layers_[index].get() : nullptr;
}

Layer* LayerManager::getActiveLayer()
{
    return getLayer(activeLayerIndex_);
}

const Layer* LayerManager::getActiveLayer() const
{
    return getLayer(activeLayerIndex_);
}

bool LayerManager::setActiveLayer(size_t index)
{
    if (!isValidIndex(index) || index == activeLayerIndex_)
        return false;
    
    size_t oldIndex = activeLayerIndex_;
    activeLayerIndex_ = index;
    notifyActiveLayerChanged(oldIndex, activeLayerIndex_);
    return true;
}

bool LayerManager::setLayerVisibility(size_t index, bool visible)
{
    Layer* layer = getLayer(index);
    if (!layer)
        return false;
    
    layer->setVisible(visible);
    notifyLayerVisibilityChanged(index, visible);
    return true;
}

bool LayerManager::setLayerOpacity(size_t index, float opacity)
{
    Layer* layer = getLayer(index);
    if (!layer)
        return false;
    
    layer->setOpacity(opacity);
    return true;
}

bool LayerManager::setLayerBlendMode(size_t index, BlendMode mode)
{
    Layer* layer = getLayer(index);
    if (!layer)
        return false;
    
    layer->setBlendMode(mode);
    return true;
}

bool LayerManager::setLayerName(size_t index, const std::string& name)
{
    Layer* layer = getLayer(index);
    if (!layer)
        return false;
    
    layer->setName(name);
    return true;
}

void LayerManager::resizeAllLayers(int width, int height)
{
    if (width <= 0 || height <= 0)
        return;
    
    canvasWidth_ = width;
    canvasHeight_ = height;
    
    for (auto& layer : layers_) {
        layer->resize(width, height);
    }
}

void LayerManager::renderComposite(RenderTexture2D& compositeTexture) const
{
    BeginTextureMode(compositeTexture);
    ClearBackground(BLUE);  // Use blue background so we can see if composite works
    
    static int debugCount = 0;
    debugCount++;
    
    DrawRectangle(50, 50, 200, 200, RED);
    DrawRectangle(300, 300, 100, 100, GREEN);
    
    int visibleLayers = 0;
    for (const auto& layer : layers_) {
        if (layer && layer->isVisible()) {
            visibleLayers++;
            if (layer->hasTexture()) {
                const auto& texture = layer->getTexture();
                Color tint = WHITE;
                tint.a = static_cast<unsigned char>(layer->getOpacity() * 255);
                DrawTexture((*texture).texture, 0, 0, tint);
                
                if (debugCount % 120 == 0) {
                    std::cout << "Rendering layer " << layer->getName() 
                              << " to composite, opacity: " << layer->getOpacity() << std::endl;
                }
            }
        }
    }
    
    if (debugCount % 120 == 0)
        std::cout << "Rendered " << visibleLayers << " layers to composite with test rectangles" << std::endl;
    
    EndTextureMode();
}

void LayerManager::clear()
{
    for (auto& layer : layers_) {
        layer->clear(BLANK);
    }
}

std::vector<std::string> LayerManager::getLayerNames() const
{
    std::vector<std::string> names;
    names.reserve(layers_.size());
    
    for (const auto& layer : layers_) {
        names.push_back(layer->getName());
    }
    
    return names;
}

int LayerManager::findLayerByName(const std::string& name) const
{
    for (size_t i = 0; i < layers_.size(); ++i) {
        if (layers_[i]->getName() == name)
            return static_cast<int>(i);
    }
    return -1;
}

void LayerManager::ensureDefaultLayer()
{
    if (layers_.empty()) {
        createLayer("Background");
        // Clear the background layer to white
        if (!layers_.empty())
            layers_[0]->clear(WHITE);
    }
}

bool LayerManager::isValidIndex(size_t index) const
{
    return index < layers_.size();
}

void LayerManager::notifyLayerCreated(size_t index)
{
    if (eventDispatcher_) {
        LayerCreatedEvent event;
        event.layerIndex = index;
        event.layerName = layers_[index]->getName();
        eventDispatcher_->publish(event);
    }
}

void LayerManager::notifyLayerDeleted(size_t index, const std::string& name)
{
    if (eventDispatcher_) {
        LayerDeletedEvent event;
        event.layerIndex = index;
        event.layerName = name;
        eventDispatcher_->publish(event);
    }
}

void LayerManager::notifyLayerVisibilityChanged(size_t index, bool visible)
{
    if (eventDispatcher_) {
        LayerVisibilityChangedEvent event;
        event.layerIndex = index;
        event.visible = visible;
        eventDispatcher_->publish(event);
    }
}

void LayerManager::notifyLayerReordered(size_t fromIndex, size_t toIndex)
{
    if (eventDispatcher_) {
        LayerReorderedEvent event;
        event.fromIndex = fromIndex;
        event.toIndex = toIndex;
        eventDispatcher_->publish(event);
    }
}

void LayerManager::notifyActiveLayerChanged(size_t oldIndex, size_t newIndex)
{
    if (eventDispatcher_) {
        ActiveLayerChangedEvent event;
        event.oldIndex = oldIndex;
        event.newIndex = newIndex;
        eventDispatcher_->publish(event);
    }
}

} // namespace EpiGimp