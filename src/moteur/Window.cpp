#include "moteur/Window.h"
#include <glad/gl.h>

namespace moteur {
Window::~Window() { close(); }
bool Window::initialize(const std::string& title, int width, int height, bool hidden) {
    error.clear();
    if (window) { error = "La fenetre est deja creee."; return false; }
    if (width <= 0 || height <= 0) { error = "Dimensions invalides."; return false; }
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) ||
        !SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {
        error = SDL_GetError(); return false;
    }
    const SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
        (hidden ? SDL_WINDOW_HIDDEN : 0);
    window = SDL_CreateWindow(title.c_str(), width, height, flags);
    if (!window) { error = SDL_GetError(); return false; }
    context = SDL_GL_CreateContext(window);
    if (!context || !makeCurrent()) { error = SDL_GetError(); close(); return false; }
    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress))) {
        error = "Chargement de GLAD impossible."; close(); return false;
    }
    SDL_GL_SetSwapInterval(1); // Non bloquant si le pilote refuse la VSync.
    return true;
}
bool Window::makeCurrent() const { return window && context && SDL_GL_MakeCurrent(window, context); }
bool Window::swapBuffers() const { return window && SDL_GL_SwapWindow(window); }
void Window::close() {
    if (context) { SDL_GL_DestroyContext(context); context = nullptr; }
    if (window) { SDL_DestroyWindow(window); window = nullptr; }
}
bool Window::setTitle(const std::string& title) { return window && SDL_SetWindowTitle(window, title.c_str()); }
SDL_WindowID Window::getId() const { return window ? SDL_GetWindowID(window) : 0; }
bool Window::getPixelSize(int& width, int& height) const {
    return window && SDL_GetWindowSizeInPixels(window, &width, &height);
}
}
