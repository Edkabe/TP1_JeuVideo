#pragma once
#include "moteur/IScene.h"
#include "moteur/SceneManager.h"
#include "moteur/Button.h"
#include "moteur/ProgressBar.h"
#include <filesystem>

namespace moteur {
// Scene de demonstration avec progression simulee du demarrage du jeu.
class DemoGameScene final : public IScene {
public:
    DemoGameScene(SceneManager& scenes, std::filesystem::path fontFile, std::string returnScene);
    bool load() override;
    void unload() override;
    void update(float deltaTime) override;
    void render() override;
    bool isLoaded() const { return loaded; }
    bool isLoadingComplete() const { return loadingComplete; }
    float getLoadingProgress() const { return progress ? progress->getProgress() : 0.0f; }
    const Button* getReturnButton() const { return back.get(); }
    const std::string& getError() const { return error; }
private:
    void layout(int width, int height);
    SceneManager& scenes;
    std::filesystem::path fontFile;
    std::string returnScene;
    std::unique_ptr<Label> title, instructions;
    std::unique_ptr<ProgressBar> progress;
    std::shared_ptr<Button> back;
    float loadingTime = 0.0f;
    bool loadingComplete = false;
    bool loaded = false;
    std::string error;
};
}
