#include "moteur/Bibliotheques.h"
#include "moteur/Scene.h"
#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>

namespace {

class ObservateurCompteur final : public moteur::EventObserver {
public:
    explicit ObservateurCompteur(int& compteur) : compteur(compteur) {}

    void onEvent(const moteur::Event&) override { ++compteur; }

private:
    int& compteur;
};

// L'interface IScene est abstraite: le contrôle utilise ici une scène minimale.
class SceneVide final : public moteur::IScene {
public:
    bool load() override { return true; }
    void unload() override {}
};

bool verifierGestionnaireEvenements() {
    SceneVide sceneA;
    SceneVide sceneB;
    if (&sceneA.getEventManager() == &sceneB.getEventManager()) return false;

    moteur::EventManager& manager = sceneA.getEventManager();
    int recus = 0;
    int recusApresRetrait = 0;
    auto observateur = std::make_shared<ObservateurCompteur>(recus);
    auto observateurRetire = std::make_shared<ObservateurCompteur>(recusApresRetrait);

    if (!manager.addObserver(observateur, SDL_EVENT_KEY_DOWN) ||
        !manager.addObserver(observateurRetire, SDL_EVENT_KEY_DOWN) ||
        !manager.removeObserver(observateurRetire)) return false;

    SDL_Event sdlEvent{};
    sdlEvent.type = SDL_EVENT_KEY_DOWN;
    const moteur::Event event(sdlEvent);
    manager.notifyObservers(event);
    if (recus != 1 || recusApresRetrait != 0) return false;

    // Un observateur inscrit aux touches ne reçoit pas un clic de souris.
    SDL_Event autreSdlEvent{};
    autreSdlEvent.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    const moteur::Event autreEvent(autreSdlEvent);
    manager.notifyObservers(autreEvent);
    if (recus != 1 || recusApresRetrait != 0) return false;

    // Un observateur détruit sans retrait manuel ne doit pas rester référencé.
    std::weak_ptr<ObservateurCompteur> observateurDetruit;
    {
        auto temporaire = std::make_shared<ObservateurCompteur>(recusApresRetrait);
        observateurDetruit = temporaire;
        if (!manager.addObserver(temporaire, SDL_EVENT_KEY_DOWN)) return false;
    }
    manager.notifyObservers(event);

    return recus == 2 && recusApresRetrait == 0 && observateurDetruit.expired();
}

}

namespace moteur
{
    bool verifierBibliotheques()
    {
        // Le chargement réel de GLAD attendra un contexte OpenGL actif.
        auto chargeur = &gladLoadGL;
        const glm::vec2 position(2.0f, 3.0f);
        const glm::vec2 deplacement(1.0f, 1.0f);
        const auto resultat = position + deplacement;
        std::cout << "SDL3 : " << SDL_GetVersion()
                  << " | SDL_image : " << IMG_Version()
                  << " | SDL_ttf : " << TTF_Version() << '\n';
        const bool evenementsValides = verifierGestionnaireEvenements();
        if (evenementsValides) std::cout << "Gestionnaire d'evenements : filtrage, retrait, scenes et duree de vie verifies.\n";
        return chargeur != nullptr && resultat.x == 3.0f && resultat.y == 4.0f && evenementsValides;
    }
}
