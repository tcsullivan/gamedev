/** @file ui.h
 * @brief Contains functions for handling the user interface.
 */

#ifndef UI_H
#define UI_H

#include <common.h>
#include <cstdarg>

#include <world.h>
#include <ft2build.h>
#include <SDL2/SDL_opengl.h>
#include FT_FREETYPE_H

#define DEBUG

typedef void(*menuFunc)();


struct menuItem{
	int member;
	union{

		struct{
			vec2 loc;
			dim2 dim;
			Color color;
			const char* text;
			menuFunc func;
		}button;

		struct{
			vec2 loc;
			dim2 dim;
			Color color;
			float minValue;
			float maxValue;
			const char* text;
			float* var;

			float sliderLoc;
		}slider;

	};
};

namespace ui {

	menuItem createButton(vec2 l, dim2 d, Color c, const char* t, menuFunc f);
	menuItem createSlider(vec2 l, dim2 d, Color c, float min, float max, const char* t, float* v);
	/**
	 *	Contains the coordinates of the mouse inside the window.
	 */

	extern vec2 mouse;

	/*
	 *	These flags are used elsewhere.
	*/

	extern bool oMenu;
	extern bool pMenu;
	extern bool menu;
	extern bool debug;
	extern bool posFlag;
	extern unsigned int fontSize;
	extern bool			 dialogBoxExists;
	extern unsigned char dialogOptChosen;
	extern bool dialogImportant;

	extern unsigned int textWrapLimit;

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
	
	/*
	 *	Draw a centered string.
	*/
	
	float putStringCentered(const float x,const float y,const char *s);
	
	/*
	 *	Draws a formatted string at the given coordinates.
	*/
	
	float putText(const float x,const float y,const char *str,...);
	
	/*
	 *	Creates a dialogBox text string (format: `name`: `text`). This function simply sets up
	 *	variables that are drawn in ui::draw(). When the dialog box exists player control is
	 *	limited until a right click is given, closing the box.
	*/
	
	void dialogBox(const char *name,const char *opt,bool passive,const char *text,...);
	void waitForDialog(void);
	
	/*
	 *	Draws a larger string in the center of the screen. Drawing is done inside this function.
	*/
	
	void importantText(const char *text,...);
	
	/*
	 *	Draw various UI elements (dialogBox, player health)
	*/
	
	void draw(void);
	

	/*
	 *	Draw various menu items
	*/
	void quitGame();
	void quitMenu();
	void optionsMenuF();
	void drawMenu(std::vector<menuItem>mi);


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
	
	void waitForNothing(unsigned int);
}

#endif // UI_H
