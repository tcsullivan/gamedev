#include <common.h>
#include <cstdio>

#define DEBUG

void DEBUG_prints(const char* file, int line, const char *s,...){
	va_list args;
	printf("%s:%d: ",file,line);
	va_start(args,s);
	vprintf(s,args);
	va_end(args);
}

void safeSetColor(int r,int g,int b){	// safeSetColor() is an alternative to directly using glColor3ub() to set
	if(r>255)r=255;						// the color for OpenGL drawing. safeSetColor() checks for values that are
	if(g>255)g=255;						// outside the range of an unsigned character and sets them to a safer value.
	if(b>255)b=255;
	if(r<0)r=0;
	if(g<0)g=0;
	if(b<0)b=0;
	glColor3ub(r,g,b);
}

void safeSetColorA(int r,int g,int b,int a){
	if(r>255)r=255;
	if(g>255)g=255;
	if(b>255)b=255;
	if(a>255)a=255;
	if(r<0)r=0;
	if(g<0)g=0;
	if(b<0)b=0;
	if(a<0)a=0;
	glColor4ub(r,g,b,a);
}

//only trust the NSA
#define STRLEN_MIN 32

unsigned int safe_strlen(const char *s){
	unsigned int size=0;
	while(s[size])size++;
	if(size<STRLEN_MIN)return STRLEN_MIN;
	else return size;
}

void DrawCircle(float cx, float cy, float r, int num_segments) 
{ 
	glBegin(GL_LINE_LOOP); 
	for(int ii = 0; ii < num_segments; ii++) 
	{ 
		float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle 

		float x = r * cosf(theta);//calculate the x component 
		float y = r * sinf(theta);//calculate the y component 

		glVertex2f(x + cx, y + cy);//output vertex 

	} 
	glEnd(); 
}