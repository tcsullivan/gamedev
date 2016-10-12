#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include <entityx/entityx.h>

#include <events.hpp>

namespace game {
    extern entityx::EventManager events;
    extern entityx::EntityManager entities;

    inline void endGame(void) {
        events.emit<GameEndEvent>();
    }
}

#endif // ENGINE_HPP_
