//Application update and draw loops
#include "../../include/Core/Application.hpp"
#include "../../include/UI/SimpleLayerPanel.hpp"
#include "../../include/Utils/Implementations.hpp"
#include <iostream>

namespace EpiGimp {

void Application::update(float deltaTime)
{
    inputHandler_->update();
    handleEvents();
    
    // Handle file dialogs
    auto simpleFileManager = static_cast<SimpleFileManager*>(fileManager_.get());
    
    // Check for open dialog result
    auto openResult = simpleFileManager->updateOpenDialog();
    if (openResult)
        canvas_->loadImage(*openResult);
    
    // Check for save dialog result  
    auto saveResult = simpleFileManager->updateSaveDialog();
    if (saveResult) {
        canvas_->saveImage(*saveResult);
    }
    
    // Only update other components if no dialog is showing
    if (!simpleFileManager->isShowingDialog()) {
        if (toolbar_) toolbar_->update(deltaTime);
        if (canvas_) canvas_->update(deltaTime);
        if (layerPanel_) layerPanel_->update(deltaTime);
    }
}
    
void Application::draw()
{
    if (canvas_) canvas_->draw();
    if (layerPanel_) layerPanel_->draw();
    if (toolbar_) toolbar_->draw();
    
    // Draw status bar
    const int statusY = config_.windowHeight - 25;
    DrawRectangle(0, statusY, config_.windowWidth, 25, LIGHTGRAY);
    DrawLine(0, statusY, config_.windowWidth, statusY, GRAY);
    
    const std::string statusText = canvas_->hasImage() 
        ? "Image loaded | Zoom: " + std::to_string(static_cast<int>(canvas_->getZoom() * 100)) + "%"
        : "Ready - Load an image to get started";
    
    DrawText(statusText.c_str(), 10, statusY + 5, 14, BLACK);
    
    // Draw file dialogs on top of everything
    auto simpleFileManager = static_cast<SimpleFileManager*>(fileManager_.get());
    simpleFileManager->updateOpenDialog();
    simpleFileManager->updateSaveDialog();
}

void Application::handleEvents()
{
    // Handle keyboard shortcuts
    if (inputHandler_->isKeyDown(KEY_LEFT_CONTROL) || inputHandler_->isKeyDown(KEY_RIGHT_CONTROL)) {
        if (inputHandler_->isKeyPressed(KEY_O)) {
            onLoadImageRequest();
        } else if (inputHandler_->isKeyPressed(KEY_S)) {
            eventDispatcher_->emit<ImageSaveRequestEvent>("");
        } else if (inputHandler_->isKeyPressed(KEY_W)) {
            // Undo operation (Ctrl+W for AZERTY keyboards)
            std::cout << "Ctrl+W detected!" << std::endl;
            std::cout << "Attempting undo... (History has " << historyManager_->getUndoCount() << " items)" << std::endl;
            if (historyManager_->undo()) {
                std::cout << "Undo successful: " << historyManager_->getNextRedoDescription() << std::endl;
            } else {
                std::cout << "Nothing to undo" << std::endl;
            }
        } else if (inputHandler_->isKeyPressed(KEY_Y)) {
            // Redo operation (Ctrl+Y)
            std::cout << "Ctrl+Y detected!" << std::endl;
            std::cout << "Attempting redo... (History has " << historyManager_->getRedoCount() << " items)" << std::endl;
            if (historyManager_->redo()) {
                std::cout << "Redo successful: " << historyManager_->getNextUndoDescription() << std::endl;
            } else {
                std::cout << "Nothing to redo" << std::endl;
            }
        }
    }
    
    // Only exit on ESC if no dialogs are showing
    auto simpleFileManager = static_cast<SimpleFileManager*>(fileManager_.get());
    if (inputHandler_->isKeyPressed(KEY_ESCAPE) && !simpleFileManager->isShowingDialog()) {
        running_ = false;
    }
}

} // namespace EpiGimp