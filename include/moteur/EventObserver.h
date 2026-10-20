#pragma once

#include "moteur/Event.h"

namespace moteur {

// Un observateur concret hérite de cette interface et définit sa réaction.
class EventObserver {
public:
    virtual ~EventObserver() = default;

    virtual void onEvent(const Event& event) = 0;
};

}
