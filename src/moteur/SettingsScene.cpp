#include "moteur/SettingsScene.h"
#include "moteur/Engine.h"
#include "moteur/Localization.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace moteur {
namespace {
std::string formatValue(float value, int precision) {
    std::ostringstream text;
    text << std::fixed << std::setprecision(precision) << value;
    return text.str();
}

const Uint32 controlEvents[] = {
    SDL_EVENT_MOUSE_MOTION,
    SDL_EVENT_MOUSE_BUTTON_DOWN,
    SDL_EVENT_MOUSE_BUTTON_UP,
    SDL_EVENT_WINDOW_FOCUS_LOST,
    SDL_EVENT_WINDOW_MOUSE_LEAVE
};
}

SettingsScene::SettingsScene(SceneManager& manager, std::filesystem::path font,
                             std::filesystem::path image, std::string destination)
    : scenes(manager), fontFile(std::move(font)), backgroundFile(std::move(image)),
      returnScene(std::move(destination)) {}

std::shared_ptr<Font> SettingsScene::acquireFont() {
    auto& resources = Engine::getInstance().getResourceManager();
    const auto key = "shared:font:" + std::to_string(22.0f) + ":" + fontFile.u8string();
    auto font = resources.get<Font>(key);
    if (!font && resources.contains(key)) {
        error = "Nom de police utilise par une autre ressource.";
        return {};
    }
    if (!font) font = std::make_shared<Font>(22.0f, key);
    if (!font->isLoaded() && !font->load(fontFile.u8string())) {
        error = font->getError();
        return {};
    }
    if (!resources.contains(key) && !resources.add(font)) {
        error = "Ajout de la police impossible.";
        return {};
    }
    return font;
}

std::shared_ptr<Texture> SettingsScene::acquireBackground() {
    auto& resources = Engine::getInstance().getResourceManager();
    const auto key = "shared:background:" + backgroundFile.u8string();
    auto texture = resources.get<Texture>(key);
    if (!texture && resources.contains(key)) {
        error = "Nom de texture utilise par une autre ressource.";
        return {};
    }
    if (!texture) texture = std::make_shared<Texture>(key);
    if (!texture->isLoaded() && !texture->load(backgroundFile.u8string())) {
        error = texture->getError();
        return {};
    }
    if (!resources.contains(key) && !resources.add(texture)) {
        error = "Ajout de l'image impossible.";
        return {};
    }
    return texture;
}

