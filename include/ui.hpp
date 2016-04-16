/** @file ui.h
 * @brief Contains functions for handling the user interface.
 */

#ifndef UI_H
#define UI_H

#include <common.hpp>
#include <inventory.hpp>
#include <cstdarg>

#include <config.hpp>
#include <world.hpp>

#include <ui_menu.hpp>
#include <ui_action.hpp>

#include <ft2build.h>
#include <SDL2/SDL_opengl.h>
#include <thread>
#include FT_FREETYPE_H

#define SDL_KEY e.key.keysym.sym

#define DEBUG

typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef  int32_t LONG;

typedef struct{
	WORD 	bfType;
	DWORD 	bfSize;
	WORD 	bfReserved1, bfReserved2;
	DWORD 	bfOffBits; //how many bytes before the image data
} __attribute__ ((packed)) BITMAPFILEHEADER;

typedef struct{
	DWORD 	biSize; //size of header in bytes
	LONG 	biWidth;
	LONG 	biHeight;
	WORD 	biPlanes;
	WORD 	biBitCount; //how many bits are in a pixel
	DWORD 	biCompression;
	DWORD 	biSizeImage; //size of image in bytes
	LONG 	biXPelsPerMeter;
	LONG 	biYPelsPerMeter;
	DWORD 	biClrUsed; //how many colors there are
	DWORD 	biClrImportant; //important colors
} __attribute__ ((packed)) BITMAPINFOHEADER;

namespace ui {

	/**
	 *	Contains the coordinates of the mouse inside the window.
	 */

	extern vec2 mouse;
    extern vec2 premouse;

	/*
	 *	These flags are used elsewhere.
	*/

	extern unsigned int fontSize;

	extern bool debug;
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

	/*
	 *	Draw a centered string.
	*/

	float putStringCentered(const float x,const float y,std::string s);

	/*
	 *	Draws a formatted string at the given coordinates.
	*/

	float putText(const float x,const float y,const char *str,...);

	/*
	 *	Creates a dialogBox text string (format: `name`: `text`). This function simply sets up
	 *	variables that are drawn in ui::draw(). When the dialog box exists player control is
	 *	limited until a right click is given, closing the box.
	*/

	void drawBox(vec2 c1, vec2 c2);
	void dialogBox(const char *name,const char *opt,bool passive,const char *text,...);
	void merchantBox(const char *name,Trade trade,const char *opt,bool passive,const char *text,...);
	void merchantBox();
	void closeBox();
	void waitForDialog(void);

	bool pageExists(void);
	void drawPage(std::string path);

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
