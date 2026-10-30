#include "moteur/DemoGameScene.h"
#include "moteur/Engine.h"
#include "moteur/Localization.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <utility>

namespace moteur {
namespace {
constexpr float loadingDuration = 3.0f;
const Uint32 buttonEvents[] = {
    SDL_EVENT_MOUSE_MOTION,
    SDL_EVENT_MOUSE_BUTTON_DOWN,
    SDL_EVENT_MOUSE_BUTTON_UP,
    SDL_EVENT_WINDOW_FOCUS_LOST,
    SDL_EVENT_WINDOW_MOUSE_LEAVE
};
}

DemoGameScene::DemoGameScene(SceneManager& manager, std::filesystem::path file, std::string destination)
    : scenes(manager), fontFile(std::move(file)), returnScene(std::move(destination)) {}

bool DemoGameScene::load() {
    if (loaded) return true;
    error.clear();
    if (!SDL_GL_GetCurrentContext() || !scenes.hasScene(returnScene)) {
        error = "Contexte ou scene de retour indisponible.";
        return false;
    }

    auto& engine = Engine::getInstance();
    auto& resources = engine.getResourceManager();
    const auto language = engine.getSettings().getLanguage();
    const auto key = "shared:font:24:" + fontFile.u8string();
    auto font = resources.get<Font>(key);
    if (!font && resources.contains(key)) { error = "Nom de police utilise par une autre ressource."; return false; }
    if (!font) font = std::make_shared<Font>(24.0f, key);
    if (!font->isLoaded() && !font->load(fontFile.u8string())) { error = font->getError(); return false; }
    if (!resources.contains(key) && !resources.add(font)) { error = "Ajout de la police impossible."; return false; }

    title = std::make_unique<Label>(font, localized(language, "Démarrage du jeu", "Starting the game"));
    instructions = std::make_unique<Label>(font,
        localized(language, "Le jeu est en train de démarrer…", "The game is starting…"));
    progress = std::make_unique<ProgressBar>();
    back = std::make_shared<Button>(font, localized(language, "Retour au menu", "Back to menu"));
    back->setBorderColor({0.12f, 0.62f, 0.68f, 1.0f});
    back->setBackgroundColor({0.06f, 0.22f, 0.28f, 1.0f});
    back->setHoverColor({0.10f, 0.42f, 0.50f, 1.0f});
    back->setTextColor({1.0f, 1.0f, 1.0f, 1.0f});
    back->setOnClick([this] {
        if (!scenes.setActiveScene(returnScene)) error = scenes.getError();
    });
    if (!progress->setRange(0.0f, loadingDuration) || !progress->setValue(0.0f)) {
        error = progress->getError();
        unload();
        return false;
    }
    progress->setBorderColor({0.12f, 0.62f, 0.68f, 1.0f});
    progress->setBackgroundColor({0.06f, 0.22f, 0.28f, 1.0f});
    progress->setProgressColor({0.10f, 0.72f, 0.58f, 1.0f});

    for (Uint32 type : buttonEvents) {
        if (!getEventManager().addObserver(back, type)) {
            error = "Inscription du bouton de retour aux evenements impossible.";
            unload();
            return false;
        }
    }

    int width = 0, height = 0;
    if (!engine.getWindow().getPixelSize(width, height) || width <= 0 || height <= 0) {
        error = SDL_GetError(); if (error.empty()) error = "Dimensions de fenetre indisponibles.";
        unload(); return false;
    }
    loaded = true;
    loadingTime = 0.0f;
    loadingComplete = false;
    layout(width, height);
    return true;
}

void DemoGameScene::layout(int width, int height) {
    if (!loaded || width <= 0 || height <= 0) return;
    const float w = static_cast<float>(width), h = static_cast<float>(height);
    const auto& language = Engine::getInstance().getSettings().getLanguage();
    title->setPosition({(w - title->getDimensions().x) * 0.5f, h * 0.24f});
    const auto statusText = localized(language,
        loadingComplete ? "Démarrage simulé terminé." : "Le jeu est en train de démarrer…",
        loadingComplete ? "Simulated startup complete." : "The game is starting…");
    if (instructions->getText() != statusText) instructions->setText(statusText);
    instructions->setPosition({(w - instructions->getDimensions().x) * 0.5f, h * 0.38f});
    const Vector2 progressSize{std::min(560.0f, std::max(120.0f, w - 80.0f)), 26.0f};
    progress->setDimensions(progressSize);
    progress->setPosition({(w - progressSize.x) * 0.5f, h * 0.52f});
    const Vector2 backSize{std::min(220.0f, w * 0.7f), 44.0f};
    back->setDimensions(backSize);
    back->setPosition({(w - backSize.x) * 0.5f, h * 0.78f});
}

void DemoGameScene::update(float deltaTime) {
    if (!loaded || loadingComplete || !std::isfinite(deltaTime) || deltaTime < 0.0f) return;
    loadingTime = std::min(loadingDuration, loadingTime + deltaTime);
    progress->setValue(loadingTime);
    if (loadingTime >= loadingDuration) loadingComplete = true;
}

void DemoGameScene::render() {
    if (!loaded) return;
    auto& engine = Engine::getInstance();
    int width = 0, height = 0;
    if (!engine.getWindow().getPixelSize(width, height)) { error = SDL_GetError(); engine.reportError(error); return; }
    if (width <= 0 || height <= 0) return;
    glViewport(0, 0, width, height);
    glClearColor(0.035f, 0.12f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBlendEquation(GL_FUNC_ADD);
    layout(width, height);
    if (!title->render()) error = title->getError();
    else if (!instructions->render()) error = instructions->getError();
    else if (!progress->render()) error = progress->getError();
    else if (!back->render()) error = back->getError();
    else return;
    engine.reportError(error);
}

void DemoGameScene::unload() {
    getEventManager().removeObserver(back);
    if (back) back->release();
    if (progress) progress->release();
    for (const auto& label : {title.get(), instructions.get()}) if (label) label->release();
    back.reset();
    progress.reset();
    title.reset();
    instructions.reset();
    loadingTime = 0.0f;
    loadingComplete = false;
    loaded = false;
}

} // namespace moteur
