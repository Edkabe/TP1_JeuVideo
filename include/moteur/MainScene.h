#pragma once
#include "moteur/IScene.h"
#include "moteur/SceneManager.h"
#include "moteur/Image.h"
#include "moteur/Button.h"
#include <array>
#include <filesystem>

namespace moteur {
class MainScene final : public IScene {
public:
    enum class Action : size_t { Start, Settings, Credits, Quit };
    // Noms fournis par l'application. Aucun nom de scene d'Oceane n'est impose.
    struct Targets { std::string start,settings,credits; };
    MainScene(SceneManager& scenes,std::filesystem::path fontFile,
              std::filesystem::path backgroundFile,Targets targets);
    bool load() override;
    void unload() override;
    void render() override;
    bool isLoaded() const { return loaded; }
    const std::string& getError() const { return error; }
    // Pointeur emprunte, invalide apres unload().
    const Button* getButton(Action action) const;
private:
    std::shared_ptr<Font> acquireFont(float size);
    std::shared_ptr<Texture> acquireBackground();
    void layout(int width,int height);
    void navigate(const std::string& destination);
    SceneManager& scenes;
    std::filesystem::path fontFile,backgroundFile;
    Targets targets;
    std::unique_ptr<Image> background;
    std::unique_ptr<Label> title,status;
    std::array<std::shared_ptr<Button>,4> buttons;
    bool loaded = false;
    std::string error;
};
}
