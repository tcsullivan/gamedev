#ifndef UI_MENU_H_
#define UI_MENU_H_

#include <string>
#include <vector>

#include <color.hpp>
#include <config.hpp>
#include <ui.hpp>
#include <vector2.hpp>

using MenuAction = std::function<void(void)>;

class Menu;

class menuItem {
public:
	int member;
	Menu* child;

	vec2 loc;
	dim2 dim;
	Color color;
	std::string text;

	struct {
		MenuAction func;
	} button;

	struct {
		float minValue;
		float maxValue;
        float sliderLoc;
		float* var;
	} slider;

	menuItem(void) {}

	menuItem(vec2 l, dim2 d, Color c, std::string t, Menu* ch = nullptr)
		: child(ch), loc(l), dim(d), color(c), text(t) {}
};

class Menu {
public:
	std::vector<menuItem> items;
	Menu* parent;

	~Menu(void) {
		items.clear();
		parent = nullptr;
	}

	void gotoParent(void);
};

class SDLReceiver : public entityx::System<SDLReceiver>, public entityx::Receiver<SDLReceiver>
{
public:
	static bool clicked;

	void configure(entityx::EventManager& ev)
	{ ev.subscribe<MainSDLEvent>(*this); }
	void receive(const MainSDLEvent& mse);
	void update(entityx::EntityManager& en, entityx::EventManager& ev, entityx::TimeDelta dt) override 
	{ (void)en, (void)ev, (void)dt; }
};

namespace ui {
    namespace menu {
        menuItem createButton(vec2 l, dim2 d, Color c, const char* t, MenuAction f);
        menuItem createChildButton(vec2 l, dim2 d, Color c, const char* ti, Menu *_child);
        menuItem createParentButton(vec2 l, dim2 d, Color c, const char* t);
        menuItem createSlider(vec2 l, dim2 d, Color c, float min, float max, const char* t, float* v);

		void init(void);
		void toggle(void);
        void draw(void);
    }
}

#endif // UI_MENU_H_
