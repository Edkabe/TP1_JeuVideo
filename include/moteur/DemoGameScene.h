#pragma once
#include "moteur/IScene.h"
#include "moteur/SceneManager.h"
#include "moteur/Button.h"
#include "moteur/ProgressBar.h"
#include <filesystem>

namespace moteur {
// Destination provisoire demandee pour Demarrer ; ce n'est pas un jeu complet.
class DemoGameScene final : public IScene {
public:
    DemoGameScene(SceneManager& scenes,std::filesystem::path fontFile,std::string returnScene);
    bool load() override;
    void unload() override;
    void update(float deltaTime) override;
    void render() override;
    bool isLoaded() const { return loaded; }
    const Button* getReturnButton() const { return back.get(); }
    const std::string& getError() const { return error; }
private:
    void layout(int width,int height);
    SceneManager& scenes;
    std::filesystem::path fontFile;
    std::string returnScene;
    std::unique_ptr<Label> title,description;
    std::unique_ptr<ProgressBar> progress;
    std::shared_ptr<Button> back;
    bool loaded = false;
    std::string error;
};
}
