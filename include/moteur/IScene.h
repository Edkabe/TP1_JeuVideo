#pragma once

#include "moteur/EventManager.h"

namespace moteur {

// Contrat commun des scènes; chaque instance conserve ses propres abonnements.
class IScene {
public:
    virtual ~IScene() = default;

    // Le gestionnaire appelle load à l'activation et unload à la désactivation.
    virtual bool load() = 0;
    virtual void unload() = 0;

    // Ces points d'entrée peuvent être redéfinis par les scènes concrètes.
    virtual void update(float) {}
    virtual void render() {}

    EventManager& getEventManager() { return eventManager; }
    const EventManager& getEventManager() const { return eventManager; }

    // La notification est synchrone; Event ne conserve pas l'événement SDL.
    void dispatchEvent(const Event& event) { eventManager.notifyObservers(event); }

private:
    EventManager eventManager;
};

}
