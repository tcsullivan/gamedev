/** @file ui.h
 * @brief Contains functions for handling the user interface.
 */

#ifndef UI_H
#define UI_H

#include <common.h>
#include <cstdarg>

#include <world.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define DEBUG

namespace ui {

	/**
	 *	Contains the coordinates of the mouse inside the window.
	 */

	extern vec2 mouse;

	/*
	 *	These flags are used elsewhere.
	*/

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
