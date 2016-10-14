#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <entityx/entityx.h>

#include <SDL2/SDL.h>

class WindowSystem : public entityx::System<WindowSystem> {
private:
    SDL_Window *window;
    SDL_GLContext glContext;

public:
	WindowSystem(void);

	void die(void);

    void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
};

#endif // WINDOW_HPP_
