#include "moteur/EventManager.h"
#include <algorithm>

namespace moteur {

bool EventManager::addObserver(const std::shared_ptr<EventObserver>& observer, Uint32 type) {
    if (!observer) return false;

    // Nettoyer les anciennes inscriptions avant de vérifier les doublons.
    registrations.erase(
        std::remove_if(registrations.begin(), registrations.end(),
            [](const Registration& registration) { return registration.observer.expired(); }),
        registrations.end());

    const auto duplicate = std::find_if(registrations.begin(), registrations.end(),
        [&](const Registration& registration) {
            return registration.type == type && registration.observer.lock() == observer;
        });
    if (duplicate != registrations.end()) return false;

    registrations.push_back({observer, type});
    return true;
}

bool EventManager::removeObserver(const std::shared_ptr<EventObserver>& observer) {
    if (!observer) return false;

    const std::size_t oldSize = registrations.size();
    registrations.erase(
        std::remove_if(registrations.begin(), registrations.end(),
            [&](const Registration& registration) {
                const auto registeredObserver = registration.observer.lock();
                return !registeredObserver || registeredObserver == observer;
            }),
        registrations.end());

    return registrations.size() != oldSize;
}

void EventManager::notifyObservers(const Event& event) {
    // Une copie permet aux callbacks de modifier les inscriptions pendant la notification.
    const auto snapshot = registrations;

    for (const Registration& registration : snapshot) {
        if (registration.type != event.getType()) continue;

        // Le verrou maintient l'observateur vivant pendant son callback.
        const auto observer = registration.observer.lock();
        if (!observer) continue;

        // Un callback précédent peut avoir retiré cet observateur.
        const auto stillRegistered = std::find_if(registrations.begin(), registrations.end(),
            [&](const Registration& current) {
                return current.type == event.getType() && current.observer.lock() == observer;
            });
        if (stillRegistered != registrations.end()) observer->onEvent(event);
    }

    // Les objets détruits sont éliminés après la distribution.
    registrations.erase(
        std::remove_if(registrations.begin(), registrations.end(),
            [](const Registration& registration) { return registration.observer.expired(); }),
        registrations.end());
}

}
