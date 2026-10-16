#pragma once

#include "moteur/EventObserver.h"
#include <memory>
#include <vector>

namespace moteur {

class EventManager {
public:
    // Un observateur s'inscrit pour un type précis; un même couple
    // observateur/type n'est conservé qu'une fois.
    bool addObserver(const std::shared_ptr<EventObserver>& observer, Uint32 type);

    // Le retrait enlève toutes les inscriptions de cet observateur.
    bool removeObserver(const std::shared_ptr<EventObserver>& observer);

    // Seuls les observateurs inscrits au type de l'événement sont appelés.
    void notifyObservers(const Event& event);

private:
    struct Registration {
        // Une référence faible évite que le gestionnaire prolonge la vie d'un objet.
        std::weak_ptr<EventObserver> observer;
        Uint32 type;
    };

    std::vector<Registration> registrations;
};

}
