#ifndef UI_MENU_H_
#define UI_MENU_H_

#include <common.hpp>
#include <config.hpp>
#include <ui.hpp>

typedef void (*menuFunc)(void);

class Menu;

class menuItem {
public:
	int member;
	Menu *child;

	vec2 loc;
	dim2 dim;
	Color color;
	std::string text;
	//union {
		struct {
			menuFunc func;
		} button;

		struct {
			float minValue;
			float maxValue;
            float sliderLoc;
			float *var;
		} slider;
	//};

	menuItem(){}
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
	Menu *parent;

	~Menu()
	{
		items.clear();
		parent = NULL;
	}

	void gotoParent(void);
};

namespace ui {
    namespace menu {
        menuItem createButton(vec2 l, dim2 d, Color c, const char* t, menuFunc f);
        menuItem createChildButton(vec2 l, dim2 d, Color c, const char* ti, Menu *_child);
        menuItem createParentButton(vec2 l, dim2 d, Color c, const char* t);
        menuItem createSlider(vec2 l, dim2 d, Color c, float min, float max, const char* t, float* v);

		void init(void);
		void toggle(void);
        void draw(void);
    }
}

#endif // UI_MENU_H_
