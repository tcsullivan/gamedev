#ifndef EVENTS_HPP_
#define EVENTS_HPP_

/**
 * A place for events to live, while gamedev slowly dies from rewriting.
 */

#include <SDL2/SDL.h>

 struct MouseScrollEvent {
 	MouseScrollEvent(int sd)
 		: scrollDistance(sd) {}

 	int scrollDistance;
 };

struct KeyDownEvent {
    KeyDownEvent(SDL_Keycode kc)
        : keycode(kc) {}

    SDL_Keycode keycode;
};

struct KeyUpEvent {
    KeyUpEvent(SDL_Keycode kc)
        : keycode(kc) {}

    SDL_Keycode keycode;
};

#endif // EVENTS_HPP_
