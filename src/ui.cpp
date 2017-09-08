#include <ui.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <bmpimage.hpp>
#include <debug.hpp>
#include <error.hpp>
#include <font.hpp>
#include <ui_menu.hpp>
#include <vector3.hpp>

#include <ui_quest.hpp>
#include <brice.hpp>
#include <world.hpp>
#include <gametime.hpp>

#include <render.hpp>
#include <engine.hpp>
#include <events.hpp>
#include <window.hpp>
#include <player.hpp>

#include <chrono>

using namespace std::literals::chrono_literals;

extern Menu *currentMenu;

std::array<SDL_Keycode, 6> controlMap = {
	SDLK_w, SDLK_a, SDLK_d, SDLK_LSHIFT, SDLK_LCTRL, SDLK_e
};

void setControl(int index, SDL_Keycode key)
{
	controlMap[index] = key;
}

SDL_Keycode getControl(int index)
{
	if (index >= static_cast<int>(controlMap.size()))
		return 0;

	return controlMap[index];
}

/*
 *	Variables for dialog boxes / options.
 */

static bool typeOutDone = true;
static bool typeOutSustain = false;

static Mix_Chunk *dialogClick;

bool inBattle = false;
Mix_Chunk *battleStart;

Mix_Chunk *sanic;

static GLuint pageTex = 0;
static bool   pageTexReady = false;

namespace ui {
	/*
	 *	Mouse coordinates.
	*/

	vec2 mouse;
	vec2 premouse={0,0};

	/*
	 *	Debugging flags.
	*/

	bool debug=false;
	bool posFlag=false;
	
    void takeScreenshot(GLubyte* pixels);

	void initSounds(void) {
		dialogClick = Mix_LoadWAV("assets/sounds/click.wav");
		battleStart = Mix_LoadWAV("assets/sounds/frig.wav");
		sanic = Mix_LoadWAV("assets/sounds/sanic.wav");
	}

	void destroyFonts(void) {
		Mix_FreeChunk(dialogClick);
		Mix_FreeChunk(battleStart);
		Mix_FreeChunk(sanic);
	}

	/**
 	 * Prevents typeOut from typing the next string it's given.
	 */

	 void dontTypeOut(void) {
		 typeOutSustain = true;
	 }

	/*
	 *	Draw a string in a typewriter-esque fashion. Each letter is rendered as calls are made
	 *	to this function. Passing a different string to the function will reset the counters.
	*/

	std::string ret;
	std::string typeOut(std::string str) {
		static unsigned int tadv = 1;
		static unsigned int tickk,
							linc=0,	//	Contains the number of letters that should be drawn.
							size=0;	//	Contains the full size of the current string.

		auto tickCount = game::time::getTickCount();

		// reset values if a new string is being passed.
		if (!linc || ret.substr(0, linc) != str.substr(0, linc)) {
			tickk = tickCount + tadv;
			ret  = str.substr(0, 1);
			size = str.size();			//	Set the new target string size
			linc = 1;					//	Reset the incrementers
			if ((typeOutDone = typeOutSustain))
				typeOutSustain = false;
		}

		if (typeOutDone)
			return str;

		// Draw the next letter if necessary.
		else if (tickk <= tickCount) {
			tickk = tickCount + tadv;
			ret += str[linc];

			if (!isspace(str[linc]))
				Mix_PlayChannel(0, dialogClick, 0);

			if (linc < size) {
				switch (str[++linc]) {
				case '!':
				case '?':
				case '.':
				case ':':
					tadv = 10;
					break;
				case ',':
				case ';':
					tadv = 5;
				break;
				default:
					tadv = 1;
					break;
				}
			} else {
				typeOutDone = true;
			}
		}

		return ret;		//	The buffered string.
	}

	/*
	 *	Draw a formatted string to the specified coordinates.
	*/

	std::string uisprintf(const char *s, va_list args) {
		std::unique_ptr<char[]> buf (new char[512]);
		vsnprintf(buf.get(), 512, s, args);
		std::string ret (buf.get());
		return ret;
	}

	/*void importantText(const char *text, ...) {
		va_list args;

		dialogBoxText.clear();

		va_start(args, text);
		auto s = uisprintf(text, args);
		va_end(args);

		dialogBoxText = s;

		dialogBoxExists = true;
		dialogImportant = true;
		dialogPassive = false;
		dialogPassiveTime = 0;
	}

	void passiveImportantText(int duration, const char *text, ...) {
		va_list args;

		dialogBoxText.clear();

		va_start(args, text);
		auto s = uisprintf(text, args);
		va_end(args);

		dialogBoxText = s;

		dialogBoxExists = true;
		dialogImportant = true;
		dialogPassive = true;
		dialogPassiveTime = duration;
	}*/