bool SettingsScene::load() {
    if (loaded) return true;
    error.clear();
    if (!SDL_GL_GetCurrentContext() || !scenes.hasScene(returnScene)) {
        error = "Contexte OpenGL ou scene de retour indisponible.";
        return false;
    }

    auto& settings = Engine::getInstance().getSettings();
    const auto& languageCode = settings.getLanguage();
    const auto font = acquireFont();
    const auto texture = acquireBackground();
    if (!font || !texture) { unload(); return false; }
    sharedFont = font;
    sharedBackground = texture;

    background = std::make_unique<Image>(texture);
    title = std::make_unique<Label>(font, localized(languageCode, "Paramètres", "Settings"));
    status = std::make_unique<Label>(font);
    status->setColor({1.0f, 0.78f, 0.4f});
    rowLabels[0] = std::make_unique<Label>(font, localized(languageCode, "Langue", "Language"));
    rowLabels[1] = std::make_unique<Label>(font, localized(languageCode, "Résolution", "Resolution"));
    rowLabels[2] = std::make_unique<Label>(font, localized(languageCode, "Plein écran", "Fullscreen"));
    rowLabels[3] = std::make_unique<Label>(font, localized(languageCode, "VSync", "VSync"));
    rowLabels[4] = std::make_unique<Label>(font);
    rowLabels[5] = std::make_unique<Label>(font);

    languageCodes = {"fr", "en"};
    std::vector<std::string> languageNames{"Français", "English"};
    const auto currentLanguage = settings.getLanguage();
    auto selectedLanguage = std::find(languageCodes.begin(), languageCodes.end(), currentLanguage);
    if (selectedLanguage == languageCodes.end()) {
        languageCodes.push_back(currentLanguage);
        languageNames.push_back("Autre (" + currentLanguage + ")");
        selectedLanguage = languageCodes.end() - 1;
    }
    language = std::make_shared<Selector>(font, std::move(languageNames));
    language->setSelectedIndex(static_cast<size_t>(selectedLanguage - languageCodes.begin()));

    resolutions = {{960, 540}, {1280, 720}, {1920, 1080}};
    const auto currentResolution = std::make_pair(settings.getWidth(), settings.getHeight());
    if (std::find(resolutions.begin(), resolutions.end(), currentResolution) == resolutions.end())
        resolutions.push_back(currentResolution);
    std::vector<std::string> resolutionNames;
    resolutionNames.reserve(resolutions.size());
    size_t selectedResolution = 0;
    for (size_t i = 0; i < resolutions.size(); ++i) {
        const auto [width, height] = resolutions[i];
        resolutionNames.push_back(std::to_string(width) + " x " + std::to_string(height));
        if (resolutions[i] == currentResolution) selectedResolution = i;
    }
    resolution = std::make_shared<Selector>(font, std::move(resolutionNames));
    resolution->setSelectedIndex(selectedResolution);

    fullscreen = std::make_shared<CheckBox>();
    vsync = std::make_shared<CheckBox>();
    fullscreen->setChecked(settings.isFullscreen());
    vsync->setChecked(settings.isVSyncEnabled());
    if (!fullscreen->initialize() || !vsync->initialize()) {
        error = !fullscreen->getError().empty() ? fullscreen->getError() : vsync->getError();
        unload();
        return false;
    }

    sensitivity = std::make_shared<Slider>();
    fov = std::make_shared<Slider>();
    if (!sensitivity->setRange(0.01f, 100.0f, 0.01f) || !sensitivity->setValue(settings.getSensitivity()) ||
        !fov->setRange(30.0f, 120.0f, 1.0f) || !fov->setValue(settings.getFov())) {
        error = !sensitivity->getError().empty() ? sensitivity->getError() : fov->getError();
        unload();
        return false;
    }
    for (const auto& slider : {sensitivity, fov}) {
        slider->setBorderColor({0.08f, 0.10f, 0.14f, 1.0f});
        slider->setBackgroundColor({0.82f, 0.85f, 0.90f, 1.0f});
        slider->setProgressColor({0.10f, 0.48f, 0.78f, 1.0f});
        slider->setHoverColor({0.20f, 0.72f, 0.96f, 1.0f});
    }

    applyButton = std::make_shared<Button>(font, localized(languageCode, "Appliquer", "Apply"));
    backButton = std::make_shared<Button>(font, localized(languageCode, "Retour", "Back"));
    for (const auto& button : {applyButton, backButton}) {
        button->setBorderColor({0.22f, 0.5f, 0.64f});
        button->setBackgroundColor({0.06f, 0.13f, 0.21f, 0.96f});
        button->setHoverColor({0.12f, 0.3f, 0.43f});
    }
    applyButton->setOnClick([this] { applyChanges(); });
    backButton->setOnClick([this] { returnToMenu(); });

    for (Uint32 type : controlEvents) {
        if (!getEventManager().addObserver(language, type) ||
            !getEventManager().addObserver(resolution, type) ||
            !getEventManager().addObserver(fullscreen, type) ||
            !getEventManager().addObserver(vsync, type) ||
            !getEventManager().addObserver(sensitivity, type) ||
            !getEventManager().addObserver(fov, type) ||
            !getEventManager().addObserver(applyButton, type) ||
            !getEventManager().addObserver(backButton, type)) {
            error = "Inscription d'un controle aux evenements impossible.";
            unload();
            return false;
        }
    }

    if (!refreshValueLabels()) { unload(); return false; }
    int width = 0, height = 0;
    if (!Engine::getInstance().getWindow().getPixelSize(width, height) || width <= 0 || height <= 0) {
        error = SDL_GetError(); if (error.empty()) error = "Dimensions de fenetre indisponibles.";
        unload(); return false;
    }
    layout(width, height);
    loaded = true;
    return true;
}

