/**
 * @file window.hpp
 * Provides a system for handling the game's window.
 */

#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <entityx/entityx.h>

#include <SDL2/SDL.h>

#include <events.hpp>

/**
 * @class WindowSystem
 * Contains everything needed to create and update a window, using SDL.
 * Also handles window resizing (WIP) and screenshots (WIP).
 */
class WindowSystem : public entityx::System<WindowSystem>, public entityx::Receiver<WindowSystem>  {
private:

	/**
	 * SDL's object for the window.
	 */
    SDL_Window *window;

	/**
	 * An OpenGL context, created when OpenGL is set up for use.
	 */
    SDL_GLContext glContext;

public:
	WindowSystem(void);

	void die(void);

	void configure(entityx::EventManager &ev);
    void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
	void receive(const WindowResizeEvent&);	
	void receive(const ScreenshotEvent&);
};

#endif // WINDOW_HPP_
