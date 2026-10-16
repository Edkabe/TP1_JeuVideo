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
    SDL_Window* getSDLWindow() const { return window; }
    SDL_WindowID getId() const;
    bool getPixelSize(int& width, int& height) const;
    const std::string& getError() const { return error; }
private:
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    std::string error;
};
}
