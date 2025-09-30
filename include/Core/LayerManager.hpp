#ifndef LAYERMANAGER_HPP
#define LAYERMANAGER_HPP

#include <vector>
#include <memory>
#include <string>
#include <optional>
#include "Layer.hpp"
#include "EventSystem.hpp"

namespace EpiGimp {

/**
 * @brief Events for layer system
 */
struct LayerCreatedEvent : public Event {
    size_t layerIndex;
    std::string layerName;
};

struct LayerDeletedEvent : public Event {
    size_t layerIndex;
    std::string layerName;
};

struct LayerVisibilityChangedEvent : public Event {
    size_t layerIndex;
    bool visible;
};

struct LayerReorderedEvent : public Event {
    size_t fromIndex;
    size_t toIndex;
};

struct ActiveLayerChangedEvent : public Event {
    size_t oldIndex;
    size_t newIndex;
};

/**
 * @brief Manages a collection of layers for the canvas
 */
class LayerManager {
private:
    std::vector<std::unique_ptr<Layer>> layers_;
    size_t activeLayerIndex_;
    EventDispatcher* eventDispatcher_;
    int canvasWidth_;
    int canvasHeight_;

public:
    explicit LayerManager(int width, int height, EventDispatcher* dispatcher = nullptr);
    ~LayerManager() = default;

    // Non-copyable but moveable
    LayerManager(const LayerManager&) = delete;
    LayerManager& operator=(const LayerManager&) = delete;
    LayerManager(LayerManager&&) = default;
    LayerManager& operator=(LayerManager&&) = default;

    // Layer management
    size_t createLayer(const std::string& name);
    bool deleteLayer(size_t index);
    bool moveLayer(size_t fromIndex, size_t toIndex);
    bool duplicateLayer(size_t index);

    // Layer access
    Layer* getLayer(size_t index);
    const Layer* getLayer(size_t index) const;
    Layer* getActiveLayer();
    const Layer* getActiveLayer() const;
    size_t getLayerCount() const { return layers_.size(); }
    size_t getActiveLayerIndex() const { return activeLayerIndex_; }

    // Layer selection
    bool setActiveLayer(size_t index);
    
    // Layer properties
    bool setLayerVisibility(size_t index, bool visible);
    bool setLayerOpacity(size_t index, float opacity);
    bool setLayerBlendMode(size_t index, BlendMode mode);
    bool setLayerName(size_t index, const std::string& name);

    // Canvas operations
    void resizeAllLayers(int width, int height);
    void renderComposite(RenderTexture2D& target) const;
    void clear();

    // Utility
    std::vector<std::string> getLayerNames() const;
    int findLayerByName(const std::string& name) const;

private:
    void ensureDefaultLayer();
    bool isValidIndex(size_t index) const;
    void notifyLayerCreated(size_t index);
    void notifyLayerDeleted(size_t index, const std::string& name);
    void notifyLayerVisibilityChanged(size_t index, bool visible);
    void notifyLayerReordered(size_t fromIndex, size_t toIndex);
    void notifyActiveLayerChanged(size_t oldIndex, size_t newIndex);
};

} // namespace EpiGimp

#endif // LAYERMANAGER_HPP