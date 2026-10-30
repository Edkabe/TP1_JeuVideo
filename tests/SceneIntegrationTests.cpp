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

class FailingUpdateScene final : public moteur::IScene {
public:
    bool load() override { return true; }
    void unload() override {}
    void update(float) override { error = "Erreur de mise a jour propagee."; }
    const std::string& getError() const override { return error; }
private:
    std::string error;
};

void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}

void click(moteur::SceneManager& scenes, float x, float y) {
    SDL_Event event{};
    event.type = SDL_EVENT_MOUSE_MOTION;
    event.motion.windowID = moteur::Engine::getInstance().getWindow().getId();
    event.motion.x = x;
    event.motion.y = y;
    scenes.dispatchEvent(moteur::Event(event));

    event = {};
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.windowID = moteur::Engine::getInstance().getWindow().getId();
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
        auto& preferences = engine.getSettings();
        check(preferences.setLanguage("fr") && preferences.setResolution(960, 540),
              "Preparation de valeurs de depart connues pour les parametres");
        preferences.setFullscreen(false);
        preferences.setVSync(true);
        preferences.setSensitivity(1.0f);
        preferences.setFov(90.0f);
        check(preferences.save(), "Sauvegarde des valeurs de depart des parametres");
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
        const auto mainFont = menuView->getSharedFont();
        const auto mainBackground = menuView->getSharedBackground();

        openFromMenu(*menuView, moteur::MainScene::Action::Settings, "parametres", scenes);
        check(settingsView->isLoaded(), "Chargement des controles de parametres");
        check(settingsView->getSharedFont() == mainFont,
              "Le menu et les parametres partagent exactement le meme objet Font");
        check(settingsView->getSharedBackground() == mainBackground,
              "Le menu et les parametres partagent exactement le meme objet Texture");
        click(scenes, 480.0f, 113.0f); // Valeur du selecteur de langue.
        click(scenes, 480.0f, 183.0f); // Valeur du selecteur de resolution.
        click(scenes, 362.0f, 276.0f); // Case VSync dans la disposition 960 x 540.
        click(scenes, 377.0f, 486.0f); // Appliquer dans la disposition 960 x 540.
        int appliedWidth = 0, appliedHeight = 0;
        check(preferences.getLanguage() == "en", "La langue choisie est appliquee au clic sur Appliquer");
        check(preferences.getWidth() == 1280 && preferences.getHeight() == 720 &&
              engine.getWindow().getSize(appliedWidth, appliedHeight) &&
              appliedWidth == 1280 && appliedHeight == 720,
              "La resolution choisie change la fenetre au clic sur Appliquer");
        check(!preferences.isVSyncEnabled() && !engine.getWindow().isVSyncEnabled(),
              "Le clic sur la case VSync applique aussi le changement a la fenetre");
        check(scenes.getActiveSceneName() == "parametres", "Appliquer garde l'ecran ouvert avec son retour visible");
        check(preferences.setResolution(960, 540), "Restauration de la resolution apres le test des controles");
        preferences.setVSync(true);
        check(engine.applySettings() && preferences.save(), "Restauration de l'affichage apres le test des controles");
        check(scenes.setActiveScene("menu") && menuView->isLoaded(), "Retour des parametres au menu");
        check(menuView->getButton(moteur::MainScene::Action::Settings)->getLabel().getText() == "Settings" &&
              menuView->getButton(moteur::MainScene::Action::Start)->getLabel().getText() == "Play",
              "Le menu se traduit en anglais apres application de la langue");

        openFromMenu(*menuView, moteur::MainScene::Action::Credits, "credits", scenes);
        check(creditsView->isLoaded(), "Chargement des credits");
        click(scenes, 480.0f, 492.0f); // Bouton Retour au menu des credits.
        check(scenes.getActiveSceneName() == "menu" && menuView->isLoaded(), "Retour des credits au menu");

        openFromMenu(*menuView, moteur::MainScene::Action::Start, "demonstration", scenes);
        check(demoView->isLoaded(), "Chargement de la scene de demarrage");
        check(demoView->getLoadingProgress() == 0.0f && !demoView->isLoadingComplete(),
              "La progression commence a zero");
        scenes.render();
        check(demoView->getError().empty(), "Rendu OpenGL de la scene de demarrage");
        demoView->update(1.5f);
        check(demoView->getLoadingProgress() > 0.49f && demoView->getLoadingProgress() < 0.51f,
              "La barre avance avec la duree du demarrage");
        demoView->update(2.0f);
        check(demoView->isLoadingComplete() && demoView->getLoadingProgress() == 1.0f,
              "La progression atteint 100 pour cent et termine le demarrage simule");
        click(demoView->getReturnButton(), scenes);
        check(scenes.getActiveSceneName() == "menu" && menuView->isLoaded(), "Retour de la scene de demarrage au menu");

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
        moteur::SceneManager failingScenes;
        check(failingScenes.addScene("erreur", std::make_unique<FailingUpdateScene>()) &&
              failingScenes.setActiveScene("erreur"), "Preparation d'une scene dont update echoue");
        check(!engine.run(failingScenes) && engine.getError() == "Erreur de mise a jour propagee.",
              "Engine::run transmet au code appelant l'erreur d'une scene");
        failingScenes.clear();
        check(engine.shutdown(), "Fermeture propre apres dechargement de toutes les scenes");
        std::cout << checks << " verifications d'integration reussies.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "ECHEC : " << error.what() << '\n';
        return 1;
    }
}
