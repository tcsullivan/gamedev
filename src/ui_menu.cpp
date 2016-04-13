#include <ui_menu.hpp>

extern bool gameRunning;

extern Menu *currentMenu;
extern Menu pauseMenu;

void Menu::
gotoParent(void)
{
	if (!parent) {
		currentMenu = NULL;
		config::update();
	} else
		currentMenu = parent;
}

void Menu::
gotoChild(void)
{
	currentMenu = child;
}

namespace ui {
    namespace menu {
        menuItem createButton(vec2 l, dim2 d, Color c, const char* t, menuFunc f){
            menuItem temp;
            
            temp.member = 0;
            temp.button.loc = l;
            temp.button.dim = d;
            temp.button.color = c;
            temp.button.text = t;
            temp.button.func = f;

            return temp;
        }

        menuItem createChildButton(vec2 l, dim2 d, Color c, const char* t){
            menuItem temp;
            
            temp.member = -1;
            temp.button.loc = l;
            temp.button.dim = d;
            temp.button.color = c;
            temp.button.text = t;
            temp.button.func = NULL;

            return temp;
        }

        menuItem createParentButton(vec2 l, dim2 d, Color c, const char* t){
            menuItem temp;

            temp.member = -2;
            temp.button.loc = l;
            temp.button.dim = d;
            temp.button.color = c;
            temp.button.text = t;
            temp.button.func = NULL;

            return temp;
        }

        menuItem createSlider(vec2 l, dim2 d, Color c, float min, float max, const char* t, float* v){
            menuItem temp;

            temp.member = 1;
            temp.slider.loc = l;
            temp.slider.dim = d;
            temp.slider.color = c;
            temp.slider.minValue = min;
            temp.slider.maxValue = max;
            temp.slider.text = t;
            temp.slider.var = v;
            temp.slider.sliderLoc = *v;

            return temp;
        }

        void draw(void) {
            SDL_Event e;
            
            setFontSize(24);
            config::update();

            mouse.x = ui::premouse.x+offset.x-(SCREEN_WIDTH/2);
            mouse.y = (offset.y+SCREEN_HEIGHT/2)-ui::premouse.y;

            //custom event polling for menu's so all other events are ignored
            while(SDL_PollEvent(&e)){
                switch(e.type){
                case SDL_QUIT:
                    gameRunning = false;
                    return;
                    break;
                case SDL_MOUSEMOTION:
                    premouse.x=e.motion.x;
                    premouse.y=e.motion.y;
                    break;
                case SDL_KEYUP:
                    if(SDL_KEY == SDLK_ESCAPE){
                        currentMenu->gotoParent();
                        return;
                    }
                    break;
                default:break;
                }
            }

            //draw the dark transparent background
            glColor4f(0.0f, 0.0f, 0.0f, .8f);
            glRectf(offset.x-SCREEN_WIDTH/2,0,offset.x+SCREEN_WIDTH/2,SCREEN_HEIGHT);

            //loop through all elements of the menu
            for(auto &m : currentMenu->items){
                //if the menu is any type of button
                if(m.member == 0 || m.member == -1 || m.member == -2){

                    //draw the button background
                    glColor3f(m.button.color.red,m.button.color.green,m.button.color.blue);
                    glRectf(offset.x+m.button.loc.x,
                            offset.y+m.button.loc.y,
                            offset.x+m.button.loc.x + m.button.dim.x,
                            offset.y+m.button.loc.y + m.button.dim.y);
                    //draw the button text
                    putStringCentered(offset.x + m.button.loc.x + (m.button.dim.x/2),
                                      (offset.y + m.button.loc.y + (m.button.dim.y/2)) - ui::fontSize/2,
                                      m.button.text);

                    //tests if the mouse is over the button
                    if(mouse.x >= offset.x+m.button.loc.x && mouse.x <= offset.x+m.button.loc.x + m.button.dim.x){
                        if(mouse.y >= offset.y+m.button.loc.y && mouse.y <= offset.y+m.button.loc.y + m.button.dim.y){

                            //if the mouse if over the button, it draws this white outline
                            glColor3f(1.0f,1.0f,1.0f);
                            glBegin(GL_LINE_STRIP);
                                glVertex2f(offset.x+m.button.loc.x, 					offset.y+m.button.loc.y);
                                glVertex2f(offset.x+m.button.loc.x+m.button.dim.x, 		offset.y+m.button.loc.y);
                                glVertex2f(offset.x+m.button.loc.x+m.button.dim.x, 		offset.y+m.button.loc.y+m.button.dim.y);
                                glVertex2f(offset.x+m.button.loc.x, 					offset.y+m.button.loc.y+m.button.dim.y);
                                glVertex2f(offset.x+m.button.loc.x, 					offset.y+m.button.loc.y);
                            glEnd();

                            //if the mouse is over the button and clicks
                            if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)){
                                switch(m.member){
                                    case 0: //normal button
                                        m.button.func();
                                        break;
                                    case -1:
                                        currentMenu->gotoChild();
                                        break;
                                    case -2:
                                        currentMenu->gotoParent();
                                    default:break;
                                }
                            }
                        }
                    }

                    //if element is a slider
                }else if(m.member == 1){
                    //combining slider text with variable amount
                    char outSV[32];
                    sprintf(outSV, "%s: %.1f",m.slider.text, *m.slider.var);

                    float sliderW, sliderH;

                    if(m.slider.dim.y > m.slider.dim.x){
                        //width of the slider handle
                        sliderW = m.slider.dim.x;
                        sliderH = m.slider.dim.y * .05;
                        //location of the slider handle
                        m.slider.sliderLoc = m.slider.minValue + (*m.slider.var/m.slider.maxValue)*(m.slider.dim.y-sliderW);
                    }else{
                        //width of the slider handle
                        sliderW = m.slider.dim.x * .05;
                        sliderH = m.slider.dim.y;
                        //location of the slider handle
                        m.slider.sliderLoc = m.slider.minValue + (*m.slider.var/m.slider.maxValue)*(m.slider.dim.x-sliderW);
                    }
                    //draw the background of the slider
                    glColor4f(m.slider.color.red,m.slider.color.green,m.slider.color.blue, .5f);
                    glRectf(offset.x+m.slider.loc.x,
                            offset.y+m.slider.loc.y,
                            offset.x+m.slider.loc.x + m.slider.dim.x,
                            offset.y+m.slider.loc.y + m.slider.dim.y);

                    //draw the slider handle
                    glColor4f(m.slider.color.red,m.slider.color.green,m.slider.color.blue, 1.0f);
                    if(m.slider.dim.y > m.slider.dim.x){
                        glRectf(offset.x+m.slider.loc.x,
                            offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05),
                            offset.x+m.slider.loc.x + sliderW,
                            offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05) + sliderH);

