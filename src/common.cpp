#include <common.h>
#include <cstring>
#include <cstdio>
#include <chrono>

#ifndef __WIN32__
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#endif // __WIN32__

#ifndef __WIN32__

unsigned int millis(void){
	std::chrono::system_clock::time_point now=std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

#endif // __WIN32__

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

int getdir(const char *dir, std::vector<std::string> &files){
    DIR *dp;
    struct dirent *dirp;
    if(!(dp = opendir(dir))){
        std::cout <<"Error ("<<errno<<") opening "<<dir<<std::endl;
        return errno;
    }
    while((dirp = readdir(dp)))
        files.push_back(std::string(dirp->d_name));
    closedir(dp);
    return 0;
}

void strVectorSortAlpha(std::vector<std::string> *v){
	static bool change;
	do{
		change = false;
		for(unsigned int i=0;i<v->size()-1;i++){
			if(v[0][i] > v[0][i+1]){
				std::swap(v[0][i],v[0][i+1]);
				change = true;
			}
		}
	}while(change);
}

const char *readFile(const char *path){
	std::ifstream in (path,std::ios::in);
	unsigned int size;
	GLchar *buf;
	
	if(!in.is_open()){
		std::cout<<"Error reading file "<<path<<"!"<<std::endl;
		abort();
	}
	
	in.seekg(0,in.end);
	buf = new GLchar[(size = in.tellg()) + 1];
	in.seekg(0,in.beg);
	in.read(buf,size);
	buf[size] = '\0';
	
	in.close();
	return buf;
}

void
UserError( std::string reason )
{
    std::cout << "User error: " << reason << "!" << std::endl;
    abort();
}

/*int strCreateFunc(const char *equ){
	static unsigned int size;
	static char *filebuf;
	static FILE *file;
	
	size = 57 + strlen(equ) + 3;
	
	filebuf = new char[size];
	memset(filebuf,0,size);
	
	strcpy(filebuf,"#include <stdio.h>\n#include <math.h>\nint main(){return ");
	strcat(filebuf,equ);
	strcat(filebuf,";}");
	
	if(!(file = fopen("gen.tmp","w"))){
		abort();
	}
	
	fwrite(filebuf,size,sizeof(char),file);
	delete[] filebuf;
	fclose(file);
	
	system("
	
	return 0;
}*/
