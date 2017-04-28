/**
 * @file ui.hpp
 * @brief the user interface system.
 */
#ifndef UI_HPP_
#define UI_HPP_

#include <cstdarg>
#include <string>

#include <entityx/entityx.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#include <color.hpp>
#include <events.hpp>
#include <vector2.hpp>

#define DEBUG
#define SDL_KEY e.key.keysym.sym

void setControl(int index, SDL_Keycode key);
SDL_Keycode getControl(int index);

class InputSystem : public entityx::System<InputSystem>, public entityx::Receiver<InputSystem> {
public:
	inline void configure(entityx::EventManager &ev) {
		ev.subscribe<MainSDLEvent>(*this);
	}

	void receive(const MainSDLEvent& event);
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
};

struct OptionDim {
	float x;
	float y;
	float width;
};

using DialogOption = std::pair<OptionDim, std::string>;

class UISystem : public entityx::System<UISystem> {
private:
	static bool fadeEnable;
	static bool fadeFast;
	static int  fadeIntensity;

	static std::string dialogText;
	static std::string importantText;
	static std::vector<DialogOption> dialogOptions;
	static int dialogOptionResult;

public:
	UISystem(void) {}

	void update(entityx::EntityManager& en, entityx::EventManager& ev, entityx::TimeDelta dt) override;

	static void render(void);

	/***
	 * Fade library
	 */

	static void fadeToggle(void);
	static void fadeToggleFast(void);
	static void waitForCover(void);
	static void waitForUncover(void);


	static inline bool isFading(void)
	{ return fadeIntensity != 0; }

	static inline bool isDialog(void)
	{ return !dialogText.empty() || !importantText.empty(); }

	/**
	 * Text library
	 */

	static void putText(const vec2& p, const std::string& s, ...);
	static void putString(const vec2& p, const std::string& s, float wrap = 0.12345f);
	static float putStringCentered(const vec2& p, const std::string& s, bool print = true);

	static void dialogBox(const std::string& n, const std::string& s, ...);
	static void dialogAddOption(const std::string& o);
	static void dialogImportant(const std::string& s);

	static void waitForDialog(void);
	static void advanceDialog(void);
	static int getDialogResult(void);
};

namespace ui {
	// the pixel-coordinates of the mouse
	extern vec2 mouse;

	// raw mouse values from SDL
    extern vec2 premouse;

	// shows the debug overlay when set to true
	extern bool debug;

	// shows tracers when set to true (alongside `debug`)
	extern bool posFlag;

	void initSounds(void);

	void drawNiceBox(vec2 c1, vec2 c2, float z);
	void drawNiceBoxColor(vec2 c1, vec2 c2, float z, Color c);

	bool pageExists(void);
	void drawPage(const GLuint& tex);

	//void importantText(const char *text,...);
	//void passiveImportantText(int duration,const char *text,...);

	/*
	 *	Draw various UI elements (dialogBox, player health)
	*/

	void draw(void);

	/*
 	 *  Takes a screenshot of the game
 	 */

	void takeScreenshot(GLubyte *pixels);

	bool handleGLEvent(SDL_Event& e);
}

#endif // UI_HPP_
