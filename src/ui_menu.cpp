#include <ui_menu.hpp>

#include <common.hpp>
#include <engine.hpp>
#include <fileio.hpp>
#include <gametime.hpp>
#include <render.hpp>
#include <texture.hpp>
#include <font.hpp>

#include <fstream>

static Menu* currentMenu = nullptr;

bool SDLReceiver::receive(const MainSDLEvent& mse)
{
	if (currentMenu == nullptr)
		return true;

	switch (mse.event.type) {
	case SDL_QUIT:
		game::endGame();
		break;
	case SDL_MOUSEBUTTONUP:
		if (mse.event.button.button & SDL_BUTTON_LEFT)
			clicked = true;
		break;
	case SDL_MOUSEBUTTONDOWN:
		break; // consume events	
	case SDL_KEYUP:
		if (currentMenu != nullptr && mse.event.key.keysym.sym == SDLK_ESCAPE) {
			currentMenu->gotoParent();
		} else {
			clicked = false;
		}
		break;
	case SDL_KEYDOWN:
		break; // consume events
	default:
		return true;
		break;
	}

	return false;
}

bool SDLReceiver::clicked = false;

extern vec2 offset;

static Menu pauseMenu;
static Menu optionsMenu;
static Menu controlsMenu;

void Menu::gotoParent(void)
{
	if (parent == nullptr) {
		game::config::update();
		FontSystem::setFontSize(FontSystem::SizeSmall);
		game::time::togglePause(false);
	}
	
	currentMenu = parent;
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
        menuItem createButton(vec2 l, dim2 d, Color c, const char* t, MenuAction f) {
            menuItem temp (l, d, c, t);

            temp.member = 0;
            temp.button.func = f;

            return temp;
        }

        menuItem createChildButton(vec2 l, dim2 d, Color c, const char* t, Menu* _child) {
            menuItem temp (l, d, c, t, _child);

            temp.member = -1;
            temp.button.func = nullptr;

            return temp;
        }

        menuItem createParentButton(vec2 l, dim2 d, Color c, const char* t) {
            menuItem temp (l, d, c, t);

            temp.member = -2;
            temp.button.func = nullptr;

            return temp;
        }

        menuItem createSlider(vec2 l, dim2 d, Color c, float min, float max, const char* t, float* v) {
            menuItem temp (l, d, c, t);

            temp.member = 1;
            temp.slider.minValue = min;
            temp.slider.maxValue = max;
            temp.slider.var = v;
            temp.slider.sliderLoc = *v;

            return temp;
        }

		void init(void) {
			dim2 obSize (256, 75);
			Color black (0, 0, 0);
			// Create the main pause menu
			pauseMenu.items.push_back(ui::menu::createParentButton(
				vec2(-128, 150), obSize, black, "Resume"));

			pauseMenu.items.push_back(ui::menu::createChildButton(
				vec2(-128, 50), obSize, black, "Options", &optionsMenu));

			pauseMenu.items.push_back(ui::menu::createChildButton(
				vec2(-128, -50), obSize, black, "Controls", &controlsMenu));

			pauseMenu.items.push_back(ui::menu::createButton(
				vec2(-128, -150), obSize, black, "Save and Quit",
				[]() {
					game::config::update(); // TODO necessary?
					game::config::save();
					game::endGame();
				} ));

			pauseMenu.items.push_back(ui::menu::createButton(
				vec2(-128, -250), obSize, black, "Segfault",
				[]() { ++*((int *)0); } ));

			// Create the options (sound) menu
			optionsMenu.items.push_back(ui::menu::createSlider(
				vec2(-static_cast<float>(game::SCREEN_WIDTH) / 4, -(512/2)), dim2(50, 512),
				black, 0, 100, "Master", &game::config::VOLUME_MASTER));
			optionsMenu.items.push_back(ui::menu::createSlider(
				vec2(-200, 100), dim2(512, 50), black, 0, 100, "Music", &game::config::VOLUME_MUSIC));
			optionsMenu.items.push_back(ui::menu::createSlider(
				vec2(-200, 0), dim2(512, 50), black, 0, 100, "SFX", &game::config::VOLUME_SFX));

			optionsMenu.parent = &pauseMenu;

			// Create the controls menu
			dim2 cbSize (400, 75);
			controlsMenu.items.push_back(ui::menu::createButton(
				vec2(-450, 300), cbSize, black, "Up: ",
				[]() { setControlF(0, controlsMenu.items[0]); } ));

			controlsMenu.items.push_back(ui::menu::createButton(
				vec2(-450, 200), cbSize, black, "Left: ",
				[]() { setControlF(1, controlsMenu.items[1]); } ));

			controlsMenu.items.push_back(ui::menu::createButton(
				vec2(-450, 100), cbSize, black, "Right: ",
				[]() { setControlF(2, controlsMenu.items[2]); } ));

			controlsMenu.items.push_back(ui::menu::createButton(
				vec2(-450, 0), cbSize, black, "Sprint: ",
				[]() { setControlF(3, controlsMenu.items[3]); } ));

			controlsMenu.items.push_back(ui::menu::createButton(
				vec2(-450, -100), cbSize, black, "Creep: ",
				[]() { setControlF(4, controlsMenu.items[4]); } ));

			controlsMenu.items.push_back(ui::menu::createButton(
				vec2(-450, -200), cbSize, black, "Inventory: ",
				[]() { setControlF(5, controlsMenu.items[5]); } ));

			controlsMenu.parent = &pauseMenu;

			initControls(&controlsMenu);
		}

		void toggle(void) {
			currentMenu = &pauseMenu;
			if (currentMenu != nullptr)
				game::time::togglePause(true);
		}

        void draw(void) {
			float z = Render::ZRange::Menu;

			if (currentMenu == nullptr)
				return;

			auto& SCREEN_WIDTH = game::SCREEN_WIDTH;
			auto& SCREEN_HEIGHT = game::SCREEN_HEIGHT;

			Render::useShader(&Render::textShader);

            game::config::update();
            FontSystem::setFontSize(FontSystem::SizeLarge);
			FontSystem::setFontZ(z - 0.03f);
	
            mouse.x = ui::premouse.x+offset.x-(SCREEN_WIDTH/2);
            mouse.y = (offset.y+SCREEN_HEIGHT/2)-ui::premouse.y;

            //custom event polling for menu's so all other events are ignored
			static float cMult = 1.0f;
			static const ColorTex back (Color(0, 0, 0, 204));

			//draw the dark transparent background
			Render::textShader.use();

			back.use();
			Render::drawRect(vec2(offset.x - SCREEN_WIDTH / 2 - 1, offset.y - (SCREEN_HEIGHT / 2)),
			    vec2(offset.x + SCREEN_WIDTH / 2, offset.y + (SCREEN_HEIGHT / 2)), z);

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
                        if (SDLReceiver::clicked) {
                            switch (m.member) {
                            case 0: //normal button
                                m.button.func();
                                break;
                            case -1:
                                currentMenu = m.child;
                                break;
                            case -2:
                                currentMenu->gotoParent();
                            default:
								break;
                            }

                        }
                    }

					ui::drawNiceBoxColor(loc, end, z - 0.01f, Color(cMult, cMult, cMult, 1.0f));
                    //draw the button text
                    UISystem::putStringCentered(vec2(loc.x + (m.dim.x / 2),
                    	loc.y + (m.dim.y / 2) - (FontSystem::getSize() / 2)),
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

					ui::drawNiceBoxColor(loc, end, z - 0.02f, Color(.5f, .5f, .5f, 1.0f));

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
                        UISystem::putStringCentered(vec2(loc.x + (m.dim.x/2), (loc.y + (m.dim.y*1.05)) - FontSystem::getSize() / 2), outSV);
                    } else {
                        ui::drawNiceBoxColor(vec2(loc.x+m.slider.sliderLoc, loc.y),
                            vec2(loc.x + m.slider.sliderLoc + sliderW, loc.y + sliderH), -8.7, Color(cMult, cMult, cMult, 1.0f));

                        //draw the now combined slider text
                        UISystem::putStringCentered(loc + (m.dim / 2) /*- FontSystem::getSize() / 2*/, outSV);
                    }
                }
            }

			SDLReceiver::clicked = false;
            FontSystem::setFontSize(FontSystem::SizeSmall);
			FontSystem::setFontZ();
        }


    }
}
