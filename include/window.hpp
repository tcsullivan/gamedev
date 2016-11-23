#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <entityx/entityx.h>

#include <SDL2/SDL.h>

#include <events.hpp>

class WindowSystem : public entityx::System<WindowSystem>, public entityx::Receiver<WindowSystem>  {
private:
    SDL_Window *window;
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
