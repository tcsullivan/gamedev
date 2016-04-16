#ifndef UI_MENU_H_
#define UI_MENU_H_

#include <common.hpp>
#include <config.hpp>
#include <ui.hpp>

typedef void (*menuFunc)(void);

struct menuItem {
	int member;
	union {
		struct {
			vec2 loc;
			dim2 dim;
			Color color;

			const char *text;
			menuFunc func;
		} button;
		struct {
			vec2 loc;
			dim2 dim;
			Color color;

			float minValue;
			float maxValue;
            float sliderLoc;

			const char *text;
			float *var;
		} slider;
	};
};

class Menu {
public:
	std::vector<menuItem> items;
	Menu *child, *parent;

	~Menu() {
        // TODO you CANNOT delete null pointers!
		/*child = NULL;
		parent = NULL;
		delete child;
		delete parent;*/
	}

	void gotoChild(void);
	void gotoParent(void);
};

namespace ui {
    namespace menu {
        menuItem createButton(vec2 l, dim2 d, Color c, const char* t, menuFunc f);
        menuItem createChildButton(vec2 l, dim2 d, Color c, const char* t);
        menuItem createParentButton(vec2 l, dim2 d, Color c, const char* t);
        menuItem createSlider(vec2 l, dim2 d, Color c, float min, float max, const char* t, float* v);

        void draw(void);
    }
}

#endif // UI_MENU_H_
