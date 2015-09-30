#include <common.h>
#include <world.h>
#include <ui.h>
#include <entities.h>
#include <cstdio>
#include <chrono>

#define TICKS_PER_SEC 20
#define MSEC_PER_TICK (1000/TICKS_PER_SEC)

SDL_Window    *window = NULL;
SDL_Surface   *renderSurface = NULL;
SDL_GLContext  mainGLContext = NULL;

bool gameRunning = true;
unsigned int tickCount   = 0,
			 prevTime    = 0,
			 currentTime = 0,
			 deltaTime   = 0;

World *currentWorld=NULL;
Player *player;

std::vector<Entity*>entity;
std::vector<NPC>npc;
std::vector<Structures>build;

int mx, my;
FILE* names;

void logic();
void render();

int entityInteractTest(NPC *speaker){
	player->qh.assign("Test");
	return 1;
}

unsigned int millis(void){
	std::chrono::system_clock::time_point now=std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

int main(int argc, char *argv[]){
	// Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO)){
		std::cout << "SDL was not able to initialize! Error: " << SDL_GetError() << std::endl;
		return -1;
	}
    atexit(SDL_Quit);
    // Initialize SDL_image
    if(!(IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG)&(IMG_INIT_PNG|IMG_INIT_JPG))){
		std::cout<<"Could not init image libraries!\n"<<std::endl;
		return -1;
	}
	atexit(IMG_Quit);
	// Turn on double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // Create the window
    window = SDL_CreateWindow("Independent Study v.0.2 alpha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
                              #ifdef FULLSCREEN
                              | SDL_WINDOW_FULLSCREEN
                              #endif // FULLSCREEN
                              );
    if(window==NULL){
		std::cout << "The window failed to generate! Error: " << SDL_GetError() << std::endl;
		std::cout << "Window address: "<<window<<std::endl;
        return -1;
    }
    //set OpenGL context
    mainGLContext = SDL_GL_CreateContext(window);
    if(mainGLContext == NULL){
		std::cout << "The OpenGL context failed to initialize! Error: " << SDL_GetError() << std::endl;
        return -1;
    }

	ui::initFonts();
	ui::setFontFace("ttf/VCR_OSD_MONO_1.001.ttf");
	ui::setFontSize(16);
	initRand(millis()); // fix

	glViewport(0,0,SCREEN_WIDTH, SCREEN_HEIGHT);
	glClearColor(.3,.5,.8,0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	SDL_ShowCursor(SDL_DISABLE);
	
	/**************************
	****     GAMELOOP      ****
	**************************/

	//************************************************************************//
	//	WORLD GENERATION STUFF												  //
	//************************************************************************//

	names = fopen("assets/names_en-us", "r+");
	// Make a world
	World *test=new World();
	test->generate(SCREEN_WIDTH/2);
	test->addLayer(400);
	test->addLayer(100);
	test->addPlatform(150,100,100,10);
	currentWorld=test;
	
	IndoorWorld *iw=new IndoorWorld();
	iw->generate(200);
	
	// Make the player
	player=new Player();
	player->spawn(0,100);
	
	// Make structures
	entity.push_back(new Entity());
	build.push_back(Structures());
	entity[0]=&build[0];
	
	static unsigned int i;
	build[0].spawn(STRUCTURET,0,10);
	build[0].inside=iw;
	for(i=0;i<entity.size()+1;i++){
		entity[i]->inWorld=test;
	}
	
	NPCp(entity[1])->addAIFunc(entityInteractTest);
	
	//************************************************************************//
	//	END WORLD GENERATION STUFF											  //
	//************************************************************************//
	currentTime=millis();
	while(gameRunning){
		prevTime = currentTime;
		currentTime = millis();
		deltaTime = currentTime - prevTime;
		
		if(prevTime + MSEC_PER_TICK >= millis()){
			logic();
			prevTime = millis();
		}

		player->loc.y+=player->vel.y*deltaTime;
		player->loc.x+=(player->vel.x*player->speed)*deltaTime;
		render();
	}
	
	/**************************
	****  CLOSE PROGRAM    ****
	**************************/
	
    //closes the window and frees resources
    SDL_GL_DeleteContext(mainGLContext);
    SDL_DestroyWindow(window);
    return 0;
}

