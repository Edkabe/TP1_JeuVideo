#pragma once
#include "moteur/IScene.h"
#include "moteur/SceneManager.h"
#include "moteur/Button.h"
#include <filesystem>
#include <vector>

namespace moteur {
class CreditsScene final : public IScene {
public:
    // Le nom de retour doit correspondre a une scene deja ajoutee au gestionnaire.
    CreditsScene(SceneManager& scenes, std::filesystem::path fontFile, std::string returnScene = "menu");
    bool load() override;
    void unload() override;
    void render() override;
    bool isLoaded() const { return loaded; }
    const std::string& getError() const { return error; }
private:
    std::shared_ptr<Font> acquireFont(float size);
    void layout(int width,int height);
    SceneManager& scenes;
    std::filesystem::path fontFile;
    std::string returnScene;
    std::vector<std::unique_ptr<Label>> labels;
    std::shared_ptr<Button> back;
    bool loaded = false;
    std::string error;
};
}
