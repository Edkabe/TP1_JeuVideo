#pragma once
#include "moteur/Window.h"
#include "moteur/ResourceManager.h"
#include "moteur/Settings.h"
#include <functional>

namespace moteur {
class IScene;
using Scene = IScene;
class SceneManager;

class Engine {
public:
    static Engine& getInstance();
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    bool initialize(const std::string& title = "TP1", int width = 960, int height = 540, bool hidden = false);
    // Applique a la fenetre les parametres d'affichage actuellement charges ou modifies.
    bool applySettings();
    // Les callbacks seront remplacés/branchés sur le gestionnaire de scènes.
    bool run(const std::function<void(float)>& update,
             const std::function<void()>& render,
             const std::function<void(const SDL_Event&)>& onEvent = {});
    // Cette surcharge distribue automatiquement les événements à une scène.
    bool run(Scene& scene,
             const std::function<void(float)>& update,
             const std::function<void()>& render);
    // Cette surcharge branche automatiquement la boucle sur la scène active.
    bool run(SceneManager& scenes);
    void stop() { running = false; }
    void reportError(const std::string& message) {
        error = message.empty() ? "Erreur du moteur sans diagnostic detaille." : message;
        running = false;
    }
    bool shutdown();
    bool isRunning() const { return running; }
    Window& getWindow() { return window; }
    ResourceManager& getResourceManager() { return resources; }
    Settings& getSettings() { return settings; }
    const Settings& getSettings() const { return settings; }
    const std::string& getError() const { return error; }
private:
    Engine() = default;
    ~Engine();
    Window window;
    ResourceManager resources;
    // Chargés à la première création du moteur et accessibles aux scènes.
    Settings settings;
    bool initialized = false;
    bool running = false;
    std::string error;
};
}
