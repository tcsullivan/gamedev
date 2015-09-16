#ifndef UICLASS_H
#define UICLASS_H

#include <common.h>
#include <cstdarg>
#include <cstdio>

namespace ui {
	extern int mousex,mousey;
	void init(const char *ttf);
	void setFontSize(unsigned int fs);
	void putText(const float x,const float y,const char *s,...);
	void putString(const float x,const float y,const char *s);
	void msgBox(const char *str,...);
	void handleEvents();
};

#endif // UICLASS_H
