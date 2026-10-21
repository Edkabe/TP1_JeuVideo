#include "moteur/DemoGameScene.h"
#include "moteur/Engine.h"
#include <algorithm>
#include <cmath>

namespace moteur {
DemoGameScene::DemoGameScene(SceneManager& manager,std::filesystem::path file,std::string destination)
    : scenes(manager),fontFile(std::move(file)),returnScene(std::move(destination)) {}
bool DemoGameScene::load() {
    if (loaded) return true;
    error.clear();
    if (!SDL_GL_GetCurrentContext() || !scenes.hasScene(returnScene)) { error = "Contexte ou scene de retour indisponible."; return false; }
    auto& resources = Engine::getInstance().getResourceManager();
    const auto key = "demo:font:" + fontFile.u8string();
    auto font = resources.get<Font>(key);
    if (!font && resources.contains(key)) { error = "Nom de police utilise par une autre ressource."; return false; }
    if (!font) font = std::make_shared<Font>(24.0f,key);
    if (!font->isLoaded() && !font->load(fontFile.u8string())) { error = font->getError(); return false; }
    if (!resources.contains(key) && !resources.add(font)) { error = "Ajout de la police impossible."; return false; }
    title = std::make_unique<Label>(font,"Scène de démonstration");
    description = std::make_unique<Label>(font,"Le jeu sera branché ici.");
    progress = std::make_unique<ProgressBar>();
    back = std::make_shared<Button>(font,"Retour au menu");
    back->setOnClick([this] { if (!scenes.setActiveScene(returnScene)) error = scenes.getError(); });
    for (Uint32 type : {SDL_EVENT_MOUSE_MOTION,SDL_EVENT_MOUSE_BUTTON_DOWN,SDL_EVENT_MOUSE_BUTTON_UP,
                       SDL_EVENT_WINDOW_FOCUS_LOST,SDL_EVENT_WINDOW_MOUSE_LEAVE}) {
        if (!getEventManager().addObserver(back,type)) { error = "Inscription du bouton impossible."; unload(); return false; }
    }
    int width = 0,height = 0; Engine::getInstance().getWindow().getPixelSize(width,height);
    layout(width,height); loaded = true;
    return true;
}
void DemoGameScene::layout(int width,int height) {
    const float w = static_cast<float>(width),h = static_cast<float>(height);
    title->setPosition({(w-title->getDimensions().x)*.5f,h*.2f});
    description->setPosition({(w-description->getDimensions().x)*.5f,h*.35f});
    const Vector2 barSize{(std::min)(400.0f,(std::max)(0.0f,w-48)),28};
    progress->setDimensions(barSize); progress->setPosition({(w-barSize.x)*.5f,h*.5f});
    const Vector2 buttonSize{(std::min)(240.0f,(std::max)(0.0f,w-48)),48};
    const Vector2 buttonPosition{(w-buttonSize.x)*.5f,h*.72f};
    if (back->getDimensions() != buttonSize) back->setDimensions(buttonSize);
    if (back->getPosition() != buttonPosition) back->setPosition(buttonPosition);
}
void DemoGameScene::update(float deltaTime) {
    if (!loaded || !std::isfinite(deltaTime) || deltaTime < 0) return;
    if (progress->getValue() == progress->getMaximum()) progress->setValue(progress->getMinimum());
    progress->advance(deltaTime*20);
}
void DemoGameScene::render() {
    if (!loaded) return;
    auto& engine = Engine::getInstance();
    int width = 0,height = 0;
    if (!engine.getWindow().getPixelSize(width,height)) { error = SDL_GetError(); engine.stop(); return; }
    if (width <= 0 || height <= 0) return;
    glViewport(0,0,width,height); glClearColor(.035f,.12f,.14f,1); glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE); glBlendEquation(GL_FUNC_ADD);
    layout(width,height);
    if (!title->render()) error = title->getError();
    else if (!description->render()) error = description->getError();
    else if (!progress->render()) error = progress->getError();
    else if (!back->render()) error = back->getError();
    else return;
    engine.stop();
}
void DemoGameScene::unload() {
    getEventManager().removeObserver(back);
    back.reset(); progress.reset(); description.reset(); title.reset(); loaded = false;
}
}