                        //draw the now combined slider text
                        putStringCentered(offset.x + m.slider.loc.x + (m.slider.dim.x/2), (offset.y + m.slider.loc.y + (m.slider.dim.y*1.05)) - ui::fontSize/2, outSV);
                    }else{
                        glRectf(offset.x+m.slider.loc.x+m.slider.sliderLoc,
                                offset.y+m.slider.loc.y,
                                offset.x+m.slider.loc.x + m.slider.sliderLoc + sliderW,
                                offset.y+m.slider.loc.y + sliderH);

                        //draw the now combined slider text
                        putStringCentered(offset.x + m.slider.loc.x + (m.slider.dim.x/2), (offset.y + m.slider.loc.y + (m.slider.dim.y/2)) - ui::fontSize/2, outSV);
                    }
                    //test if mouse is inside of the slider's borders
                    if(mouse.x >= offset.x+m.slider.loc.x && mouse.x <= offset.x+m.slider.loc.x + m.slider.dim.x){
                        if(mouse.y >= offset.y+m.slider.loc.y && mouse.y <= offset.y+m.slider.loc.y + m.slider.dim.y){

                            //if it is we draw a white border around it
                            glColor3f(1.0f,1.0f,1.0f);
                            glBegin(GL_LINE_STRIP);
                                glVertex2f(offset.x+m.slider.loc.x, 					offset.y+m.slider.loc.y);
                                glVertex2f(offset.x+m.slider.loc.x+m.slider.dim.x, 		offset.y+m.slider.loc.y);
                                glVertex2f(offset.x+m.slider.loc.x+m.slider.dim.x, 		offset.y+m.slider.loc.y+m.slider.dim.y);
                                glVertex2f(offset.x+m.slider.loc.x, 					offset.y+m.slider.loc.y+m.slider.dim.y);
                                glVertex2f(offset.x+m.slider.loc.x, 					offset.y+m.slider.loc.y);

                                if(m.slider.dim.y > m.slider.dim.x){
                                    //and a border around the slider handle
                                    glVertex2f(offset.x+m.slider.loc.x, 		  offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05));
                                    glVertex2f(offset.x+m.slider.loc.x + sliderW, offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05));
                                    glVertex2f(offset.x+m.slider.loc.x + sliderW, offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05) + sliderH);
                                    glVertex2f(offset.x+m.slider.loc.x,           offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05) + sliderH);
                                    glVertex2f(offset.x+m.slider.loc.x,           offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05));
                                }else{
                                    //and a border around the slider handle
                                    glVertex2f(offset.x+m.slider.loc.x + m.slider.sliderLoc, offset.y+m.slider.loc.y);
                                    glVertex2f(offset.x+m.slider.loc.x + (m.slider.sliderLoc + sliderW), offset.y+m.slider.loc.y);
                                    glVertex2f(offset.x+m.slider.loc.x + (m.slider.sliderLoc + sliderW), offset.y+m.slider.loc.y+m.slider.dim.y);
                                    glVertex2f(offset.x+m.slider.loc.x + m.slider.sliderLoc, offset.y+m.slider.loc.y+m.slider.dim.y);
                                    glVertex2f(offset.x+m.slider.loc.x + m.slider.sliderLoc, offset.y+m.slider.loc.y);
                                }

                            glEnd();

                            //if we are inside the slider and click it will set the slider to that point
                            if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)){
                                //change handle location
                                if(m.slider.dim.y > m.slider.dim.x){
                                    *m.slider.var = (((mouse.y-offset.y) - m.slider.loc.y)/m.slider.dim.y)*100;
                                    //draw a white box over the handle
                                    glColor3f(1.0f,1.0f,1.0f);
                                    glRectf(offset.x+m.slider.loc.x,
                                            offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05),
                                            offset.x+m.slider.loc.x + sliderW,
                                            offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05) + sliderH);

                                }else{
                                    *m.slider.var = (((mouse.x-offset.x) - m.slider.loc.x)/m.slider.dim.x)*100;
                                    //draw a white box over the handle
                                    glColor3f(1.0f,1.0f,1.0f);
                                    glRectf(offset.x+m.slider.loc.x + m.slider.sliderLoc,
                                            offset.y+m.slider.loc.y,
                                            offset.x+m.slider.loc.x + (m.slider.sliderLoc + sliderW),
                                            offset.y+m.slider.loc.y + m.slider.dim.y);
                                }
                            }

                            //makes sure handle can't go below or above min and max values
                            if(*m.slider.var >= m.slider.maxValue)*m.slider.var = m.slider.maxValue;
                            else if(*m.slider.var <= m.slider.minValue)*m.slider.var = m.slider.minValue;
                        }
                    }
                }
            }
            setFontSize(16);
        }


    }
}
