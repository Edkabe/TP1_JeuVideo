#pragma once

#include "moteur/EventManager.h"

namespace moteur {

class Scene {
public:
    virtual ~Scene() = default;

    // Chaque scène possède son propre gestionnaire d'événements.
    EventManager& getEventManager() { return eventManager; }
    const EventManager& getEventManager() const { return eventManager; }

    // Le moteur appelle cette méthode quand la scène active reçoit un événement.
    void dispatchEvent(const Event& event) { eventManager.notifyObservers(event); }

private:
    EventManager eventManager;
};

}
