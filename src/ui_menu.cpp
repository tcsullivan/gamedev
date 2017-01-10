#include <ui_menu.hpp>

#include <engine.hpp>
#include <render.hpp>
#include <texture.hpp>

#include <fstream>

extern Menu *currentMenu;

static Menu pauseMenu;
static Menu optionsMenu;
static Menu controlsMenu;

void Menu::gotoParent(void)
{
	if (!parent) {
		currentMenu = nullptr;
		game::config::update();
	} else {
		currentMenu = parent;
	}
}

inline void segFault(void)
{
	++*((int *)0);
}

void quitGame(void)
{
	game::config::update();
	game::config::save();
	game::endGame();
}

std::string& deleteWord(std::string& s)
{
	while (s.back() != ' ')
		s.pop_back();

	return s;
}

std::string sym2str(const SDL_Keycode& c)
{
	std::string s = "";

	switch (c) {
	case SDLK_UP     : s = "UP"      ; break;
	case SDLK_DOWN   : s = "DOWN"    ; break;
	case SDLK_LEFT   : s = "LEFT"    ; break;
	case SDLK_RIGHT  : s = "RIGHT"   ; break;
	case SDLK_LSHIFT : s = "LSHIFT"  ; break;
	case SDLK_RSHIFT : s = "RSHIFT"  ; break;
	case SDLK_LALT   : s = "LALT"    ; break;
	case SDLK_RALT   : s = "RALT"    ; break;
	case SDLK_LCTRL  : s = "LCONTROL"; break;
	case SDLK_RCTRL  : s = "RCONTROL"; break;
	case SDLK_TAB    : s = "TAB"     ; break;
	default          : s += static_cast<char>(c); break;
	}

	return s;
}

void initControls(Menu *m)
{
	auto cfg = readFileA("config/controls.dat");
	unsigned i = 0;
	SDL_Keycode z;

	for (const auto &l : cfg) {
		z = static_cast<SDL_Keycode>(std::stoi(l));
		setControl(i, z);
		m->items[i++].text += sym2str(z);
	}
}

void saveControls(void)
{
	std::ofstream out ("config/controls.dat");
	SDL_Keycode q = 1;
	unsigned int i = 0;

	while ((q = getControl(i++)) != 0) {
		auto d = std::to_string(q) + '\n';
		out.write(d.data(), d.size());
	}

	out.close();
}
void setControlF(unsigned int index, menuItem &m)
{
	SDL_Event e;

	do SDL_WaitEvent(&e);
	while (e.type != SDL_KEYDOWN);

	setControl(index, e.key.keysym.sym);
	deleteWord(m.text);

	m.text += sym2str(e.key.keysym.sym);
	saveControls();
}

namespace ui {
    namespace menu {
        menuItem createButton(vec2 l, dim2 d, Color c, const char* t, menuFunc f) {
            menuItem temp;

            temp.member = 0;
            temp.loc = l;
            temp.dim = d;
            temp.color = c;
            temp.text = t;
            temp.button.func = f;
			temp.child = nullptr;

            return temp;
        }

        menuItem createChildButton(vec2 l, dim2 d, Color c, const char* t, Menu *_child) {
            menuItem temp;

            temp.member = -1;
            temp.loc = l;
            temp.dim = d;
            temp.color = c;
            temp.text = t;
            temp.button.func = nullptr;
			temp.child = _child;

            return temp;
        }

        menuItem createParentButton(vec2 l, dim2 d, Color c, const char* t) {
            menuItem temp;

            temp.member = -2;
            temp.loc = l;
            temp.dim = d;
            temp.color = c;
            temp.text = t;
            temp.button.func = nullptr;
			temp.child = nullptr;

            return temp;
        }

        menuItem createSlider(vec2 l, dim2 d, Color c, float min, float max, const char* t, float* v) {
            menuItem temp;

            temp.member = 1;
            temp.loc = l;
            temp.dim = d;
            temp.color = c;
            temp.slider.minValue = min;
            temp.slider.maxValue = max;
            temp.text = t;
            temp.slider.var = v;
            temp.slider.sliderLoc = *v;
			temp.child = nullptr;

            return temp;
        }

