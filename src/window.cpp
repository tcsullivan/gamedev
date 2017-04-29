#include <window.hpp>

#include <config.hpp>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

constexpr const char* WINDOW_TITLE  = "gamedev";

SDL_Window*   WindowSystem::window;
SDL_GLContext WindowSystem::glContext;

WindowSystem::WindowSystem(void)
{
    // attempt to initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cout << "SDL was not able to initialize! Error: " << SDL_GetError();
        abort();
    }
    atexit(SDL_Quit);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // attempt to initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
        std::cout << "Could not init image libraries! Error: " << IMG_GetError();
        abort();
    }
    atexit(IMG_Quit);

    // attempt to initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! Error: " << Mix_GetError();
        abort();
    }
    Mix_AllocateChannels(8);
    atexit(Mix_Quit);
	atexit(Mix_CloseAudio);

    // create the SDL window object
    window = SDL_CreateWindow(WINDOW_TITLE,
                              SDL_WINDOWPOS_UNDEFINED,	// Spawn the window at random (undefined) x and y coordinates
                              SDL_WINDOWPOS_UNDEFINED,	//
                              game::SCREEN_WIDTH,
                              game::SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
                              );

    if (window == nullptr) {
        std::cout << "The window failed to generate! SDL_Error: " << SDL_GetError();
        abort();
    }

    // create the OpenGL object that SDL provides
    if ((glContext = SDL_GL_CreateContext(window)) == nullptr) {
        std::cout << "The OpenGL context failed to initialize! SDL_Error: " << SDL_GetError();
        abort();
    }
}

void WindowSystem::die(void)
{
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
}


void WindowSystem::configure(entityx::EventManager &ev)
{
    ev.subscribe<WindowResizeEvent>(*this);
	ev.subscribe<ScreenshotEvent>(*this);
}


void WindowSystem::receive(const WindowResizeEvent &wre)
{
	game::SCREEN_WIDTH = wre.x;
	game::SCREEN_HEIGHT = wre.y;

	glViewport(0, 0, wre.x, wre.y);
	SDL_SetWindowSize(window, wre.x, wre.y);
}

#include <ui.hpp>

#include <atomic>

static std::atomic_bool doScreenshot;

void WindowSystem::receive(const ScreenshotEvent &scr)
{
	(void)scr;
	doScreenshot.store(true);
}

void WindowSystem::render(void)
{
	if (doScreenshot.load()) {
		doScreenshot.store(false);
		// Make the BYTE array, factor of 3 because it's RBG.
		int count = 3 * game::SCREEN_WIDTH * game::SCREEN_HEIGHT;
		GLubyte* pixels = new GLubyte[count];
		glReadPixels(0, 0, game::SCREEN_WIDTH, game::SCREEN_HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, pixels);
		ui::takeScreenshot(pixels);
	}

    SDL_GL_SwapWindow(window);
}
