#include <common.hpp>

#include <cstring>
#include <cstdio>
#include <chrono>
#include <fstream>
#include <sstream>

#ifndef __WIN32__

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>

#include <texture.hpp>

unsigned int millis(void) {
	std::chrono::system_clock::time_point now=std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

#endif // __WIN32__

std::vector<std::string> StringTokenizer(const std::string& str, char delim)
{
	std::vector<std::string> tokens;
	std::istringstream is (str);
	std::string token;

	while (getline(is, token, delim))
		tokens.push_back(token);

	return tokens;
}

void DEBUG_prints(const char* file, int line, const char *s,...)
{
	va_list args;
	printf("%s:%d: ", file, line);
	va_start(args, s);
	vprintf(s, args);
	va_end(args);
}

void safeSetColor(int r, int g, int b)
{
	r = static_cast<int>(fmax(fmin(r, 255), 0));
	g = static_cast<int>(fmax(fmin(g, 255), 0));
	b = static_cast<int>(fmax(fmin(b, 255), 0));
	glColor3ub(r, g, b);
}

void safeSetColorA(int r,int g,int b,int a) {
	r = static_cast<int>(fmax(fmin(r, 255), 0));
	g = static_cast<int>(fmax(fmin(g, 255), 0));
	b = static_cast<int>(fmax(fmin(b, 255), 0));
	a = static_cast<int>(fmax(fmin(a, 255), 0));
	glColor4ub(r, g, b, a);
}

int getdir(std::string dir, std::vector<std::string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if (!(dp = opendir(dir.c_str()))) {
        std::cout <<"Error ("<<errno<<") opening "<<dir<<std::endl;
        return errno;
    }
    while((dirp = readdir(dp)))
        files.push_back(std::string(dirp->d_name));
    closedir(dp);
    return 0;
}

void strVectorSortAlpha(std::vector<std::string> *v)
{
	static bool change;
	do {
		change = false;
		for (unsigned int i=0; i < v->size() - 1; i++) {
			if (v[0][i] > v[0][i + 1]) {
				std::swap(v[0][i], v[0][i + 1]);
				change = true;
			}
		}
	} while (change);
}

const char *readFile(const char *path)
{
	std::ifstream in (path,std::ios::in);
	unsigned int size;
	GLchar *buf;

	if (!in.is_open())
		UserError("Error reading file " + (std::string)path + "!");

	in.seekg(0,in.end);
	buf = new GLchar[(size = in.tellg()) + 1];
	in.seekg(0,in.beg);
	in.read(buf,size);
	buf[size] = '\0';

	in.close();
	return buf;
}

void UserError(std::string reason)
{
    std::cout << "User error: " << reason << "!\n";
    abort();
}
