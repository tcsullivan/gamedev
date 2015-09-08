#include <windowClass.h>

void Window::setupRender(){
	glClearColor(0,0,0,0); //set the background color
	
}

void Window::render(){
								 //a matrix is a blank canvas for the computer to draw on, the matrices are stored in a "stack"
								 //GL_PROJECTION has 2 matrices
								 //GL_MODELVIEW has 32 matrices
	glMatrixMode(GL_PROJECTION); //set the matrix mode as projection so we can set the ortho size and the camera settings later on
    glPushMatrix(); //push the  matrix to the top of the matrix stack
    glLoadIdentity(); //replace the entire matrix stack with the updated GL_PROJECTION mode

	glOrtho(0,SCREEN_WIDTH, 0,SCREEN_HEIGHT, -1,1); //set the the size of the screen

    glMatrixMode(GL_MODELVIEW); //set the matrix to modelview so we can draw objects
    glPushMatrix(); //push the  matrix to the top of the matrix stack
    glLoadIdentity(); //replace the entire matrix stack with the updated GL_MODELVIEW mode

	glPushMatrix(); //basically here we put a blank canvas (new matrix) on the screen to draw on
	glClear(GL_COLOR_BUFFER_BIT); //clear the matrix on the top of the stack

	/**************************
	**** RENDER STUFF HERE ****
	**************************/
	glColor3f(1.0f, 0.0f, 0.0f); //color to red
	glRectf(0,0, 50,50); //draw a test rectangle
	glColor3f(0.0f, 1.0f, 0.0f); //color to blue
	glRectf(50,0, 100,50); //draw a test rectangle
	glColor3f(0.0f, 0.0f, 1.0f); //color to green
	glRectf(100,0,150,50); //draw a test rectangle
	/**************************
	****  CLOSE THE LOOP   ****
	**************************/

	glPopMatrix(); //take the matrix(s) off the stack to pass them to the renderer
	SDL_GL_SwapWindow(window); //give the stack to SDL to render it
}