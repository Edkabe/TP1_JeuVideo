#pragma once

#include "moteur/IScene.h"
#include "moteur/SceneManager.h"
#include "moteur/Image.h"
#include "moteur/Label.h"
#include "moteur/Selector.h"
#include "moteur/CheckBox.h"
#include "moteur/Slider.h"
#include "moteur/Button.h"
#include <array>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace moteur {

/// @brief Écran qui prépare et sauvegarde les préférences de l'application.
class SettingsScene final : public IScene {
public:
    SettingsScene(SceneManager& scenes, std::filesystem::path fontFile,
                  std::filesystem::path backgroundFile, std::string returnScene);

    bool load() override;
    void unload() override;
    void update(float deltaTime) override;
    void render() override;

    bool isLoaded() const { return loaded; }
    const std::string& getError() const override { return error; }
    const std::shared_ptr<Font>& getSharedFont() const { return sharedFont; }
    const std::shared_ptr<Texture>& getSharedBackground() const { return sharedBackground; }

private:
    std::shared_ptr<Font> acquireFont();
    std::shared_ptr<Texture> acquireBackground();
    void layout(int width, int height);
    bool refreshLanguageTexts();
    bool refreshValueLabels();
    void applyChanges();
    void returnToMenu();

    SceneManager& scenes;
    std::filesystem::path fontFile;
    std::filesystem::path backgroundFile;
    std::string returnScene;
    std::vector<std::string> languageCodes;
    std::vector<std::pair<int, int>> resolutions;

    std::unique_ptr<Image> background;
    std::shared_ptr<Font> sharedFont;
    std::shared_ptr<Texture> sharedBackground;
    std::unique_ptr<Label> title;
    std::unique_ptr<Label> status;
    std::array<std::unique_ptr<Label>, 6> rowLabels;
    std::shared_ptr<Selector> language;
    std::shared_ptr<Selector> resolution;
    std::shared_ptr<CheckBox> fullscreen;
    std::shared_ptr<CheckBox> vsync;
    std::shared_ptr<Slider> sensitivity;
    std::shared_ptr<Slider> fov;
    std::shared_ptr<Button> applyButton;
    std::shared_ptr<Button> backButton;
    bool loaded = false;
    std::string error;
};

} // namespace moteur