void render(){
													//a matrix is a blank canvas for the computer to draw on, the matrices are stored in a "stack"
													//GL_PROJECTION has 2 matrices
													//GL_MODELVIEW has 32 matrices
	glMatrixMode(GL_PROJECTION); 					//set the matrix mode as projection so we can set the ortho size and the camera settings later on
	glPushMatrix(); 								//push the  matrix to the top of the matrix stack
	glLoadIdentity(); 								//replace the entire matrix stack with the updated GL_PROJECTION mode
	glOrtho(player->loc.x-SCREEN_WIDTH/2,player->loc.x+SCREEN_WIDTH/2,0,SCREEN_HEIGHT,-1,1);
	glMatrixMode(GL_MODELVIEW); 					//set the matrix to modelview so we can draw objects
	glPushMatrix(); 								//push the  matrix to the top of the matrix stack
	glLoadIdentity(); 								//replace the entire matrix stack with the updated GL_MODELVIEW mode
	glPushMatrix();									//basically here we put a blank canvas (new matrix) on the screen to draw on
	glClear(GL_COLOR_BUFFER_BIT); 					//clear the matrix on the top of the stack

	/**************************
	**** RENDER STUFF HERE ****
	**************************/

	currentWorld->draw(&player->loc);	// Draw the world around the player
	glColor3ub(0,0,0);
	player->draw();						// Draw the player

	if(ui::debug){
		static unsigned int debugDiv=0;
		static int fps,d;
		static float rndy; //variable to round the player y-coord so it is easier to read
		if(++debugDiv==20){
			fps=1000/deltaTime;
			d=deltaTime;
			debugDiv=0;
		}else if(debugDiv%10==0){
			rndy = player->loc.y;
		}
		ui::putText(player->loc.x-SCREEN_WIDTH/2,SCREEN_HEIGHT-ui::fontSize,"FPS: %d\nD: %d G:%d\nRes: %ux%u\nE: %d\nPOS: (x)%+.2f\n     (y)%+.2f",
					fps,d,player->ground,SCREEN_WIDTH,SCREEN_HEIGHT,entity.size(),player->loc.x,rndy);
	}

	ui::draw();							// Draw any UI elements if they need to be

	for(int i=0;i<=entity.size();i++){
		//entity[i]->draw();
		entity[i]->loc.x += entity[i]->vel.x * deltaTime;
		entity[i]->loc.y += entity[i]->vel.y * deltaTime;
	}

	/**************************
	****  CLOSE THE LOOP   ****
	**************************/
	mx = ui::mouse.x + player->loc.x;
	my = ui::mouse.y;
	my = 720 - my;
	mx -= (SCREEN_WIDTH/2);
	glColor3ub(255,255,255);
	glBegin(GL_TRIANGLES);
		glVertex2i(mx,my);
		glVertex2i(mx+HLINE*3.5,my);
		glVertex2i(mx,my-HLINE*3.5);
	glEnd();

	ui::putText(player->loc.x-SCREEN_WIDTH/2,SCREEN_HEIGHT/2,"Quest count: %d",player->qh.current.size());

	glPopMatrix(); 									//take the matrix(s) off the stack to pass them to the renderer
	SDL_GL_SwapWindow(window); 						//give the stack to SDL to render it
}

void logic(){
	ui::handleEvents();
	currentWorld->detect(player);
	for(int i=0;i<=entity.size();i++){
		if(entity[i]->alive&&entity[i]->type == NPCT){
			entity[i]->wander((rand()%120 + 30), &entity[i]->vel);
			if( pow((entity[i]->loc.x - player->loc.x),2) + pow((entity[i]->loc.y - player->loc.y),2) <= pow(40*HLINE,2)){
				if(mx >= entity[i]->loc.x && mx <= entity[i]->loc.x + entity[i]->width && my >= entity[i]->loc.y && my <= entity[i]->loc.y + entity[i]->width
				 && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)))
					entity[i]->interact();
			}
		}
	}
}
