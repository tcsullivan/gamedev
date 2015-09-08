#include <windowClass.h>

void Window::setupRender(){
	glClearColor(0,0,0,0); //set the background color
	
}

void Window::render(){

	glMatrixMode(GL_PROJECTION); //set the matrix mode as projection so we can set the ortho size and the camera settings later on
    glPushMatrix(); //load the settings into the matrix
    glLoadIdentity(); //save the matrix

	glOrtho(0,SCREEN_WIDTH, 0,SCREEN_HEIGHT, -1,1); //set the the size of the screen

    glMatrixMode(GL_MODELVIEW); //set the matrix to modelview so we can draw objects
    glPushMatrix(); //load
    glLoadIdentity(); //save

	glPushMatrix(); //start a new matrix
	glClear(GL_COLOR_BUFFER_BIT); //clear the screen

	/**************************
	**** RENDER STUFF HERE ****
	**************************/
	glColor3f(1.0f, 0.0f, 0.0f);
	glRectf(0,0, 50,50); //draw a test rectangle
	glColor3f(0.0f, 1.0f, 0.0f);
	glRectf(50,0, 100,50);
	glColor3f(0.0f, 0.0f, 1.0f);
	glRectf(100,0,150,50);
	/**************************
	****  CLOSE THE LOOP   ****
	**************************/

	glPopMatrix(); //see ya' matrix
	SDL_GL_SwapWindow(window); //give the render to SDL to display it
}