#pragma once

#include "moteur/IScene.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace moteur {

class SceneManager {
public:
    SceneManager() = default;
    ~SceneManager();

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    bool addScene(std::string name, std::unique_ptr<IScene> scene);
    bool removeScene(const std::string& name);
    bool setActiveScene(const std::string& name);
    void clear();

    IScene* getActiveScene();
    const IScene* getActiveScene() const;
    const std::string& getActiveSceneName() const { return activeName; }
    const std::string& getError() const { return error; }
    const std::string& getActiveSceneError() const;
    bool hasScene(const std::string& name) const;

    // Ces fonctions ne font rien lorsqu'aucune scène n'est active.
    void dispatchEvent(const Event& event);
    void update(float deltaTime);
    void render();

private:
    using SceneMap = std::unordered_map<std::string, std::unique_ptr<IScene>>;
    SceneMap scenes;
    std::string activeName;
    std::string error;
};

}
