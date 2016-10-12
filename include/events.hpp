#ifndef EVENTS_HPP_
#define EVENTS_HPP_

/**
 * A place for events to live, while gamedev slowly dies from rewriting.
 */

#include <SDL2/SDL.h>

#include <string>

struct MouseScrollEvent {
 	MouseScrollEvent(int sd = 0)
 		: scrollDistance(sd) {}

 	int scrollDistance;
 };

struct KeyDownEvent {
    KeyDownEvent(SDL_Keycode kc = 0)
        : keycode(kc) {}

    SDL_Keycode keycode;
};

struct KeyUpEvent {
    KeyUpEvent(SDL_Keycode kc = 0)
        : keycode(kc) {}

    SDL_Keycode keycode;
};

struct GameEndEvent {
    GameEndEvent(bool r = true)
        : really(r) {}

    bool really;
};

struct BGMToggleEvent {
    BGMToggleEvent(std::string f)
        : file(f) {}

    std::string file;
}

#endif // EVENTS_HPP_
