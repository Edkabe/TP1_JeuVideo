#include "moteur/CreditsScene.h"
#include "moteur/Engine.h"
#include "moteur/Localization.h"
#include <SDL3/SDL.h>
#include <algorithm>

namespace moteur {
CreditsScene::CreditsScene(SceneManager& manager,std::filesystem::path file,std::string destination)
    : scenes(manager),fontFile(std::move(file)),returnScene(std::move(destination)) {}

std::shared_ptr<Font> CreditsScene::acquireFont(float size) {
    auto& resources = Engine::getInstance().getResourceManager();
    const std::string key = "credits:font:" + std::to_string(size) + ":" + fontFile.u8string();
    auto font = resources.get<Font>(key);
    if (!font && resources.contains(key)) { error = "Le nom de la police est utilise par une autre ressource."; return {}; }
    if (font && font->isLoaded()) return font;
    if (!font) font = std::make_shared<Font>(size,key);
    if (!font->load(fontFile.u8string())) { error = font->getError(); return {}; }
    if (!resources.contains(key) && !resources.add(font)) { error = "Ajout de la police impossible."; return {}; }
    return font;
}
bool CreditsScene::load() {
    if (loaded) return true;
    error.clear();
    if (!SDL_GL_GetCurrentContext()) { error = "Contexte OpenGL actif necessaire."; return false; }
    if (returnScene.empty() || !scenes.hasScene(returnScene)) { error = "Scene de retour absente : " + returnScene; return false; }
    const auto titleFont = acquireFont(32), bodyFont = acquireFont(20);
    if (!titleFont || !bodyFont) return false;
    const auto& language = Engine::getInstance().getSettings().getLanguage();
    labels.push_back(std::make_unique<Label>(titleFont, localized(language, "Crédits", "Credits")));
    for (const char* text : {
            localized(language, "TP1 - Moteur graphique", "TP1 - Graphics engine"),
            localized(language, "Projet réalisé par", "Project by"),
            "Edkabe et oceane-talla",
            "SDL3 | OpenGL | GLAD | GLM", "SDL_image | SDL_ttf"})
        labels.push_back(std::make_unique<Label>(bodyFont,text));
    labels[2]->setColor({0.65f,0.78f,0.95f});
    back = std::make_shared<Button>(bodyFont,localized(language,"Retour au menu","Back to menu"));
    back->setOnClick([this] {
        if (!scenes.setActiveScene(returnScene)) error = scenes.getError();
    });
    for (Uint32 type : {SDL_EVENT_MOUSE_MOTION,SDL_EVENT_MOUSE_BUTTON_DOWN,SDL_EVENT_MOUSE_BUTTON_UP,
                       SDL_EVENT_WINDOW_FOCUS_LOST,SDL_EVENT_WINDOW_MOUSE_LEAVE}) {
        if (!getEventManager().addObserver(back,type)) {
            error = "Inscription du bouton Retour impossible."; unload(); return false;
        }
    }
    int width = 0,height = 0;
    Engine::getInstance().getWindow().getPixelSize(width,height);
    layout(width,height);
    loaded = true;
    return true;
}
void CreditsScene::layout(int width,int height) {
    if (!back || labels.size() != 6) return;
    const float w = static_cast<float>(width),h = static_cast<float>(height);
    constexpr float rows[] = {.10f,.25f,.37f,.45f,.60f,.67f};
    for (size_t i = 0; i < labels.size(); ++i) {
        const auto size = labels[i]->getDimensions();
        labels[i]->setPosition({(w-size.x)*.5f,h*rows[i]});
    }
    const Vector2 size{(std::min)(220.0f,(std::max)(0.0f,w-40)),48};
    const Vector2 position{(w-size.x)*.5f,(std::max)(0.0f,h-72)};
    // Les setters de Button annulent un appui : ne les rappeler qu'au redimensionnement.
    if (back->getDimensions() != size) back->setDimensions(size);
    if (back->getPosition() != position) back->setPosition(position);
}
void CreditsScene::render() {
    if (!loaded) return;
    int width = 0,height = 0;
    auto& engine = Engine::getInstance();
    if (!engine.getWindow().getPixelSize(width,height)) { error = SDL_GetError(); engine.stop(); return; }
    if (width <= 0 || height <= 0) return;
    glViewport(0,0,width,height); glClearColor(.035f,.055f,.09f,1); glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE); glBlendEquation(GL_FUNC_ADD);
    layout(width,height);
    for (const auto& label : labels) {
        if (!label->render()) { error = label->getError(); engine.stop(); return; }
    }
    if (!back->render()) { error = back->getError(); engine.stop(); }
}
void CreditsScene::unload() {
    getEventManager().removeObserver(back);
    back.reset(); labels.clear(); loaded = false;
    // Les polices restent reutilisables dans le gestionnaire global de ressources.
}
}
