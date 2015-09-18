#ifndef UI_H
#define UI_H

#include <common.h>
#include <cstdarg>

namespace ui {
	void initFonts(void);
	void setFontFace(const char *ttf);
	void setFontSize(unsigned int size);
	
	void putString(const float x,const float y,const char *s);
	void putText(const float x,const float y,const char *str,...);
	
	void handleEvents(void);
}

#endif // UI_H
