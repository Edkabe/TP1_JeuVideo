#include "moteur/Window.h"
#include <glad/gl.h>
#include <cmath>
#include <string>

namespace moteur {
namespace {
const SDL_DisplayMode* findMode(SDL_DisplayMode** modes, int count,
                               const SDL_DisplayMode& wanted) {
    const SDL_DisplayMode* best = nullptr;
    for (int i = 0; modes && i < count; ++i) {
        const auto* mode = modes[i];
        if (!mode || mode->w != wanted.w || mode->h != wanted.h ||
            mode->format != wanted.format ||
            std::fabs(mode->refresh_rate - wanted.refresh_rate) > 0.01f) continue;
        if (!best || mode->refresh_rate > best->refresh_rate) best = mode;
    }
    return best;
}

const SDL_DisplayMode* findResolution(SDL_DisplayMode** modes, int count,
                                     int width, int height) {
    const SDL_DisplayMode* best = nullptr;
    for (int i = 0; modes && i < count; ++i) {
        const auto* mode = modes[i];
        if (!mode || mode->w != width || mode->h != height) continue;
        if (!best || mode->refresh_rate > best->refresh_rate) best = mode;
    }
    return best;
}
}

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
    if (!SDL_SetWindowMinimumSize(window, 320, 540)) {
        error = SDL_GetError(); close(); return false;
    }
    windowedWidth = width;
    windowedHeight = height;
    context = SDL_GL_CreateContext(window);
    if (!context || !makeCurrent()) { error = SDL_GetError(); close(); return false; }
    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress))) {
        error = "Chargement de GLAD impossible."; close(); return false;
    }
    vsyncEnabled = false;
    return true;
}
bool Window::makeCurrent() const { return window && context && SDL_GL_MakeCurrent(window, context); }
bool Window::swapBuffers() const { return window && SDL_GL_SwapWindow(window); }
void Window::close() {
    if (context) { SDL_GL_DestroyContext(context); context = nullptr; }
    if (window) { SDL_DestroyWindow(window); window = nullptr; }
    windowedWidth = 0;
    windowedHeight = 0;
    vsyncEnabled = false;
}
bool Window::setTitle(const std::string& title) { return window && SDL_SetWindowTitle(window, title.c_str()); }
SDL_WindowID Window::getId() const { return window ? SDL_GetWindowID(window) : 0; }
bool Window::getSize(int& width, int& height) const {
    return window && SDL_GetWindowSize(window, &width, &height);
}
bool Window::isFullscreen() const {
    return window && (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;
}
bool Window::getPixelSize(int& width, int& height) const {
    return window && SDL_GetWindowSizeInPixels(window, &width, &height);
}

bool Window::setWindowedSize(int width, int height) {
    error.clear();
    if (!window || width <= 0 || height <= 0 || isFullscreen()) {
        error = "Dimensions de fenetre invalides ou fenetre en plein ecran.";
        return false;
    }
    if (!SDL_SetWindowSize(window, width, height) || !SDL_SyncWindow(window)) {
        error = SDL_GetError();
        return false;
    }
    int actualWidth = 0, actualHeight = 0;
    if (!getSize(actualWidth, actualHeight) || actualWidth != width || actualHeight != height) {
        error = "Le gestionnaire de fenetres n'a pas applique la resolution demandee.";
        return false;
    }
    windowedWidth = width;
    windowedHeight = height;
    return true;
}

bool Window::setFullscreen(bool enabled) {
    error.clear();
    if (!window) { error = "La fenetre n'est pas initialisee."; return false; }
    if (isFullscreen() == enabled) return true;
    if (enabled) {
        if (!getSize(windowedWidth, windowedHeight)) { error = SDL_GetError(); return false; }
    }
    if (!SDL_SetWindowFullscreen(window, enabled) || !SDL_SyncWindow(window)) {
        error = SDL_GetError();
        return false;
    }
    if (isFullscreen() != enabled) {
        error = "Le gestionnaire de fenetres n'a pas applique le mode demande.";
        return false;
    }
    return true;
}

bool Window::setVSync(bool enabled) {
    error.clear();
    if (!context || !SDL_GL_SetSwapInterval(enabled ? 1 : 0)) {
        error = SDL_GetError();
        return false;
    }
    int actualInterval = 0;
    if (!SDL_GL_GetSwapInterval(&actualInterval) || ((actualInterval != 0) != enabled)) {
        error = SDL_GetError();
        if (error.empty()) error = "Le pilote n'a pas applique la valeur VSync demandee.";
        return false;
    }
    vsyncEnabled = actualInterval != 0;
    return true;
}

bool Window::applyDisplaySettings(int width, int height, bool fullscreen, bool vsync) {
    error.clear();
    if (!window || !context || width <= 0 || height <= 0) {
        error = "Fenetre ou parametres d'affichage invalides.";
        return false;
    }

    const bool oldFullscreen = isFullscreen();
    const bool oldVsync = vsyncEnabled;
    int oldWindowedWidth = windowedWidth;
    int oldWindowedHeight = windowedHeight;
    if (oldWindowedWidth <= 0 || oldWindowedHeight <= 0) {
        if (!getSize(oldWindowedWidth, oldWindowedHeight)) {
            error = SDL_GetError();
            return false;
        }
    }
    SDL_DisplayMode oldModeValue{};
    const SDL_DisplayMode* oldMode = oldFullscreen ? SDL_GetWindowFullscreenMode(window) : nullptr;
    const bool hadExclusiveMode = oldMode != nullptr;
    if (oldMode) oldModeValue = *oldMode;

    auto rollback = [&](const std::string& cause) {
        std::string rollbackError;
        auto noteFailure = [&](const char* operation) {
            if (rollbackError.empty()) rollbackError = operation;
        };

        if (isFullscreen() && (!SDL_SetWindowFullscreen(window, false) || !SDL_SyncWindow(window)))
            noteFailure("impossible de quitter le plein ecran");
        if (isFullscreen()) {
            error = cause + "; restauration impossible : " + rollbackError;
            return false;
        }
        if (!SDL_SetWindowFullscreenMode(window, nullptr)) noteFailure("impossible de restaurer le mode fenetre");
        if (!SDL_SetWindowSize(window, oldWindowedWidth, oldWindowedHeight) || !SDL_SyncWindow(window))
            noteFailure("impossible de restaurer la resolution fenetree");
        windowedWidth = oldWindowedWidth;
        windowedHeight = oldWindowedHeight;

        if (oldFullscreen) {
            const SDL_DisplayMode* restoreMode = nullptr;
            SDL_DisplayMode** availableModes = nullptr;
            int modeCount = 0;
            if (hadExclusiveMode) {
                const SDL_DisplayID display = SDL_GetDisplayForWindow(window);
                availableModes = SDL_GetFullscreenDisplayModes(display, &modeCount);
                restoreMode = findMode(availableModes, modeCount, oldModeValue);
                if (!restoreMode) noteFailure("ancien mode plein ecran introuvable");
            }
            if ((!hadExclusiveMode || restoreMode) &&
                (!SDL_SetWindowFullscreenMode(window, restoreMode) ||
                 !SDL_SetWindowFullscreen(window, true) || !SDL_SyncWindow(window)))
                noteFailure("impossible de restaurer le plein ecran");
            if (availableModes) SDL_free(availableModes);
        }

        if (!SDL_GL_SetSwapInterval(oldVsync ? 1 : 0)) noteFailure("impossible de restaurer VSync");
        else vsyncEnabled = oldVsync;
        error = cause;
        if (!rollbackError.empty()) error += "; restauration incomplete : " + rollbackError;
        return false;
    };

    if (oldFullscreen && !setFullscreen(false)) return rollback(error);
    if (!SDL_SetWindowFullscreenMode(window, nullptr)) return rollback(SDL_GetError());
    if (!setWindowedSize(width, height)) return rollback(error);

    SDL_DisplayMode** availableModes = nullptr;
    if (fullscreen) {
        int modeCount = 0;
        const SDL_DisplayID display = SDL_GetDisplayForWindow(window);
        if (!display) return rollback(SDL_GetError());
        availableModes = SDL_GetFullscreenDisplayModes(display, &modeCount);
        const SDL_DisplayMode* selectedMode = findResolution(availableModes, modeCount, width, height);
        if (!selectedMode) {
            if (availableModes) SDL_free(availableModes);
            return rollback("La resolution choisie n'est pas disponible en plein ecran.");
        }
        if (!SDL_SetWindowFullscreenMode(window, selectedMode) || !setFullscreen(true)) {
            const std::string cause = !error.empty() ? error : SDL_GetError();
            if (availableModes) SDL_free(availableModes);
            return rollback(cause);
        }
        int actualWidth = 0, actualHeight = 0;
        if (!getSize(actualWidth, actualHeight) || actualWidth != width || actualHeight != height) {
            if (availableModes) SDL_free(availableModes);
            return rollback("Le mode plein ecran n'a pas la resolution demandee.");
        }
        if (availableModes) SDL_free(availableModes);
    }
    if (!setVSync(vsync)) return rollback(error);

    error.clear();
    return true;
}
}
