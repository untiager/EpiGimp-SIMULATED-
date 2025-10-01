#include "Core/RaylibWrappers.hpp"
#include <algorithm>
#include <filesystem>
#include <cctype>

namespace EpiGimp {

void ImageResource::resize(int width, int height)
{
    if (isValid())
        ImageResize(getMutable(), width, height);
}

bool ImageResource::exportToFile(const std::string& path) const
{
    std::string actualPath;
    return exportToFile(path, actualPath);
}

bool ImageResource::exportToFile(const std::string& path, std::string& actualPath) const
{
    if (!isValid())
        return false;
    
    actualPath = validateAndFixExtension(path);
    if (actualPath.empty())
        return false; // Invalid extension and couldn't fix
    
    return ExportImage(*image_, actualPath.c_str());
}

std::string ImageResource::validateAndFixExtension(const std::string& path) const
{
    std::filesystem::path filePath(path);
    std::string extension = filePath.extension().string();
    
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    std::vector<std::string> supportedExts = {".png", ".bmp", ".tga", ".jpg", ".jpeg"};
    
    if (std::find(supportedExts.begin(), supportedExts.end(), extension) != supportedExts.end())
        return path; // Extension is valid, return as-is
    
    if (extension.empty())
        return path + ".png";
    
    std::string newPath = filePath.replace_extension(".png").string();
    return newPath;
}

WindowResource::WindowResource(int width, int height, const std::string& title) 
    : initialized_(false)
{
    InitWindow(width, height, title.c_str());
    if (IsWindowReady()) {
        initialized_ = true;
        SetTargetFPS(60);
    }
}

WindowResource::~WindowResource()
{
    if (initialized_)
        CloseWindow();
}

bool WindowResource::shouldClose() const
{ 
    return WindowShouldClose(); 
}

int WindowResource::getWidth() const
{ 
    return GetScreenWidth(); 
}

int WindowResource::getHeight() const
{ 
    return GetScreenHeight(); 
}

RenderTextureResource::RenderTextureResource(int width, int height) 
    : renderTexture_(new RenderTexture2D(LoadRenderTexture(width, height)), [](RenderTexture2D* rt){
        if (rt && rt->id > 0)
            UnloadRenderTexture(*rt);
        delete rt;
    }) {}

void RenderTextureResource::beginDrawing() const
{
    if (isValid())
        BeginTextureMode(*renderTexture_);
}

void RenderTextureResource::endDrawing() const
{
    if (isValid())
        EndTextureMode();
}

void RenderTextureResource::clear(Color color) const
{
    if (isValid()) {
        beginDrawing();
        ClearBackground(color);
        endDrawing();
    }
}

} // namespace EpiGimp