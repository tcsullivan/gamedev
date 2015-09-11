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

Entities *entit1;
Player player;
UIClass ui;
World *currentWorld;

//static int randNext=1;

void irand(unsigned int seed){
	srand(seed);
}

int grand(void){
	return rand();
}

unsigned int logic(unsigned int interval,void *param);

float interpolate(float goal, float current, float dt){
	float difference = goal - current;
	if(difference > dt){
		return current + dt;}
	if(difference < dt){
		return current - dt;}
	return goal;
}

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
	SDL_AddTimer(MSEC_PER_TICK,logic,NULL);
	glClearColor(.3,.5,.8,0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	/**************************
	****     GAMELOOP      ****
	**************************/

	entit1 = &player;
	entit1->spawn(0, 0);

	// Generate the world
	World *w=NULL,*w2=NULL;
	w2=new World(4,w,NULL);
	w=new World(2,NULL,w2);
	
	currentWorld=w;
	
	// Save the world if necessary
	/*FILE *f=fopen("world.dat","r");
	unsigned int fSave;
	if(!f){
		f=fopen("world.dat","w");
		if(f){
			fSave=time(NULL);
			fwrite(&fSave,sizeof(unsigned int),1,f);
			fclose(f);
		}
	}else{
		fread(&fSave,sizeof(unsigned int),1,f);
		fclose(f);
	}*/
	irand(time(NULL));
>>>>>>> Stashed changes
	
	float gw;
	
	while(gameRunning){
		prevTime = currentTime;
		currentTime = tickCount;
		deltaTime = currentTime - prevTime;

		gw=currentWorld->getWidth();
		if(player.loci.x+player.width>-1+gw){
			if(currentWorld->toRight){
				goWorldRight(currentWorld)
				player.loci.x=-1+HLINE;
			}else{
				player.loci.x=gw-1-player.width-HLINE;
			}
		}
		if(player.loci.x<-1){
			if(currentWorld->toLeft){
				goWorldLeft(currentWorld);
				player.loci.x=currentWorld->getWidth()-1-player.width-HLINE;
			}else{
				player.loci.x=-1+HLINE;
			}
		}

		player.vel.x = interpolate(player.velg.x, player.vel.x, deltaTime) * .005;
		if(player.vel.x > .05) player.vel.x = .05;
		if(player.vel.x < -.05) player.vel.x = -.05;
		player.loci.x += player.vel.x;
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
		glOrtho(-1 + player.loci.x, 1 + player.loci.x , -1, 1, -1,1); //set the the size of the screen
		glMatrixMode(GL_MODELVIEW); 					//set the matrix to modelview so we can draw objects
		glPushMatrix(); 								//push the  matrix to the top of the matrix stack
		glLoadIdentity(); 								//replace the entire matrix stack with the updated GL_MODELVIEW mode
		glPushMatrix();									//basically here we put a blank canvas (new matrix) on the screen to draw on
		glClear(GL_COLOR_BUFFER_BIT); 					//clear the matrix on the top of the stack

		/**************************
		**** RENDER STUFF HERE ****
		**************************/
		 
		currentWorld->draw();
		glColor3ub(120,30,30);
		glRectf(player.loci.x, player.loci.y, player.loci.x + player.width, player.loci.y + player.height);
		
		/**************************
		****  CLOSE THE LOOP   ****
		**************************/

		glPopMatrix(); 									//take the matrix(s) off the stack to pass them to the renderer
		SDL_GL_SwapWindow(window); 						//give the stack to SDL to render it
}

unsigned int logic(unsigned int interval,void *param){
	ui.handleEvents();								// Handle events

	player.vel.x=0;
	currentWorld->detect(&player.loci,player.width);

	//std::cout << player.vel.x << std::endl;
	//std::cout << player.velg.y << std::endl;
	//std::cout << "d:" << deltaTime << std::endl;

	tickCount++;
	return interval;
}	
