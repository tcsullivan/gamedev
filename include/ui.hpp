/* ----------------------------------------------------------------------------
** The user interface system.
**
** This file contains everything user-interface related.
** --------------------------------------------------------------------------*/
#ifndef UI_H
#define UI_H

#define DEBUG
#define SDL_KEY e.key.keysym.sym

/* ----------------------------------------------------------------------------
** Includes section
** --------------------------------------------------------------------------*/

// standard library headers
#include <cstdarg>
#include <cstdint>
#include <thread>

// local game headers
#include <common.hpp>
#include <config.hpp>
#include <entities.hpp>
#include <inventory.hpp>
#include <ui_menu.hpp>
#include <ui_action.hpp>

// local library headers
#include <SDL2/SDL_opengl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifndef __WIN32__
#	include <bmpimage.hpp>
#endif // __WIN32__

/* ----------------------------------------------------------------------------
** The UI namespace
** --------------------------------------------------------------------------*/

namespace ui {

	// the pixel-coordinates of the mouse
	extern vec2 mouse;

	// raw mouse values from SDL
    extern vec2 premouse;

	// the currently used font size for text rendering
	extern unsigned int fontSize;

	// shows the debug overlay when set to true
	extern bool debug;

	// shows tracers when set to true (alongside `debug`)
	extern bool posFlag;

	extern unsigned char dialogOptChosen;
	extern unsigned char merchOptChosen;
	extern bool 		 dialogBoxExists;
	extern bool 		 dialogImportant;
	extern bool 		 dialogPassive;

	extern unsigned int textWrapLimit;
	extern int fontTransInv;

	/*
	 *	Initializes the FreeType system.
	*/

	void initFonts(void);

	void destroyFonts(void);

	/*
	 *	Sets the current font/font size.
	*/

	void setFontFace(const char *ttf);
	void setFontSize(unsigned int size);
	void setFontColor(unsigned char r,unsigned char g,unsigned char b, unsigned char a);
	void setFontZ(float z);

	/*
	 *	Draw a centered string.
	*/

	float putStringCentered(const float x,const float y,std::string s);

	/*
	 *	Draws a formatted string at the given coordinates.
	*/

	float putText(const float x,const float y,const char *str,...);

	/**
	 * This function is a facility for logic events to draw text; the text
	 * will be prepared then drawn in the render loop.
	 */
	void putTextL(vec2 c,const char *str, ...);

	/*
	 *	Creates a dialogBox text string (format: `name`: `text`). This function simply sets up
	 *	variables that are drawn in ui::draw(). When the dialog box exists player control is
	 *	limited until a right click is given, closing the box.
	*/

	void drawBox(vec2 c1, vec2 c2);
	void drawNiceBox(vec2 c1, vec2 c2, float z);
	void dialogBox(std::string name, std::string opt, bool passive, std::string text, ...);
	void merchantBox(const char *name,Trade trade,const char *opt,bool passive,const char *text,...);
	void merchantBox();
	void closeBox();
	void waitForDialog(void);

	bool pageExists(void);
	void drawPage(const GLuint& tex);

	void dontTypeOut(void);
	/*
	 *	Draws a larger string in the center of the screen. Drawing is done inside this function.
	*/

	void importantText(const char *text,...);
	void passiveImportantText(int duration,const char *text,...);

	/*
	 *	Draw various UI elements (dialogBox, player health)
	*/

	void draw(void);
	void drawFade(void);
	void fadeUpdate(void);

	void quitGame();



	/*
	 *	Handle keyboard/mouse events.
	*/
	void handleEvents(void);

	/*
	 *	Toggle the black overlay thing.
	*/

	void toggleBlack(void);
	void toggleBlackFast(void);
	void toggleWhite(void);
	void toggleWhiteFast(void);
	void waitForCover(void);

}

#endif // UI_H
