#include "moteur/Button.h"
#include "moteur/Engine.h"
#include "moteur/SceneManager.h"
#include <filesystem>
#include <iostream>

namespace {
class DemoScene final : public moteur::IScene {
public:
    DemoScene(moteur::SceneManager& manager, std::shared_ptr<moteur::Font> font,
              std::string name, std::string destination, moteur::Color color, bool& valid)
        : manager(manager), font(std::move(font)), name(std::move(name)),
          destination(std::move(destination)), color(color), valid(valid) {}
    bool load() override {
        title = std::make_unique<moteur::Label>(font,"Scene " + name);
        title->setPosition({30,25});
        button = std::make_shared<moteur::Button>(font,"Aller a " + destination);
        button->setPosition({30,80}); button->setDimensions({260,50});
        button->setOnClick([this] {
            if (!manager.setActiveScene(destination)) {
                std::cerr << manager.getError() << '\n';
                valid = false; moteur::Engine::getInstance().stop();
            }
        });
        for (Uint32 type : {SDL_EVENT_MOUSE_MOTION,SDL_EVENT_MOUSE_BUTTON_DOWN,SDL_EVENT_MOUSE_BUTTON_UP,
                           SDL_EVENT_WINDOW_FOCUS_LOST,SDL_EVENT_WINDOW_MOUSE_LEAVE})
            if (!getEventManager().addObserver(button,type)) return false;
        return true;
    }
    void unload() override {
        getEventManager().removeObserver(button);
        button.reset(); title.reset();
    }
    void render() override {
        auto& engine = moteur::Engine::getInstance();
        int width = 0,height = 0; engine.getWindow().getPixelSize(width,height);
        glViewport(0,0,width,height); glClearColor(color.r,color.g,color.b,color.a); glClear(GL_COLOR_BUFFER_BIT);
        if (!title->render() || !button->render()) { valid = false; engine.stop(); }
    }
private:
    moteur::SceneManager& manager;
    std::shared_ptr<moteur::Font> font;
    std::string name,destination;
    moteur::Color color;
    bool& valid;
    std::unique_ptr<moteur::Label> title;
    std::shared_ptr<moteur::Button> button;
};
bool demonstrate(const std::filesystem::path& fontPath) {
    auto font = std::make_shared<moteur::Font>(20.0f);
    if (!font->load(fontPath.string())) { std::cerr << font->getError() << '\n'; return false; }
    bool valid = true;
    moteur::SceneManager scenes;
    if (!scenes.addScene("A",std::make_unique<DemoScene>(scenes,font,"A","B",moteur::Color{0,0,1},valid)) ||
        !scenes.addScene("B",std::make_unique<DemoScene>(scenes,font,"B","A",moteur::Color{0,1,0},valid)) ||
        !scenes.setActiveScene("A")) { std::cerr << scenes.getError() << '\n'; return false; }
    std::cout << "Cliquez sur le bouton pour changer de scene. Fermez la fenetre pour terminer.\n";
    const bool ran = moteur::Engine::getInstance().run(scenes);
    scenes.clear(); // Ressources graphiques liberees avant le contexte.
    return ran && valid;
}
}
int main(int argc,char** argv) {
    auto& engine = moteur::Engine::getInstance();
    if (!engine.initialize("Demo sprint 2 : scenes A et B",360,180)) { std::cerr << engine.getError() << '\n'; return 1; }
    const char* windows = SDL_getenv("WINDIR");
    if (!windows) windows = SDL_getenv("SystemRoot");
    const auto fontPath = argc > 1 ? std::filesystem::path(argv[1]) :
        std::filesystem::path(windows ? windows : "C:/Windows") / "Fonts/arial.ttf";
    bool valid = false;
    try { valid = demonstrate(fontPath); }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; }
    if (!engine.shutdown()) { std::cerr << engine.getError() << '\n'; return 1; }
    return valid ? 0 : 1;
}