	void drawPage(const GLuint& tex) {
		pageTex = tex;
		pageTexReady = true;
	}

	void drawNiceBox(vec2 c1, vec2 c2, float z) {
		drawNiceBoxColor(c1, c2, z, Color(1.0f, 1.0f, 1.0f));
	}

	void drawNiceBoxColor(vec2 c1, vec2 c2, float z, Color c) {
		// the textures for the box corners
		static Texture boxCorner  ("assets/ui/button_corners.png");
		static Texture boxSideTop ("assets/ui/button_top_bot_borders.png");
		static Texture boxSide    ("assets/ui/button_side_borders.png");

		// the dimensions of the corner textures
		static const auto& boxCornerDim  = boxCorner.getDim();
		static const auto& boxCornerDim2 = boxCornerDim / 2;

		// the amount of bytes to skip in the OpenGL arrays (see below)
		auto stride = 5 * sizeof(GLfloat);

		// we always want to make sure c1 is lower left and c2 is upper right
		if (c1.x > c2.x) std::swap(c1.x, c2.x);
		if (c1.y > c2.y) std::swap(c1.y, c2.y);

		// if the box is too small, we will not be able to draw it
		if (c2.x - c1.x < (boxCornerDim.x) || c2.y - c1.y < (boxCornerDim.y))
			return;

		GLfloat box_ul[] = {c1.x, 						c2.y - boxCornerDim2.y, z,	0.0f, 0.5f,
                          	c1.x + boxCornerDim2.x, 	c2.y - boxCornerDim2.y, z,	0.5f, 0.5f,
                          	c1.x + boxCornerDim2.x,	c2.y, 				    z,	0.5f, 1.0f,

                          	c1.x + boxCornerDim2.x,	c2.y, 					z, 0.5f, 1.0f,
                       	  	c1.x, 						c2.y, 					z, 0.0f, 1.0f,
                          	c1.x, 						c2.y - boxCornerDim2.y, z,	0.0f, 0.5f};

		GLfloat box_ll[] = {c1.x, 						c1.y, 					z,	0.0f, 0.0f,
                          	c1.x + boxCornerDim2.x, 	c1.y,					z,	0.5f, 0.0f,
                          	c1.x + boxCornerDim2.x,	c1.y + boxCornerDim2.y, z,	0.5f, 0.5f,

                          	c1.x + boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.5f,
                       	  	c1.x, 						c1.y + boxCornerDim2.y, z, 0.0f, 0.5f,
                          	c1.x, 						c1.y,					z, 0.0f, 0.0f};

		GLfloat box_ur[] = {c2.x - boxCornerDim2.x,	c2.y - boxCornerDim2.y, z,	0.5f, 0.5f,
                          	c2.x, 						c2.y - boxCornerDim2.y, z,	1.0f, 0.5f,
                          	c2.x,						c2.y, 					 z,	1.0f, 1.0f,

                          	c2.x,						c2.y, 					 z, 1.0f, 1.0f,
                       	  	c2.x - boxCornerDim2.x, 	c2.y, 					 z, 0.5f, 1.0f,
                          	c2.x - boxCornerDim2.x, 	c2.y - boxCornerDim2.y, z,	0.5f, 0.5f};

		GLfloat box_lr[] = {c2.x - boxCornerDim2.x, 	c1.y, 					 z,	0.5f, 0.0f,
                          	c2.x, 						c1.y,					 z,	1.0f, 0.0f,
                          	c2.x,						c1.y + boxCornerDim2.y, z,	1.0f, 0.5f,

                          	c2.x,						c1.y + boxCornerDim2.y, z, 1.0f, 0.5f,
                       	  	c2.x - boxCornerDim2.x, 	c1.y + boxCornerDim2.y, z, 0.5f, 0.5f,
                          	c2.x - boxCornerDim2.x, 	c1.y,					 z,	0.5f, 0.0f};

		GLfloat box_l[] =  {c1.x,						c1.y + boxCornerDim2.y, z, 0.0f, 0.0f,
							c1.x + boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.0f,
							c1.x + boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.5f, 1.0f,

							c1.x + boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.5f, 1.0f,
							c1.x,						c2.y - boxCornerDim2.y, z, 0.0f, 1.0f,
							c1.x,						c1.y + boxCornerDim2.y, z, 0.0f, 0.0f};

		GLfloat box_r[] =  {c2.x - boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.0f,
							c2.x,						c1.y + boxCornerDim2.y, z, 1.0f, 0.0f,
							c2.x,						c2.y - boxCornerDim2.y, z, 1.0f, 1.0f,

							c2.x,						c2.y - boxCornerDim2.y, z, 1.0f, 1.0f,
							c2.x - boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.5f, 1.0f,
							c2.x - boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.0f};

        GLfloat box_b[] =  {c1.x + boxCornerDim2.x,	c1.y,					 z,	0.0f, 0.0f,
							c2.x - boxCornerDim2.x,	c1.y,					 z, 1.0f, 0.0f,
							c2.x - boxCornerDim2.x,	c1.y + boxCornerDim2.y, z,	1.0f, 0.5f,

							c2.x - boxCornerDim2.x,	c1.y + boxCornerDim2.y, z,	1.0f, 0.5f,
							c1.x + boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.0f, 0.5f,
							c1.x + boxCornerDim2.x,	c1.y,					 z, 0.0f, 0.0f};

        GLfloat box_t[] =  {c1.x + boxCornerDim2.x,	c2.y - boxCornerDim2.y, z,	0.0f, 0.5f,
							c2.x - boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 1.0f, 0.5f,
							c2.x - boxCornerDim2.x,	c2.y, 					 z,	1.0f, 1.0f,

							c2.x - boxCornerDim2.x,	c2.y,					 z,	1.0f, 1.0f,
							c1.x + boxCornerDim2.x,	c2.y,					 z, 0.0f, 1.0f,
							c1.x + boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.0f, 0.5f};

		GLfloat box_f[] =  {c1.x + boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.5f,
							c2.x - boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.5f,
							c2.x - boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.5f, 0.5f,

							c2.x - boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.5f, 0.5f,
							c1.x + boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.5f, 0.5f,
							c1.x + boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.5f};

		glActiveTexture(GL_TEXTURE0);
		glUniform1f(Render::textShader.uniform[WU_texture], 0);

		Render::textShader.use();
		Render::textShader.enable();

		glUniform4f(Render::textShader.uniform[WU_tex_color], c.red, c.green, c.blue, c.alpha);
		boxCorner.use();

		// draw upper left corner
        glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_ul[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_ul[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// lower left corner
        glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_ll[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_ll[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// upper right corner
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_ur[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_ur[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// lower right corner
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_lr[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_lr[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// draw the middle of the box
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_f[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_f[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		boxSide.use();

		// draw the left edge of the box
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_l[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_l[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// draw right edge of the box
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_r[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_r[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		boxSideTop.use();

		// draw bottom of the box
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_b[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_b[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// draw top of the box
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_t[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_t[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        Render::textShader.disable();
		Render::textShader.unuse();
	}

	void draw(void){
		std::string rtext;

		auto SCREEN_HEIGHT = static_cast<float>(game::SCREEN_HEIGHT);

		// will return if not toggled
		quest::draw();

		if (pageTexReady) {

            GLfloat page_verts[] = {
				offset.x - 300, SCREEN_HEIGHT - 100, -6.0, 0, 0,
				offset.x + 300, SCREEN_HEIGHT - 100, -6.0, 1, 0,
				offset.x + 300, SCREEN_HEIGHT - 600, -6.0, 1, 1,
				offset.x + 300, SCREEN_HEIGHT - 600, -6.0, 1, 1,
				offset.x - 300, SCREEN_HEIGHT - 600, -6.0, 0, 1,
				offset.x - 300, SCREEN_HEIGHT - 100, -6.0, 0, 0,
			};

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, pageTex);
            glUniform1i(Render::textShader.uniform[WU_texture], 0);

			Render::textShader.use();
			Render::textShader.enable();

            glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), page_verts);
            glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), page_verts + 3);
            glDrawArrays(GL_TRIANGLES, 0 ,6);

            Render::textShader.disable();
			Render::textShader.unuse();

		} /* else if (dialogBoxExists) {
			rtext = typeOut(dialogBoxText);

			if (dialogImportant) {
				setFontColor(255, 255, 255);
				if (dialogPassive) {
					dialogPassiveTime -= game::time::getDeltaTime() * 12;
					if (dialogPassiveTime < 0) {
						dialogPassive = false;
						dialogImportant = false;
						dialogBoxExists = false;
					}
				}

				*if (fadeIntensity == 255 || dialogPassive) {
					setFontSize(24);
					putStringCentered(offset.x,offset.y,rtext);
					setFontSize(16);
				}*
			} else { //normal dialog box

				float y = offset.y + SCREEN_HEIGHT / 2 - HLINES(8);
				float x = offset.x - 300;

				drawNiceBox(vec2 {x, y}, vec2 {x + 600, y - SCREEN_HEIGHT / 4}, -7.0);

				setFontZ(-7.2f);
				rtext = typeOut(dialogBoxText);
				UISystem::putString(vec2(x + HLINES(2), y - fontSize - game::HLINE), rtext);

				for (i = 0; i < dialogOptText.size(); i++) {
					auto& sec = dialogOptText[i].second;

					setFontColor(255, 255, 255);
					auto tmp = putStringCentered(offset.x, sec.y,dialogOptText[i].first);
					sec.z = offset.x + tmp;
					sec.x = offset.x - tmp;
					sec.y = y - SCREEN_HEIGHT / 4 + (fontSize + game::HLINE) * (i + 1);
					if (mouse.x > sec.x && mouse.x < sec.z &&
					    mouse.y > sec.y && mouse.y < sec.y + 16) { // fontSize
						  setFontColor(255, 255, 0);
						  putStringCentered(offset.x, sec.y, dialogOptText[i].first);
					}
				}

				setFontColor(255, 255, 255);
			}

			static unsigned int rtext_oldsize = 0;
			if (rtext_oldsize != rtext.size()) {
				if (!isspace(rtext[(rtext_oldsize = rtext.size()) - 1]))
					Mix_PlayChannel(0, dialogClick, 0);

			}

		} else {
			for (const auto &s : textToDraw)
				putString(s.first.x, s.first.y, s.second);
		}*/

		//if (!fadeIntensity) {
			/*vec2 hub = {
				(SCREEN_WIDTH/2+offset.x)-fontSize*10,
				(offset.y+SCREEN_HEIGHT/2)-fontSize
			};*/

			/*putText(hub.x,hub.y,"Health: %u/%u",player->health>0?(unsigned)player->health:0,
												(unsigned)player->maxHealth
												);*/
			/*static GLuint frontHealth = Texture::genColor(Color(255,0,0));
			static GLuint backHealth = Texture::genColor(Color(150,0,0));

			if (player->isAlive()) {
				hub.y-=fontSize*1.15;

                GLfloat tex[] = {0.0, 0.0,
                                 1.0, 0.0,
                                 0.0, 1.0,
                                 1.0, 1.0};

                GLfloat back[] = {hub.x,        hub.y,      -7.0,
                                  hub.x + 150,  hub.y,      -7.0,
                                  hub.x,        hub.y + 12, -7.0,
                                  hub.x + 150,  hub.y + 12, -7.0};

                GLfloat front[] = {hub.x,       hub.y,      -7.1,
                                   hub.x + 150, hub.y,      -7.1,
                                   hub.x,       hub.y + 12, -7.1,
                                   hub.x + 150, hub.y + 12, -7.1};


                glUniform1i(Render::textShader.uniform[WU_texture], 0);

				Render::textShader.use();
				Render::textShader.enable();

                glBindTexture(GL_TEXTURE_2D, frontHealth);

                glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, front);
                glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                glBindTexture(GL_TEXTURE_2D, backHealth);

                glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, back);
                glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                Render::textShader.disable();
				Render::textShader.unuse();
            }*/

			/*
			 *	Lists all of the quests the player is currently taking.
			*/

			/*setFontColor(255,255,255,fontTransInv);
			if (player->inv->invOpen) {
				hub.y = player->loc.y + fontSize * 8;
				hub.x = player->loc.x;// + player->width / 2;

				putStringCentered(hub.x,hub.y,"Current Quests:");

				for(auto &c : player->qh.current) {
					hub.y -= fontSize * 1.15;
					putStringCentered(hub.x,hub.y,c.title);
				}

				hub.y = offset.y + 40*1.2;
				hub.x = offset.x + SCREEN_WIDTH/2 - 40*1.5;

				putStringCentered(hub.x,hub.y,"Equipment:");

				hub.y = offset.y + SCREEN_HEIGHT/2 - 20;
				hub.x = offset.x - SCREEN_WIDTH/2 + 45*4*1.5;

				putStringCentered(hub.x,hub.y,"Inventory:");
			}*/
		//	setFontColor(255,255,255,255);
		//}

		menu::draw();

		// draw the mouse
		static const Texture mouseTex ("assets/goodmouse.png");
		Render::textShader.use();
		glActiveTexture(GL_TEXTURE0);
		mouseTex.use();
		Render::useShader(&Render::textShader);
		Render::drawRect(vec2(ui::mouse.x, ui::mouse.y - 64), vec2(ui::mouse.x + 64, ui::mouse.y), -9.9);
		Render::textShader.unuse();
	}

    void takeScreenshot(GLubyte* pixels) {
		auto SCREEN_WIDTH = game::SCREEN_WIDTH;
		auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;

		time_t epoch = time(nullptr);
		struct tm* timen = localtime(&epoch);

		std::string name = "screenshots/";
		name += std::to_string(1900 + timen->tm_year) += "-";
		name += std::to_string(timen->tm_mon + 1) += "-";
		name += std::to_string(timen->tm_mday) += "_";
		name += std::to_string(timen->tm_hour) += "-";
		name += std::to_string(timen->tm_min) += "-";
		name += std::to_string(timen->tm_sec);
		name += ".bmp";
		FILE* bmp = fopen(name.c_str(), "w+");

		// unsigned long header_size = sizeof(BITMAPFILEHEADER) +
		// 							sizeof(BITMAPINFOHEADER);

		BITMAPFILEHEADER bmfh;
		BITMAPINFOHEADER bmih;

		memset(&bmfh, 0, sizeof(BITMAPFILEHEADER));
		memset(&bmih, 0, sizeof(BITMAPINFOHEADER));

		bmfh.bfType = 0x4d42;

		bmfh.bfOffBits = 54;
		bmfh.bfSize = sizeof(BITMAPFILEHEADER) +
					  sizeof(BITMAPINFOHEADER);
		bmfh.bfReserved1 = 0;
		bmfh.bfReserved2 = 0;


		bmih.biSize = sizeof(BITMAPINFOHEADER);
		bmih.biBitCount = 24;

		bmih.biClrImportant = 0;
		bmih.biClrUsed = 0;

		bmih.biCompression = 0;

		bmih.biWidth = SCREEN_WIDTH;
		bmih.biHeight = SCREEN_HEIGHT;

		bmih.biPlanes = 1;
		bmih.biSizeImage = 0;

		bmih.biXPelsPerMeter = 0x0ec4;
		bmih.biYPelsPerMeter = 0x0ec4;

		fwrite(&bmfh, 1,sizeof(BITMAPFILEHEADER),bmp);
		fwrite(&bmih, 1,sizeof(BITMAPINFOHEADER),bmp);
		fwrite(pixels, 1,3*SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(GLubyte),bmp);

		delete[] pixels;

		fclose(bmp);
	}
}

using namespace ui;

bool InputSystem::receive(const MainSDLEvent& event)
{
	const auto& e = event.event;
	auto& ev = game::events;
	
	switch (e.type) {
	// escape - quit game
	case SDL_QUIT:
		game::endGame();
		break;

	// window events - used for resizing and stuff
	case SDL_WINDOWEVENT:
		switch (e.window.event) {
		case SDL_WINDOWEVENT_RESIZED:
			std::cout << "Window " << e.window.windowID << " resized to: " << e.window.data1 << ", " << e.window.data2 << std::endl;
			auto w = e.window.data1;
			auto h = e.window.data2;
			ev.emit<WindowResizeEvent>(w,h);
			break;
		}
		break;

	// mouse movement - update mouse vector
	case SDL_MOUSEMOTION:
		premouse.x=e.motion.x;
		premouse.y=e.motion.y;
		break;

	case SDL_MOUSEBUTTONUP:
		ev.emit<MouseReleaseEvent>(mouse, e.button.button);
		break;

	case SDL_MOUSEBUTTONDOWN:
		if (UISystem::isDialog() || pageTexReady) {
			if ((e.button.button & SDL_BUTTON_RIGHT))
				UISystem::advanceDialog();
		} else {
			ev.emit<MouseClickEvent>(mouse, e.button.button);
		}
		break;

	case SDL_MOUSEWHEEL:
		ev.emit<MouseScrollEvent>(e.wheel.y);
		break;

	// key presses
	case SDL_KEYDOWN:
		ev.emit<KeyDownEvent>(SDL_KEY);
		switch(SDL_KEY){
			case SDLK_t:
				game::time::tick(100);
				break;
		}
		break;
	
	// key release
	case SDL_KEYUP:
		ev.emit<KeyUpEvent>(SDL_KEY);

		if (SDL_KEY == SDLK_ESCAPE) {
			ui::menu::toggle();
		} else if (SDL_KEY == SDLK_h) {
			quest::toggle();
		} else switch (SDL_KEY) {
		case SDLK_F3:
			debug ^= true;
			break;
		case SDLK_b:
			if (debug)
				posFlag ^= true;
			break;
		case SDLK_F12:
			// Make the BYTE array, factor of 3 because it's RBG.
			/*static GLubyte* pixels;
			pixels = new GLubyte[ 3 * SCREEN_WIDTH * SCREEN_HEIGHT];
			glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			takeScreenshot(pixels);*/

			ev.emit<ScreenshotEvent>(game::SCREEN_WIDTH, game::SCREEN_HEIGHT);

			std::cout << "Took screenshot" << std::endl;
			break;
		case SDLK_UP:
			//player->inv->setSelectionUp();
			break;
		case SDLK_DOWN:
			//player->inv->setSelectionDown();
			break;
		default:
			break;
		}
		break;
		
	default:
		break;
		
	}
	return true;
}

void InputSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;
	
	// update mouse coords
	mouse.x = premouse.x + offset.x - (game::SCREEN_WIDTH / 2);
	mouse.y = (offset.y + game::SCREEN_HEIGHT / 2) - premouse.y;
}

bool UISystem::fadeEnable = false;
bool UISystem::fadeFast = false;
int  UISystem::fadeIntensity = 0;

std::string UISystem::dialogText;
std::string UISystem::importantText;
std::vector<DialogOption> UISystem::dialogOptions;
std::string UISystem::dialogOptionResult;

void UISystem::fadeToggle(void)
{
	fadeEnable ^= true;
	fadeFast = false;

	fadeIntensity = fadeEnable ? 0 : 255;
}

void UISystem::fadeToggleFast(void)
{
	fadeEnable ^= true;
	fadeFast = true;
}

void UISystem::waitForCover(void)
{
	fadeIntensity = 0;
	while (fadeIntensity < 255)
		std::this_thread::sleep_for(1ms);
}

void UISystem::waitForUncover(void)
{
	fadeIntensity = 255;
	while (fadeIntensity > 0)
		std::this_thread::sleep_for(1ms);
}

void UISystem::putText(const vec2& p, const std::string& s, ...)
{
	va_list args;
	
	va_start(args, s);
	putString(p, uisprintf(s.c_str(), args));
	va_end(args);
}

void UISystem::putString(const vec2& p, const std::string& s, float wrap)
{
	vec2 offset = p, add;

	for (auto c : s) {
		switch (c) {
		case '\n':
			offset.y -= FontSystem::getSize() * 1.05f;
			offset.x = p.x;
			break;
		case '\b':
			offset.x -= add.x;
			break;
		case '\r':
		case '\t':
			break;
		case ' ':
			offset.x += FontSystem::getSize() / 2.0f;
			break;
		default:
			add = FontSystem::putChar(floor(offset.x), floor(offset.y), c);
			offset += add;
			break;
		}

		if (wrap != 0.12345f && offset.x >= (wrap - 10)) {
			offset.y -= FontSystem::getSize() * 1.05f;
			offset.x = p.x;
		}
	}

	//return offset.x;
}

float UISystem::putStringCentered(const vec2& p, const std::string& s, bool print)
{
	int i = 0, lastnl = 0;
	float width = 0, yy = p.y;
	auto& font = FontSystem::getFont();

	do {
		switch (s[i]) {
		case '\n':
			putString(vec2(floor(p.x - width / 2), yy), s.substr(0, i));
			lastnl = 1 + i;
			width = 0;
			yy -= FontSystem::getSize() * 1.15f;
			break;
		case '\b':
			break;
		case ' ':
			width += FontSystem::getSize() / 2;
			break;
		default:
			width += font[i].wh.x + FontSystem::getSize() * 0.1f;
			break;
		}

	} while(s[++i]);

	if (print)
		putString(vec2(floor(p.x - width / 2), yy), s.substr(lastnl));
	return width;
}

void UISystem::dialogBox(const std::string& n, const std::string& s, ...)
{
	va_list args;

	dialogText = n + ": ";

	va_start(args, s);
	dialogText += ui::uisprintf(s.c_str(), args);
	va_end(args);

	ui::ret.clear();
}

void UISystem::dialogAddOption(const std::string& o, const std::string& v)
{
	dialogOptions.emplace_back(0, 0, 0, o, v);
}

void UISystem::dialogImportant(const std::string& s)
{
	importantText = s;
	ui::ret.clear();
}

void UISystem::waitForDialog(void)
{
	while (isDialog())
		std::this_thread::sleep_for(1ms);
}

std::string UISystem::getDialogResult(void)
{
	return dialogOptionResult;
}

void UISystem::advanceDialog(void)
{
	if (!typeOutDone) {
		typeOutDone = true;
		return;
	}

	dialogText.clear();
	importantText.clear();

	if (!dialogOptions.empty()) {
		int r = 1;
		dialogOptionResult.clear();
		for (auto& o : dialogOptions) {
			if (ui::mouse.x > o.x - o.width / 2 && ui::mouse.x < o.x + o.width / 2 &&
				ui::mouse.y > o.y && ui::mouse.y < o.y + 20) {
				dialogOptionResult = o.value;
				break;
			}
			r++;
		}

		dialogOptions.clear();
	}
}

void UISystem::update(entityx::EntityManager& en, entityx::EventManager& ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;
}

void UISystem::render(void)
{
	if ((fadeEnable & (fadeIntensity < 255)))
		fadeIntensity += fadeFast ? 15 : 5;
	else if ((!fadeEnable & (fadeIntensity > 0)))
		fadeIntensity -= fadeFast ? 15 : 5;

	if (fadeIntensity < 0)
		fadeIntensity = 0;
	if (fadeIntensity > 255)
		fadeIntensity = 255;

	if (fadeIntensity != 0) {
		vec2 p1 (offset.x - game::SCREEN_WIDTH / 2, offset.y - game::SCREEN_HEIGHT / 2);
		vec2 p2 (p1.x + game::SCREEN_WIDTH, p1.y + game::SCREEN_HEIGHT);

		GLfloat backdrop[] = {
			p1.x, p1.y, -7.9, 0, 0,
			p2.x, p1.y, -7.9, 0, 0, 
			p2.x, p2.y, -7.9, 0, 0,
			p2.x, p2.y, -7.9, 0, 0,
			p1.x, p2.y, -7.9, 0, 0,
			p1.x, p1.y, -7.9, 0, 0,
		};

		Render::textShader.use();
		Render::textShader.enable();

		Colors::black.use();
		glUniform4f(Render::textShader.uniform[WU_tex_color], 1.0f, 1.0f, 1.0f, fadeIntensity / 255.0f);
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), backdrop);
		glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), backdrop + 3);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		Render::textShader.disable();
		Render::textShader.unuse();
		//setFontZ(-8.0);
	}

	if (!dialogText.empty()) {
		vec2 where (offset.x - 300, game::SCREEN_HEIGHT - 60);
		ui::drawNiceBox(vec2(where.x - 10, where.y - 200), vec2(where.x + 620, where.y + 20), -5.5f);
		FontSystem::setFontZ(-6.0f);
		putString(where, ui::typeOut(dialogText), where.x + 600);

		if (!dialogOptions.empty()) {
			float y = where.y - 180;
			for (auto& o : dialogOptions) {
				o.x = offset.x;
				o.y = y;
				o.width = putStringCentered(vec2(o.x, o.y), o.text, false);
				y += 20;

				if (ui::mouse.x > o.x - o.width / 2 && ui::mouse.x < o.x + o.width / 2 &&
					ui::mouse.y > o.y && ui::mouse.y < y)
					FontSystem::setFontColor(255, 255, 0);

				putStringCentered(vec2(o.x, o.y), o.text);
				FontSystem::setFontColor(255, 255, 255);
			}
		}
	}

	if (!importantText.empty()) {
		FontSystem::setFontSize(24);
		FontSystem::setFontZ(-9.0f);
		putStringCentered(vec2(offset.x, 400), ui::typeOut(importantText));
		FontSystem::setFontZ(-6.0f);
		FontSystem::setFontSize(16);
	}
}
