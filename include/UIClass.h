#ifndef UICLASS_H
#define UICLASS_H

#include <common.h>
#include <cstdarg>
#include <cstdio>

class UIClass {
private:
	unsigned int fontSize;
public:
	void init(const char *ttf);
	void setFontSize(unsigned int fs);
	void putText(const float x,const float y,const char *s,...);
	void putString(const float x,const float y,const char *s);
	void handleEvents();
};

#endif // UICLASS_H