		void init(void) {
			// Create the main pause menu
			pauseMenu.items.push_back(ui::menu::createParentButton({-128,100},{256,75},{0.0f,0.0f,0.0f}, "Resume"));
			pauseMenu.items.push_back(ui::menu::createChildButton({-128, 0},{256,75},{0.0f,0.0f,0.0f}, "Options", &optionsMenu));
			pauseMenu.items.push_back(ui::menu::createChildButton({-128,-100},{256,75},{0.0f,0.0f,0.0f}, "Controls", &controlsMenu));
			pauseMenu.items.push_back(ui::menu::createButton({-128,-200},{256,75},{0.0f,0.0f,0.0f}, "Save and Quit", quitGame));
			pauseMenu.items.push_back(ui::menu::createButton({-128,-300},{256,75},{0.0f,0.0f,0.0f}, "Segfault", segFault));

			// Create the options (sound) menu
			optionsMenu.items.push_back(ui::menu::createSlider({0-static_cast<float>(game::SCREEN_WIDTH)/4,0-(512/2)}, {50,512}, {0.0f, 0.0f, 0.0f}, 0, 100, "Master", &game::config::VOLUME_MASTER));
			optionsMenu.items.push_back(ui::menu::createSlider({-200,100}, {512,50}, {0.0f, 0.0f, 0.0f}, 0, 100, "Music", &game::config::VOLUME_MUSIC));
			optionsMenu.items.push_back(ui::menu::createSlider({-200,000}, {512,50}, {0.0f, 0.0f, 0.0f}, 0, 100, "SFX", &game::config::VOLUME_SFX));
			optionsMenu.parent = &pauseMenu;

			// Create the controls menu
			controlsMenu.items.push_back(ui::menu::createButton({-450,300}, {400, 75}, {0.0f, 0.0f, 0.0f}, "Up: ", nullptr));
			controlsMenu.items.back().button.func = [](){ setControlF(0, controlsMenu.items[0]); };
			controlsMenu.items.push_back(ui::menu::createButton({-450,200}, {400, 75}, {0.0f, 0.0f, 0.0f}, "Left: ", nullptr));
			controlsMenu.items.back().button.func = [](){ setControlF(1, controlsMenu.items[1]); };
			controlsMenu.items.push_back(ui::menu::createButton({-450,100}, {400, 75}, {0.0f, 0.0f, 0.0f}, "Right: ", nullptr));
			controlsMenu.items.back().button.func = [](){ setControlF(2, controlsMenu.items[2]); };
			controlsMenu.items.push_back(ui::menu::createButton({-450,0}, {400, 75}, {0.0f, 0.0f, 0.0f}, "Sprint: ", nullptr));
			controlsMenu.items.back().button.func = [](){ setControlF(3, controlsMenu.items[3]); };
			controlsMenu.items.push_back(ui::menu::createButton({-450,-100}, {400, 75}, {0.0f, 0.0f, 0.0f}, "Creep: ", nullptr));
			controlsMenu.items.back().button.func = [](){ setControlF(4, controlsMenu.items[4]); };
			controlsMenu.items.push_back(ui::menu::createButton({-450,-200}, {400, 75}, {0.0f, 0.0f, 0.0f}, "Inventory: ", nullptr));
			controlsMenu.items.back().button.func = [](){ setControlF(5, controlsMenu.items[5]); };
			controlsMenu.parent = &pauseMenu;
			initControls(&controlsMenu);
		}

		void toggle(void) {
			currentMenu = &pauseMenu;
		}

