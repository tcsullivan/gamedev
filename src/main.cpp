#include <common.h>
#include <cstdio>
#include <ctime>

#define TICKS_PER_SEC 20
#define MSEC_PER_TICK (1000/TICKS_PER_SEC)

SDL_Window    *window = NULL;
SDL_Surface   *renderSurface = NULL;
SDL_GLContext  mainGLContext = NULL;

bool gameRunning = true;

static unsigned int tickCount   = 0,
					prevTime    = 0,
					currentTime = 0,
					deltaTime   = 0;

Entity  *entPlay;	   //The player base
Entity  *entnpc;	   //The NPC base
Player  player;		   //The actual player object
NPC     npc;		   // A test NPC
UIClass ui;			   // Handles the user interface
World   *currentWorld; // Points to the current 'world' the player is in

float interpolate(float goal, float current, float dt){
	float difference=goal-current;
	if(difference>dt)return current+dt;
	if(difference<dt)return current-dt;
	return goal;
}

void logic();
void render();

int main(int argc,char **argv){
    //runs start-up procedures
    if(!SDL_Init(SDL_INIT_VIDEO)){
    	atexit(SDL_Quit);
    	if(!(IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG)&(IMG_INIT_PNG|IMG_INIT_JPG))){
			std::cout<<"Could not init image libraries!\n"<<std::endl;
			return -1;
		}
		atexit(IMG_Quit);
		//Turn on double Buffering
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        //create the window
        window = SDL_CreateWindow("Independent Study v.0.1 alpha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
                                  #ifdef FULLSCREEN
                                  | SDL_WINDOW_FULLSCREEN
                                  #endif // FULLSCREEN
                                  );
        if(window){
        	//set OpenGL context
            mainGLContext = SDL_GL_CreateContext(window);
            if(mainGLContext == NULL){
            	std::cout << "The OpenGL context failed to initialize! Error: " << SDL_GetError() << std::endl;
            }
		}else{
			std::cout << "The window failed to generate! Error: " << SDL_GetError() << std::endl;
        	return -1;
        }
    }else{
		std::cout << "SDL was not able to initialize! Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	glClearColor(.3,.5,.8,0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	/**************************
	****     GAMELOOP      ****
	**************************/

	irand(time(NULL));

	entPlay = &player;
	entPlay->spawn(0, 0);
	entnpc = &npc;
	npc.type = -1;						 //this will make the NPC spawn the start of a village
	entnpc->spawn(.5,0);// (grand()%20)-10 ,0); //this will spawn the start of a village

	// Generate the world
	World *w=NULL,*w2=NULL;
	w2=new World(4,w,NULL);
	w=new World(2,NULL,w2);
	
	currentWorld=w;
	currentWorld->addLayer(3);
	currentWorld->addEntity((void *)entnpc);

	float gw;
	
	while(gameRunning){
		prevTime = currentTime;
		currentTime = SDL_GetTicks();
		deltaTime = currentTime - prevTime;

		if(prevTime + MSEC_PER_TICK >= SDL_GetTicks()){						//the logic loop to run at a dedicated time
			logic();
			prevTime = SDL_GetTicks();
		}

		player.loc.x += player.vel.x*deltaTime;						//update the player's x based on 

		gw=currentWorld->getWidth();
		if(player.loc.x+player.width>-1+gw){
			if(currentWorld->toRight){
				goWorldRight(currentWorld)
				player.loc.x=-1+HLINE;
			}else{
				player.loc.x=gw-1-player.width-HLINE;
			}
		}
		if(player.loc.x<-1){
			if(currentWorld->toLeft){
				goWorldLeft(currentWorld);
				player.loc.x=currentWorld->getWidth()-1-player.width-HLINE;
			}else{
				player.loc.x=-1+HLINE;
			}
		}

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
		//set the the size of the screen
		if(player.loc.x-1<-1){
			glOrtho(-1,1,-1,1,-1,1);
		}else if(player.loc.x+1>-1+currentWorld->getWidth()){
			glOrtho(-3+currentWorld->getWidth(),-1+currentWorld->getWidth(),-1,1,-1,1);
		}else{
			glOrtho(-1 + player.loc.x, 1 + player.loc.x , -1, 1, -1,1);
		}
		glMatrixMode(GL_MODELVIEW); 					//set the matrix to modelview so we can draw objects
		glPushMatrix(); 								//push the  matrix to the top of the matrix stack
		glLoadIdentity(); 								//replace the entire matrix stack with the updated GL_MODELVIEW mode
		glPushMatrix();									//basically here we put a blank canvas (new matrix) on the screen to draw on
		glClear(GL_COLOR_BUFFER_BIT); 					//clear the matrix on the top of the stack

		/**************************
		**** RENDER STUFF HERE ****
		**************************/
		 
		currentWorld->draw(); // layers dont scale x correctly...
		glColor3ub(120,30,30);							//render the player
		glRectf(player.loc.x, player.loc.y, player.loc.x + player.width, player.loc.y + player.height);

		///TEMP NPC RENDER!!!!!!
		/*glColor3ub(98, 78, 44);							//render the NPC(s)
		glRectf(npc.loc.x, npc.loc.y, npc.loc.x + .25, npc.loc.y + .25);
		glColor3ub(83, 49, 24);
		glBegin(GL_TRIANGLES);
			glVertex2f(npc.loc.x, npc.loc.y + .25);
			glVertex2f(npc.loc.x + .25, npc.loc.y + .25);
			glVertex2f(npc.loc.x + .125, npc.loc.y + .35);
		glEnd();*/
		///BWAHHHHHHHHHHHH
		
		/**************************
		****  CLOSE THE LOOP   ****
		**************************/

		glPopMatrix(); 									//take the matrix(s) off the stack to pass them to the renderer
		SDL_GL_SwapWindow(window); 						//give the stack to SDL to render it
}

void logic(){
	ui.handleEvents();								// Handle events

	if(player.right == true) {player.vel.x = .002;}
	if(player.left == true) {player.vel.x = -.002;}
	if(player.right == false && player.left == false) {player.vel.x = 0;}

	std::cout<<"\r("<<player.loc.x<<","<<player.loc.y<<")";

	currentWorld->detect(&player.loc,player.width);
	currentWorld->detect(&npc.loc,npc.height);

	tickCount++;
}	
