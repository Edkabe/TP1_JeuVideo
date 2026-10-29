#include "moteur/Engine.h"
#include <SDL3/SDL.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {
int checks = 0;
void check(bool value, const char* message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}
}

int main() {
    // Le moteur lit settings.bin relativement au dossier courant. Un dossier
    // temporaire isole ce test des preferences de l'application de l'utilisateur.
    const auto directory = std::filesystem::temp_directory_path() / "tp1-engine-settings-tests";
    std::error_code ec;
    std::filesystem::remove_all(directory, ec);
    std::filesystem::create_directories(directory, ec);
    if (ec) {
        std::cerr << "Impossible de preparer le dossier de test: " << ec.message() << '\n';
        return 1;
    }
    std::filesystem::current_path(directory);

    try {
        moteur::Settings initial("settings.bin");
        check(initial.setLanguage("en"), "Langue de depart valide");
        check(initial.setResolution(800, 600), "Resolution de depart valide");
        check(initial.setSensitivity(2.5f) && initial.setFov(105.0f), "Sensibilite et FOV de depart valides");
        initial.setFullscreen(false);
        initial.setVSync(true);
        check(initial.save(), "Preparation du fichier pour l'initialisation");

        auto& engine = moteur::Engine::getInstance();
        check(engine.initialize("Test reglages", 960, 540, true), "Initialisation avec les reglages sauvegardes");
        int width = 0, height = 0;
        check(engine.getWindow().getSize(width, height) && width == 800 && height == 600,
              "La resolution sauvegardee est appliquee au demarrage");
        check(!engine.getWindow().isFullscreen(), "Le mode fenetre sauvegarde est applique au demarrage");
        check(engine.getWindow().isVSyncEnabled(), "La VSync sauvegardee est appliquee au demarrage");
        check(engine.getSettings().getLanguage() == "en", "La langue sauvegardee est accessible depuis le moteur");
        check(engine.getSettings().getSensitivity() == 2.5f && engine.getSettings().getFov() == 105.0f,
              "La sensibilite et le FOV sauvegardes sont accessibles depuis le moteur");

        auto& settings = engine.getSettings();
        check(settings.setResolution(1024, 768), "Nouvelle resolution valide");
        settings.setVSync(false);
        check(engine.applySettings(), "Application immediate de la resolution et de la VSync");
        check(engine.getWindow().getSize(width, height) && width == 1024 && height == 768,
              "La nouvelle resolution est visible immediatement");
        check(!engine.getWindow().isVSyncEnabled(), "La desactivation VSync est visible immediatement");

        // Choisir une resolution vraiment annoncee par l'ecran pour verifier le
        // passage en plein ecran sans supposer un mode precis sur chaque PC.
        const SDL_DisplayID display = SDL_GetDisplayForWindow(engine.getWindow().getSDLWindow());
        int modeCount = 0;
        SDL_DisplayMode** modes = display ? SDL_GetFullscreenDisplayModes(display, &modeCount) : nullptr;
        const SDL_DisplayMode* chosen = nullptr;
        for (int i = 0; modes && i < modeCount; ++i) {
            if (modes[i] && modes[i]->w >= 320 && modes[i]->w <= 1920 &&
                modes[i]->h >= 200 && modes[i]->h <= 1080) {
                chosen = modes[i];
                break;
            }
        }
        if (chosen) {
            const int fullscreenWidth = chosen->w;
            const int fullscreenHeight = chosen->h;
            check(settings.setResolution(fullscreenWidth, fullscreenHeight), "Resolution plein ecran valide");
            settings.setFullscreen(true);
            check(engine.applySettings(), "Application du mode plein ecran demande");
            check(engine.getWindow().isFullscreen(), "Le plein ecran est actif");
            check(engine.getWindow().getSize(width, height) && width == fullscreenWidth && height == fullscreenHeight,
                  "La resolution plein ecran demandee est active");

            check(settings.setResolution(1024, 768), "Retour a une resolution fenetree valide");
            settings.setFullscreen(false);
            check(engine.applySettings(), "Retour au mode fenetre");
            check(!engine.getWindow().isFullscreen(), "Le mode fenetre est restaure");
            check(engine.getWindow().getSize(width, height) && width == 1024 && height == 768,
                  "La resolution fenetree est restauree");
        } else {
            std::cout << "Aucun mode plein ecran exploitable: verification plein ecran ignoree.\n";
        }
        if (modes) SDL_free(modes);

        check(engine.shutdown(), "Arret et sauvegarde des reglages");
        check(settings.load(), "Relecture des reglages apres arret");
        check(settings.getWidth() == 1024 && settings.getHeight() == 768 &&
              !settings.isFullscreen() && !settings.isVSyncEnabled(),
              "Les valeurs appliquees correspondent au fichier sauvegarde");
        check(settings.getLanguage() == "en" && settings.getSensitivity() == 2.5f && settings.getFov() == 105.0f,
              "La langue, la sensibilite et le FOV restent sauvegardes");
        check(engine.initialize("Test redemarrage", 960, 540, true), "Reinitialisation du moteur apres relecture");
        check(engine.getWindow().getSize(width, height) && width == settings.getWidth() && height == settings.getHeight(),
              "Le redemarrage reapplique les valeurs sauvegardees");
        check(!engine.getWindow().isFullscreen() && !engine.getWindow().isVSyncEnabled(),
              "Le redemarrage reapplique les booleens sauvegardes");
        check(engine.shutdown(), "Arret final du moteur");

        std::cout << checks << " verifications reussies.\n";
        return 0;
    } catch (const std::exception& exception) {
        auto& engine = moteur::Engine::getInstance();
        if (engine.isRunning()) engine.stop();
        engine.shutdown();
        std::cerr << "ECHEC : " << exception.what() << '\n';
        return 1;
    }
}
