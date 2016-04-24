#ifndef UI_MENU_H_
#define UI_MENU_H_

#include <common.hpp>
#include <config.hpp>
#include <ui.hpp>

typedef void (*menuFunc)(void);

class menuItem {
public:
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
	~menuItem(){
		//button.text = NULL;
		//slider.text = NULL;

		//delete[] button.text;
		//delete[] slider.text;
		//delete slider.var;
	}
};

class Menu {
public:
	std::vector<menuItem> items;
	Menu *parent, *child;

	~Menu()
	{
		items.clear();
		//delete child;
		//delete parent;
		child = NULL;
		parent = NULL;
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

		void init(void);
		void toggle(void);
        void draw(void);
    }
}

#endif // UI_MENU_H_