void SettingsScene::layout(int width, int height) {
    if (!title || !background) return;
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    background->resize({w, h});
    title->setPosition({(w - title->getDimensions().x) * 0.5f, h * 0.045f});

    const float controlWidth = (std::min)(420.0f, (std::max)(0.0f, w - 80.0f));
    const float left = (w - controlWidth) * 0.5f;
    const Vector2 selectorSize{controlWidth, 42.0f};
    language->setDimensions(selectorSize);
    language->setPosition({left, h * 0.17f});
    resolution->setDimensions(selectorSize);
    resolution->setPosition({left, h * 0.30f});
    rowLabels[0]->setPosition({left, h * 0.13f});
    rowLabels[1]->setPosition({left, h * 0.26f});

    const float checkSize = 24.0f;
    const float checkX = (w - 260.0f) * 0.5f;
    fullscreen->setSize(checkSize);
    fullscreen->setPosition(checkX, h * 0.41f);
    vsync->setSize(checkSize);
    vsync->setPosition(checkX, h * 0.49f);
    rowLabels[2]->setPosition({checkX + checkSize + 12.0f, h * 0.41f});
    rowLabels[3]->setPosition({checkX + checkSize + 12.0f, h * 0.49f});

    const Vector2 sliderSize{controlWidth, 26.0f};
    sensitivity->setDimensions(sliderSize);
    sensitivity->setPosition({left, h * 0.61f});
    fov->setDimensions(sliderSize);
    fov->setPosition({left, h * 0.74f});
    rowLabels[4]->setPosition({left, h * 0.57f});
    rowLabels[5]->setPosition({left, h * 0.70f});

    const float buttonWidth = (std::min)(190.0f, (std::max)(0.0f, (w - 52.0f) * 0.5f));
    const float buttonHeight = 44.0f;
    const float gap = 16.0f;
    const float pairWidth = buttonWidth * 2.0f + gap;
    const float buttonY = h * 0.86f;
    applyButton->setDimensions({buttonWidth, buttonHeight});
    applyButton->setPosition({(w - pairWidth) * 0.5f, buttonY});
    backButton->setDimensions({buttonWidth, buttonHeight});
    backButton->setPosition({(w - pairWidth) * 0.5f + buttonWidth + gap, buttonY});
    status->setPosition({(w - status->getDimensions().x) * 0.5f, h * 0.95f});
}

bool SettingsScene::refreshValueLabels() {
    if (!rowLabels[4] || !rowLabels[5]) return false;
    const auto& languageCode = Engine::getInstance().getSettings().getLanguage();
    const auto sensitivityText = std::string(localized(languageCode, "Sensibilité : ", "Sensitivity: ")) +
        formatValue(sensitivity->getValue(), 2);
    const auto fovText = std::string(localized(languageCode, "Champ de vision : ", "Field of view: ")) +
        formatValue(fov->getValue(), 0) + "°";
    if (rowLabels[4]->getText() != sensitivityText && !rowLabels[4]->setText(sensitivityText)) {
        error = rowLabels[4]->getError(); return false;
    }
    if (rowLabels[5]->getText() != fovText && !rowLabels[5]->setText(fovText)) {
        error = rowLabels[5]->getError(); return false;
    }
    return true;
}

void SettingsScene::update(float deltaTime) {
    if (!loaded || !std::isfinite(deltaTime) || deltaTime < 0.0f) return;
    if (!refreshValueLabels()) return;
}

void SettingsScene::applyChanges() {
    auto& settings = Engine::getInstance().getSettings();
    const size_t languageIndex = language->getSelectedIndex();
    const size_t resolutionIndex = resolution->getSelectedIndex();
    const bool validSelection = languageIndex < languageCodes.size() && resolutionIndex < resolutions.size();
    const auto selectedResolution = validSelection ? resolutions[resolutionIndex] : std::pair<int, int>{0, 0};
    if (!validSelection) {
        error = "Une valeur de parametre selectionnee est invalide.";
        status->setText(localized(settings.getLanguage(),
                                  "Une valeur de paramètre est invalide.",
                                  "A selected setting is invalid."));
        return;
    }
    auto next = settings.getValues();
    next.language = languageCodes[languageIndex];
    next.width = selectedResolution.first;
    next.height = selectedResolution.second;
    next.fullscreen = fullscreen->isChecked();
    next.vsync = vsync->isChecked();
    next.sensitivity = sensitivity->getValue();
    next.fov = fov->getValue();
    auto& engine = Engine::getInstance();
    if (!engine.commitSettings(next)) {
        error = engine.getError();
        status->setText(std::string(localized(settings.getLanguage(),
                                              "Application ou enregistrement impossible : ",
                                              "Could not apply or save settings: ")) + error);
        return;
    }
    error.clear();
    if (!refreshLanguageTexts() || !refreshValueLabels()) return;
    status->setText(localized(settings.getLanguage(),
                              "Paramètres enregistrés et appliqués.",
                              "Settings applied and saved."));
}

