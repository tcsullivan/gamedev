#ifndef EVENTS_HPP_
#define EVENTS_HPP_

/**
 * A place for events to live, while gamedev slowly dies from rewriting.
 */

#include <SDL2/SDL.h>

#include <string>
#include <common.hpp>

class World;

//////////////////////////
/// INPUT EVENTS
//////////////////////////

struct MouseScrollEvent {
 	MouseScrollEvent(int sd = 0)
 		: scrollDistance(sd) {}

 	int scrollDistance;
};

struct MouseClickEvent {
	MouseClickEvent(vec2 pos, int b)
		: position(pos), button(b) {}

	vec2 position;
	int button;
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

//////////////////////////
/// ENGINE EVENTS
//////////////////////////

struct GameEndEvent {
    GameEndEvent(bool r = true)
        : really(r) {}

    bool really;
};

//////////////////////////
/// WORLD EVENTS
//////////////////////////

struct BGMToggleEvent {
    BGMToggleEvent(std::string f = "", World *w = nullptr)
        : file(f), world(w) {}

    std::string file;
	World *world;
};

//////////////////////////
/// WINDOW EVENTS
//////////////////////////

struct WindowResizeEvent {
	WindowResizeEvent(int w = 640, int h = 480)
		: x(w), y(h) {}

	int x;
	int y;
};

struct ScreenshotEvent {
	ScreenshotEvent(int w = game::SCREEN_HEIGHT, int h = game::SCREEN_WIDTH)
		: w(w), h(h) {}

	int w;
	int h;
};

#endif // EVENTS_HPP_
