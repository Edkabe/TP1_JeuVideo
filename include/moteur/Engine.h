#pragma once
#include "moteur/Window.h"
#include "moteur/ResourceManager.h"
#include <functional>

namespace moteur {
class Scene;

class Engine {
public:
    static Engine& getInstance();
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    bool initialize(const std::string& title = "TP1", int width = 960, int height = 540, bool hidden = false);
    // Les callbacks seront remplacés/branchés sur le gestionnaire de scènes.
    bool run(const std::function<void(float)>& update,
             const std::function<void()>& render,
             const std::function<void(const SDL_Event&)>& onEvent = {});
    // Cette surcharge distribue automatiquement les événements à une scène.
    bool run(Scene& scene,
             const std::function<void(float)>& update,
             const std::function<void()>& render);
    void stop() { running = false; }
    bool shutdown();
    bool isRunning() const { return running; }
    Window& getWindow() { return window; }
    ResourceManager& getResourceManager() { return resources; }
    const std::string& getError() const { return error; }
private:
    Engine() = default;
    ~Engine();
    Window window;
    ResourceManager resources;
    bool initialized = false;
    bool running = false;
    std::string error;
};
}
