#include <common.h>
#include <ctime>

SDL_Window    *window = NULL;
SDL_Surface   *renderSurface = NULL;
SDL_GLContext  mainGLContext = NULL;

bool gameRunning = true;

UIClass ui;
Entities *entit1;
Player player;
World *currentWorld;

int main(int argc,char **argv){
    // Initialize SDL
    if(!SDL_Init(SDL_INIT_VIDEO)){
    	atexit(SDL_Quit);
    }else{
		std::cout << "SDL was not able to initialize! Error: " << SDL_GetError() << std::endl;
		return -1;
	}
	// Initialize SDL_image
	if((IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG)&(IMG_INIT_PNG|IMG_INIT_JPG))){
		atexit(IMG_Quit);
	}else{
		std::cout<<"Could not init image libraries!\n"<<std::endl;
		return -1;
	}
	// Create the window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    window = SDL_CreateWindow("Independent Study v.0.1 alpha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
                              #ifdef FULLSCREEN
                              | SDL_WINDOW_FULLSCREEN
                              #endif // FULLSCREEN
                              );
	if(!window){
		std::cout << "The window failed to generate! Error: " << SDL_GetError() << std::endl;
		return -1;
	}
	// Set OpenGL context
	if((mainGLContext = SDL_GL_CreateContext(window))==NULL){
		std::cout << "The OpenGL context failed to initialize! Error: " << SDL_GetError() << std::endl;
	}
	// Setup rand() and OpenGL
	srand(time(NULL));
	glClearColor(.3,.5,.8,0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	/**************************
	****     GAMELOOP      ****
	**************************/

	entit1 = &player;
	entit1->spawn(0,0);

	World *w=NULL;
	World *w2=new World(4,w,NULL);
	w=new World(2,NULL,w2);
	currentWorld=w;
	
	while(gameRunning){
		ui.handleEvents();								// Handle events
														//a matrix is a blank canvas for the computer to draw on, the matrices are stored in a "stack"
														//GL_PROJECTION has 2 matrices
														//GL_MODELVIEW has 32 matrices
		glMatrixMode(GL_PROJECTION); 					//set the matrix mode as projection so we can set the ortho size and the camera settings later on
		glPushMatrix(); 								//push the  matrix to the top of the matrix stack
		glLoadIdentity(); 								//replace the entire matrix stack with the updated GL_PROJECTION mode
		glOrtho(-1,1,-1,1,-1,1);						//set the the size of the screen
		glMatrixMode(GL_MODELVIEW); 					//set the matrix to modelview so we can draw objects
		glPushMatrix(); 								//push the  matrix to the top of the matrix stack
		glLoadIdentity(); 								//replace the entire matrix stack with the updated GL_MODELVIEW mode
		glPushMatrix();									//basically here we put a blank canvas (new matrix) on the screen to draw on
		glClear(GL_COLOR_BUFFER_BIT); 					//clear the matrix on the top of the stack

		/**************************
		**** RENDER STUFF HERE ****
		**************************/
		 
		currentWorld->draw();
		glColor3ub(0,0,0);
		glRectf(player.loc.x, player.loc.y, player.loc.x + player.width, player.loc.y + player.height);
		
		/**************************
		****  CLOSE THE LOOP   ****
		**************************/

		glPopMatrix(); 									//take the matrix(s) off the stack to pass them to the renderer
		SDL_GL_SwapWindow(window); 						//give the stack to SDL to render it
	}
	
	/**************************
	****  CLOSE PROGRAM    ****
	**************************/
	
    //closes the window and frees resources
    SDL_GL_DeleteContext(mainGLContext);
    SDL_DestroyWindow(window);
    return 0;
}
