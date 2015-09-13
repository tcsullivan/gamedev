#ifndef UICLASS_H
#define UICLASS_H

#include <common.h>

class UIClass {
	public:
		void init(const char *ttf);
		void setFontSize(unsigned int fs);
		void putText(float x,float y,const char *s);
		void handleEvents();
};

#endif // UICLASS_H