        void draw(void) {
			auto SCREEN_WIDTH = game::SCREEN_WIDTH;
			auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;

            SDL_Event e;

			bool clicked = false;

			Render::useShader(&Render::textShader);

            setFontSize(24);
            game::config::update();
			setFontZ(-9.0);
	
            mouse.x = ui::premouse.x+offset.x-(SCREEN_WIDTH/2);
            mouse.y = (offset.y+SCREEN_HEIGHT/2)-ui::premouse.y;

            //custom event polling for menu's so all other events are ignored
            while(SDL_PollEvent(&e)) {
                switch (e.type) {
                case SDL_QUIT:
                    game::endGame();
                    return;
                    break;
                case SDL_MOUSEMOTION:
                    premouse.x=e.motion.x;
                    premouse.y=e.motion.y;
                    break;
				case SDL_MOUSEBUTTONUP:
					if (e.button.button & SDL_BUTTON_LEFT) {
						clicked = true;
					}
					break;	
                case SDL_KEYUP:
                    if (SDL_KEY == SDLK_ESCAPE) {
                        currentMenu->gotoParent();
                        return;
                    }
                    break;
                default:break;
                }
            }

			static float cMult = 1.0f;
			static const ColorTex back (Color(0, 0, 0, 204));

			//draw the dark transparent background
			Render::textShader.use();

			back.use();
			Render::drawRect(vec2(offset.x - SCREEN_WIDTH / 2 - 1, offset.y - (SCREEN_HEIGHT / 2)),
			                 vec2(offset.x + SCREEN_WIDTH / 2, offset.y + (SCREEN_HEIGHT / 2)), -8.5);

			Render::textShader.unuse();

            //loop through all elements of the menu
            for (auto &m : currentMenu->items) {
				// reset the background modifier
				cMult = 1.0f;

				vec2 loc (offset.x + m.loc.x, offset.y + m.loc.y);
				vec2 end (loc.x + m.dim.x, loc.y + m.dim.y);

				//if the menu is any type of button
                if (m.member == 0 || m.member == -1 || m.member == -2) {
                    //tests if the mouse is over the button
                    if ((mouse.x >= loc.x && mouse.x <= end.x) && (mouse.y >= loc.y && mouse.y <= end.y)) {
						// set the darkness multiplier
						cMult = 0.6f;

                        //if the mouse is over the button and clicks
                        if (clicked) {
                            switch(m.member) {
                                case 0: //normal button
                                    m.button.func();
                                    break;
                                case -1:
                                    currentMenu = m.child;
                                    break;
                                case -2:
                                    currentMenu->gotoParent();
                                default:break;
                            }
                        }
                    }

					ui::drawNiceBoxColor(loc, end, -8.6, Color(cMult, cMult, cMult, 1.0f));
                    //draw the button text
                    putStringCentered(loc.x + (m.dim.x / 2),
                    	loc.y + (m.dim.y / 2) - (ui::fontSize / 2),
                        m.text);

					//if element is a slider
                } else if (m.member == 1) {
                    //combining slider text with variable amount
                    char outSV[32];
                    sprintf(outSV, "%s: %.1f",m.text.c_str(), *m.slider.var);
	
                    float sliderW = m.dim.x, sliderH = m.dim.y;
                    m.slider.sliderLoc = m.slider.minValue + (*m.slider.var / m.slider.maxValue);

                    if (sliderH > sliderW) {
                        sliderH *= 0.05f;
                        //location of the slider handle
                        m.slider.sliderLoc *= m.dim.y - sliderW;
                    } else {
                        sliderW *= 0.05f;
                        //location of the slider handle
                        m.slider.sliderLoc *= m.dim.x - sliderW;
                    }

					ui::drawNiceBoxColor(loc, end, -8.6, Color(.5f, .5f, .5f, 1.0f));

                    //test if mouse is inside of the slider's borders
                    if ((mouse.x >= loc.x && mouse.x <= end.x) && (mouse.y >= loc.y && mouse.y <= end.y)) {
						// change multiplier background modifier
						cMult = 0.75f;

						//if we are inside the slider and click it will set the slider to that point
                        if (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                            //change handle location
                            if (m.dim.y > m.dim.x)
                                *m.slider.var = ((mouse.y-offset.y) - m.loc.y)/m.dim.y * 100;
                            else
                                *m.slider.var = ((mouse.x-offset.x) - m.loc.x)/m.dim.x * 100;

							cMult = 0.5f;
                        }

                        //makes sure handle can't go below or above min and max values
						*m.slider.var = std::clamp(*m.slider.var, m.slider.minValue, m.slider.maxValue);
                    }

					//draw the slider handle
					if (m.dim.y > m.dim.x) {
                        ui::drawNiceBoxColor(vec2(loc.x, loc.y + (m.slider.sliderLoc * 1.05)),
                        	vec2(loc.x + sliderW, loc.y + (m.slider.sliderLoc * 1.05) + sliderH), -8.7, Color(cMult, cMult, cMult, 1.0f));

                        //draw the now combined slider text
                        putStringCentered(loc.x + (m.dim.x/2), (loc.y + (m.dim.y*1.05)) - ui::fontSize/2, outSV);
                    } else {
                        ui::drawNiceBoxColor(vec2(loc.x+m.slider.sliderLoc, loc.y),
                            vec2(loc.x + m.slider.sliderLoc + sliderW, loc.y + sliderH), -8.7, Color(cMult, cMult, cMult, 1.0f));

                        //draw the now combined slider text
                        putStringCentered(loc.x + (m.dim.x/2), (loc.y + (m.dim.y/2)) - ui::fontSize/2, outSV);
                    }
                }
            }
            setFontSize(16);
			setFontZ(-8.0);
        }


    }
}
