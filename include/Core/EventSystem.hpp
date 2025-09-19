//Modern event system using observer pattern
#ifndef EVENT_SYSTEM_HPP
#define EVENT_SYSTEM_HPP

#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <any>
#include <string>

namespace EpiGimp {

enum class DrawingTool;

// Base event class
class Event {
public:
    virtual ~Event() = default;
};

// Event types
class ImageLoadedEvent : public Event {
public:
    std::string filePath;
    explicit ImageLoadedEvent(std::string path) : filePath(std::move(path)) {}
};

class ImageSaveRequestEvent : public Event {
public:
    std::string filePath;
    explicit ImageSaveRequestEvent(std::string path) : filePath(std::move(path)) {}
};

class ImageSavedEvent : public Event {
public:
    std::string filePath;
    bool success;
    ImageSavedEvent(std::string path, bool succeeded) : filePath(std::move(path)), success(succeeded) {}
};

class LoadImageRequestEvent : public Event {};

class ToolSelectedEvent : public Event {
public:
    DrawingTool toolType;
    explicit ToolSelectedEvent(DrawingTool type) : toolType(type) {}
};

class ColorChangedEvent : public Event {
public:
    Color selectedColor;
    explicit ColorChangedEvent(Color color) : selectedColor(color) {}
};

class ErrorEvent : public Event {
public:
    std::string message;
    explicit ErrorEvent(std::string msg) : message(std::move(msg)) {}
};

// Event handler interface
template<typename EventType>
using EventHandler = std::function<void(const EventType&)>;

// Event dispatcher
class EventDispatcher {
private:
    std::unordered_map<std::type_index, std::vector<std::function<void(const Event&)>>> handlers_;

public:
    template<typename EventType>
    void subscribe(EventHandler<EventType> handler) {
        auto wrapper = [handler](const Event& event) {
            handler(static_cast<const EventType&>(event));
        };
        handlers_[std::type_index(typeid(EventType))].push_back(wrapper);
    }

    template<typename EventType>
    void publish(const EventType& event) {
        auto it = handlers_.find(std::type_index(typeid(EventType)));
        if (it != handlers_.end()) {
            for (const auto& handler : it->second) {
                handler(event);
            }
        }
    }

    template<typename EventType, typename... Args>
    void emit(Args&&... args) {
        EventType event(std::forward<Args>(args)...);
        publish(event);
    }
};

} // namespace EpiGimp

#endif // EVENT_SYSTEM_HPP