bool SettingsScene::refreshLanguageTexts() {
    if (!title) return false;
    const auto& languageCode = Engine::getInstance().getSettings().getLanguage();
    if (!title->setText(localized(languageCode, "Paramètres", "Settings")) ||
        !rowLabels[0]->setText(localized(languageCode, "Langue", "Language")) ||
        !rowLabels[1]->setText(localized(languageCode, "Résolution", "Resolution")) ||
        !rowLabels[2]->setText(localized(languageCode, "Plein écran", "Fullscreen")) ||
        !rowLabels[3]->setText(localized(languageCode, "VSync", "VSync")) ||
        !applyButton->setTitle(localized(languageCode, "Appliquer", "Apply")) ||
        !backButton->setTitle(localized(languageCode, "Retour", "Back"))) {
        error = "Mise a jour des textes de l'ecran Parametres impossible.";
        return false;
    }
    return true;
}

void SettingsScene::returnToMenu() {
    if (!scenes.setActiveScene(returnScene)) {
        error = scenes.getError();
        if (status) status->setText(localized(Engine::getInstance().getSettings().getLanguage(),
                                             "Impossible de revenir au menu.",
                                             "Could not return to the menu."));
    }
}

void SettingsScene::render() {
    if (!loaded) return;
    auto& engine = Engine::getInstance();
    int width = 0, height = 0;
    if (!engine.getWindow().getPixelSize(width, height)) {
        error = SDL_GetError();
        engine.reportError(error);
        return;
    }
    if (width <= 0 || height <= 0) return;
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBlendEquation(GL_FUNC_ADD);
    layout(width, height);

    if (!background->render()) error = background->getError();
    else if (!title->render()) error = title->getError();
    else if (!rowLabels[0]->render() || !rowLabels[1]->render() ||
             !rowLabels[2]->render() || !rowLabels[3]->render() ||
             !rowLabels[4]->render() || !rowLabels[5]->render()) error = "Affichage des libelles impossible.";
    else if (!language->render()) error = language->getError();
    else if (!resolution->render()) error = resolution->getError();
    else if (!sensitivity->render()) error = sensitivity->getError();
    else if (!fov->render()) error = fov->getError();
    else if (!applyButton->render()) error = applyButton->getError();
    else if (!backButton->render()) error = backButton->getError();
    else {
        if (!fullscreen->render()) error = fullscreen->getError();
        else if (!vsync->render()) error = vsync->getError();
        else if (!status->render()) error = status->getError();
        else return;
    }
    engine.reportError(error);
}

void SettingsScene::unload() {
    getEventManager().removeObserver(language);
    getEventManager().removeObserver(resolution);
    getEventManager().removeObserver(fullscreen);
    getEventManager().removeObserver(vsync);
    getEventManager().removeObserver(sensitivity);
    getEventManager().removeObserver(fov);
    getEventManager().removeObserver(applyButton);
    getEventManager().removeObserver(backButton);
    if (applyButton) applyButton->release();
    if (backButton) backButton->release();
    if (language) language->release();
    if (resolution) resolution->release();
    if (sensitivity) sensitivity->release();
    if (fov) fov->release();
    if (background) background->release();
    for (auto& label : rowLabels) if (label) label->release();
    if (status) status->release();
    if (title) title->release();
    applyButton.reset(); backButton.reset(); language.reset(); resolution.reset();
    if (fullscreen) fullscreen->release();
    if (vsync) vsync->release();
    fullscreen.reset(); vsync.reset(); sensitivity.reset(); fov.reset();
    background.reset(); status.reset(); title.reset();
    sharedFont.reset(); sharedBackground.reset();
    for (auto& label : rowLabels) label.reset();
    languageCodes.clear();
    resolutions.clear();
    loaded = false;
}

} // namespace moteur
