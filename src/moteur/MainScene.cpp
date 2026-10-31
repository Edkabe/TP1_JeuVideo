#include "moteur/MainScene.h"
#include "moteur/Engine.h"
#include "moteur/Localization.h"
#include <SDL3/SDL.h>
#include <algorithm>

namespace moteur {
MainScene::MainScene(SceneManager& manager,std::filesystem::path font,std::filesystem::path image,Targets destinations)
    : scenes(manager),fontFile(std::move(font)),backgroundFile(std::move(image)),targets(std::move(destinations)) {}
const Button* MainScene::getButton(Action action) const {
    const auto index = static_cast<size_t>(action);
    return index < buttons.size() ? buttons[index].get() : nullptr;
}
std::shared_ptr<Font> MainScene::acquireFont(float size) {
    auto& resources = Engine::getInstance().getResourceManager();
    const auto key = "shared:font:" + std::to_string(size) + ":" + fontFile.u8string();
    auto font = resources.get<Font>(key);
    if (!font && resources.contains(key)) { error = "Nom de police utilise par une autre ressource."; return {}; }
    if (font && font->isLoaded()) return font;
    if (!font) font = std::make_shared<Font>(size,key);
    if (!font->load(fontFile.u8string())) { error = font->getError(); return {}; }
    if (!resources.contains(key) && !resources.add(font)) { error = "Ajout de la police impossible."; return {}; }
    return font;
}
std::shared_ptr<Texture> MainScene::acquireBackground() {
    auto& resources = Engine::getInstance().getResourceManager();
    const auto key = "shared:background:" + backgroundFile.u8string();
    auto texture = resources.get<Texture>(key);
    if (!texture && resources.contains(key)) { error = "Nom de texture utilise par une autre ressource."; return {}; }
    if (texture && texture->isLoaded()) return texture;
    if (!texture) texture = std::make_shared<Texture>(key);
    if (!texture->load(backgroundFile.u8string())) { error = texture->getError(); return {}; }
    if (!resources.contains(key) && !resources.add(texture)) { error = "Ajout de l'image impossible."; return {}; }
    return texture;
}
bool MainScene::load() {
    if (loaded) return true;
    error.clear();
    if (!SDL_GL_GetCurrentContext()) { error = "Contexte OpenGL actif necessaire."; return false; }
    if (targets.start.empty() || targets.settings.empty() || targets.credits.empty() ||
        !scenes.hasScene(targets.start) || !scenes.hasScene(targets.settings) || !scenes.hasScene(targets.credits)) {
        error = "Une scene de destination du menu est absente.";
        return false;
    }
    const auto texture = acquireBackground();
    if (!texture) return false;
    const auto titleFont = acquireFont(36),bodyFont = acquireFont(22);
    if (!titleFont || !bodyFont) return false;
    sharedFont = bodyFont;
    sharedBackground = texture;
    const auto& language = Engine::getInstance().getSettings().getLanguage();
    background = std::make_unique<Image>(texture);
    title = std::make_unique<Label>(titleFont,localized(language,"TP1 - Moteur graphique","TP1 - Graphics engine"));
    status = std::make_unique<Label>(bodyFont);
    status->setColor({1,.78f,.4f});
    const char* captions[] = {
        localized(language,"Jeu","Play"), localized(language,"Paramètres","Settings"),
        localized(language,"Crédits","Credits"), localized(language,"Quitter","Quit")
    };
    for (size_t i = 0; i < buttons.size(); ++i) {
        buttons[i] = std::make_shared<Button>(bodyFont,captions[i]);
        buttons[i]->setBorderColor({.22f,.5f,.64f});
        buttons[i]->setBackgroundColor({.06f,.13f,.21f,.96f});
        buttons[i]->setHoverColor({.12f,.3f,.43f});
        for (Uint32 type : {SDL_EVENT_MOUSE_MOTION,SDL_EVENT_MOUSE_BUTTON_DOWN,SDL_EVENT_MOUSE_BUTTON_UP,
                           SDL_EVENT_WINDOW_FOCUS_LOST,SDL_EVENT_WINDOW_MOUSE_LEAVE}) {
            if (!getEventManager().addObserver(buttons[i],type)) {
                error = "Inscription du bouton impossible."; unload(); return false;
            }
        }
    }
    // Copier la destination avant la transition : unload() detruit les controles.
    buttons[0]->setOnClick([this] { navigate(targets.start); });
    buttons[1]->setOnClick([this] { navigate(targets.settings); });
    buttons[2]->setOnClick([this] { navigate(targets.credits); });
    buttons[3]->setOnClick([] { Engine::getInstance().stop(); });
    int width = 0,height = 0;
    if (!Engine::getInstance().getWindow().getPixelSize(width,height) || width <= 0 || height <= 0) {
        error = SDL_GetError(); if (error.empty()) error = "Dimensions de fenetre indisponibles.";
        unload(); return false;
    }
    layout(width,height); loaded = true;
    return true;
}
void MainScene::navigate(const std::string& destination) {
    const std::string name = destination;
    if (name.empty() || !scenes.hasScene(name)) {
        error = "Scene de destination non disponible : " + name;
        if (status) status->setText(localized(Engine::getInstance().getSettings().getLanguage(),
                                             "Cet écran n'est pas encore disponible.",
                                             "This screen is not available yet."));
        return;
    }
    if (!scenes.setActiveScene(name)) {
        error = scenes.getError();
        if (status) status->setText(localized(Engine::getInstance().getSettings().getLanguage(),
                                             "Impossible d'ouvrir cet écran.",
                                             "Could not open this screen."));
    }
}
void MainScene::layout(int width,int height) {
    if (!background || !title || !status) return;
    const float w = static_cast<float>(width),h = static_cast<float>(height);
    background->resize({w,h});
    title->setPosition({(w-title->getDimensions().x)*.5f,h*.12f});
    const Vector2 size{(std::min)(280.0f,(std::max)(0.0f,w-48)),std::clamp(h*.09f,36.0f,52.0f)};
    for (size_t i = 0; i < buttons.size(); ++i) {
        const Vector2 position{(w-size.x)*.5f,h*.32f+static_cast<float>(i)*(size.y+12)};
        if (buttons[i]->getDimensions() != size) buttons[i]->setDimensions(size);
        if (buttons[i]->getPosition() != position) buttons[i]->setPosition(position);
    }
    status->setPosition({(w-status->getDimensions().x)*.5f,(std::max)(0.0f,h-45)});
}
void MainScene::render() {
    if (!loaded) return;
    auto& engine = Engine::getInstance();
    int width = 0,height = 0;
    if (!engine.getWindow().getPixelSize(width,height)) { error = SDL_GetError(); engine.reportError(error); return; }
    if (width <= 0 || height <= 0) return;
    glViewport(0,0,width,height); glClearColor(0,0,0,1); glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE); glBlendEquation(GL_FUNC_ADD);
    layout(width,height);
    if (!background->render()) { error = background->getError(); engine.reportError(error); return; }
    if (!title->render()) { error = title->getError(); engine.reportError(error); return; }
    for (const auto& button : buttons) {
        if (!button->render()) { error = button->getError(); engine.reportError(error); return; }
    }
    if (!status->render()) { error = status->getError(); engine.reportError(error); }
}
void MainScene::unload() {
    for (auto& button : buttons) { getEventManager().removeObserver(button); button.reset(); }
    status.reset(); title.reset(); background.reset(); loaded = false;
    sharedFont.reset(); sharedBackground.reset();
}
}
