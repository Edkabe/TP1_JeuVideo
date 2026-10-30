#pragma once

#include "moteur/EventManager.h"
#include <string>

namespace moteur {

// Contrat commun des scènes; chaque instance conserve ses propres abonnements.
class IScene {
public:
    virtual ~IScene() = default;

    // Contrat: true signifie que toutes les ressources externes, les composants
    // et abonnements nécessaires sont prêts pour update/render. Toute erreur
    // d'initialisation doit être décrite par getError() avant false; unload()
    // reste valide après un échec partiel. Les scènes gardent ce diagnostic
    // accessible jusqu'à ce que l'appelant l'ait traité.
    // Le gestionnaire appelle load à l'activation et unload à la désactivation.
    virtual bool load() = 0;
    virtual void unload() = 0;

    // Ces points d'entrée peuvent être redéfinis par les scènes concrètes.
    virtual void update(float) {}
    virtual void render() {}
    // Une scene conserve son dernier diagnostic afin que la boucle moteur
    // puisse le transmettre a l'appelant apres update/render.
    virtual const std::string& getError() const { static const std::string empty; return empty; }

    EventManager& getEventManager() { return eventManager; }
    const EventManager& getEventManager() const { return eventManager; }

    // La notification est synchrone; Event ne conserve pas l'événement SDL.
    void dispatchEvent(const Event& event) { eventManager.notifyObservers(event); }

private:
    EventManager eventManager;
};

}
