#pragma once

#include <SDL3/SDL.h>

namespace moteur {

class Event {
public:
    // L'événement SDL reste emprunté : les observateurs sont notifiés tout de
    // suite pendant que l'événement original est encore valide.
    explicit Event(const SDL_Event& event) : sdlEvent(&event) {}

    Uint32 getType() const { return sdlEvent->type; }
    const SDL_Event& getSDLEvent() const { return *sdlEvent; }

private:
    // Ce pointeur n'est valide que pendant la durée de vie de l'événement SDL.
    const SDL_Event* sdlEvent;
};

}
