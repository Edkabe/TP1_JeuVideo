#include "moteur/Engine.h"
#include "moteur/Scene.h"
#include <chrono>

namespace moteur {
Engine& Engine::getInstance() { static Engine instance; return instance; }
Engine::~Engine() { running = false; shutdown(); }
bool Engine::initialize(const std::string& title, int width, int height, bool hidden) {
    if (running || initialized) { error = "Moteur deja initialise ou actif."; return false; }
    error.clear();
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) { error = SDL_GetError(); return false; }
    if (!window.initialize(title, width, height, hidden)) {
        error = window.getError(); SDL_QuitSubSystem(SDL_INIT_VIDEO); return false;
    }
    initialized = true;
    return true;
}
bool Engine::run(const std::function<void(float)>& update, const std::function<void()>& render,
                 const std::function<void(const SDL_Event&)>& onEvent) {
    if (running) { error = "La boucle est deja active."; return false; }
    if (!update || !render) { error = "Callbacks de boucle manquants."; return false; }
    if (!initialized && !initialize()) return false;
    if (!window.makeCurrent()) { error = SDL_GetError(); return false; }
    error.clear();
    running = true;
    // Réinitialiser l'état même si un callback lance une exception.
    struct RunningGuard { bool& flag; ~RunningGuard() { flag = false; } } guard{running};
    auto previous = std::chrono::steady_clock::now();
    while (running) {
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - previous).count();
        previous = now;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == window.getId())) stop();
            if (onEvent) onEvent(event);
            if (!running) break;
        }
        if (!running) break;
        update(dt);
        if (!running) break;
        render();
        if (!running) break;
        if (!window.swapBuffers()) { error = SDL_GetError(); return false; }
        SDL_Delay(1);
    }
    return true;
}

bool Engine::run(Scene& scene, const std::function<void(float)>& update,
                 const std::function<void()>& render) {
    // L'Engine récupère les événements SDL; la scène active les distribue à ses observateurs.
    return run(update, render, [&scene](const SDL_Event& sdlEvent) {
        const Event event(sdlEvent);
        scene.dispatchEvent(event);
    });
}
bool Engine::shutdown() {
    if (running) { error = "Arreter la boucle avant de fermer le moteur."; return false; }
    // Les objets graphiques clients doivent être libérés AVANT cet appel.
    // Vider le gestionnaire tant que le contexte est encore actif.
    if (initialized && !window.makeCurrent()) { error = SDL_GetError(); return false; }
    resources.clear();
    if (!settings.save()) { error = settings.getError(); return false; }
    window.close();
    if (initialized) SDL_QuitSubSystem(SDL_INIT_VIDEO);
    initialized = false;
    return true;
}
}
