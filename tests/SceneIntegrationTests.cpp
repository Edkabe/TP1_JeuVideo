#include "moteur/Engine.h"
#include "moteur/MainScene.h"
#include "moteur/CreditsScene.h"
#include "moteur/DemoGameScene.h"
#include "moteur/SettingsScene.h"
#include <SDL3/SDL.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
int checks = 0;

void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}

void click(moteur::SceneManager& scenes, float x, float y) {
    SDL_Event event{};
    event.type = SDL_EVENT_MOUSE_MOTION;
    event.motion.x = x;
    event.motion.y = y;
    scenes.dispatchEvent(moteur::Event(event));

    event = {};
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = x;
    event.button.y = y;
    scenes.dispatchEvent(moteur::Event(event));
    event.type = SDL_EVENT_MOUSE_BUTTON_UP;
    scenes.dispatchEvent(moteur::Event(event));
}

void click(const moteur::Button* button, moteur::SceneManager& scenes) {
    check(button != nullptr, "Bouton de scene disponible");
    const auto position = button->getPosition();
    const auto dimensions = button->getDimensions();
    click(scenes, position.x + dimensions.x * 0.5f, position.y + dimensions.y * 0.5f);
}

void openFromMenu(moteur::MainScene& menu, moteur::MainScene::Action action,
                  const char* expectedScene, moteur::SceneManager& scenes) {
    click(menu.getButton(action), scenes);
    check(scenes.getActiveSceneName() == expectedScene, "Le bouton du menu active sa scene cible");
}
}

int main() {
    try {
        auto& engine = moteur::Engine::getInstance();
        check(engine.initialize("Tests integration TP1", 960, 540, true), "Initialisation du moteur cache");

        const char* base = SDL_GetBasePath();
        const char* windows = SDL_getenv("WINDIR");
        if (!windows) windows = SDL_getenv("SystemRoot");
        check(base != nullptr && windows != nullptr, "Chemins des ressources et de Windows disponibles");
        const auto font = std::filesystem::path(windows) / "Fonts/arial.ttf";
        const auto background = std::filesystem::path(base) / "assets/images/menu-background.svg";

        moteur::SceneManager scenes;
        auto menu = std::make_unique<moteur::MainScene>(scenes, font, background,
            moteur::MainScene::Targets{"demonstration", "parametres", "credits"});
        auto* menuView = menu.get();
        auto demo = std::make_unique<moteur::DemoGameScene>(scenes, font, "menu");
        auto* demoView = demo.get();
        auto credits = std::make_unique<moteur::CreditsScene>(scenes, font, "menu");
        auto* creditsView = credits.get();
        auto settings = std::make_unique<moteur::SettingsScene>(scenes, font, background, "menu");
        auto* settingsView = settings.get();

        check(scenes.addScene("menu", std::move(menu)), "Ajout du menu");
        check(scenes.addScene("demonstration", std::move(demo)), "Ajout de la demonstration");
        check(scenes.addScene("credits", std::move(credits)), "Ajout des credits");
        check(scenes.addScene("parametres", std::move(settings)), "Ajout des parametres");
        check(scenes.setActiveScene("menu") && menuView->isLoaded(), "Chargement initial du menu");

        openFromMenu(*menuView, moteur::MainScene::Action::Settings, "parametres", scenes);
        check(settingsView->isLoaded(), "Chargement des controles de parametres");
        click(scenes, 583.0f, 486.0f); // Bouton Retour dans la disposition 960 x 540.
        check(scenes.getActiveSceneName() == "menu" && menuView->isLoaded(), "Retour des parametres au menu");

        openFromMenu(*menuView, moteur::MainScene::Action::Credits, "credits", scenes);
        check(creditsView->isLoaded(), "Chargement des credits");
        click(scenes, 480.0f, 492.0f); // Bouton Retour au menu des credits.
        check(scenes.getActiveSceneName() == "menu" && menuView->isLoaded(), "Retour des credits au menu");

        openFromMenu(*menuView, moteur::MainScene::Action::Start, "demonstration", scenes);
        check(demoView->isLoaded(), "Chargement de la demonstration et de son glisseur");
        click(demoView->getReturnButton(), scenes);
        check(scenes.getActiveSceneName() == "menu" && menuView->isLoaded(), "Retour de la demonstration au menu");

        const auto resourceCount = engine.getResourceManager().size();
        openFromMenu(*menuView, moteur::MainScene::Action::Settings, "parametres", scenes);
        click(scenes, 583.0f, 486.0f);
        openFromMenu(*menuView, moteur::MainScene::Action::Credits, "credits", scenes);
        click(scenes, 480.0f, 492.0f);
        openFromMenu(*menuView, moteur::MainScene::Action::Start, "demonstration", scenes);
        click(demoView->getReturnButton(), scenes);
        check(engine.getResourceManager().size() == resourceCount,
              "Les ressources partagees sont reutilisees apres des transitions repetees");

        scenes.clear();
        check(engine.shutdown(), "Fermeture propre apres dechargement de toutes les scenes");
        std::cout << checks << " verifications d'integration reussies.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "ECHEC : " << error.what() << '\n';
        return 1;
    }
}
