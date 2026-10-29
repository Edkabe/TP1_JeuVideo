#pragma once
#include <SDL3/SDL.h>
#include <string>

namespace moteur {
class Window {
public:
    Window() = default;
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    bool initialize(const std::string& title, int width, int height, bool hidden = false);
    bool makeCurrent() const;
    bool swapBuffers() const;
    void close();
    bool setTitle(const std::string& title);
    // Applique resolution fenetree, mode plein ecran et synchronisation VSync atomiquement.
    bool applyDisplaySettings(int width, int height, bool fullscreen, bool vsync);
    bool getSize(int& width, int& height) const;
    bool isFullscreen() const;
    bool isVSyncEnabled() const { return vsyncEnabled; }
    SDL_Window* getSDLWindow() const { return window; }
    SDL_WindowID getId() const;
    bool getPixelSize(int& width, int& height) const;
    const std::string& getError() const { return error; }
private:
    bool setWindowedSize(int width, int height);
    bool setFullscreen(bool enabled);
    bool setVSync(bool enabled);
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    int windowedWidth = 0;
    int windowedHeight = 0;
    bool vsyncEnabled = false;
    std::string error;
};
}
