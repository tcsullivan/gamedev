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

#endif // __WIN32__

unsigned int millis(void) {
	std::chrono::system_clock::time_point now=std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}


vec2 str2coord(std::string s)
{
	auto cpos = s.find(',');
	s[cpos] = '\0';
	return vec2 (std::stof(s), std::stof(s.substr(cpos + 1)));
}

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
#ifndef __WIN32__
    DIR *dp;
    struct dirent *dirp;
    if (!(dp = opendir(dir.c_str()))) {
        std::cout <<"Error ("<<errno<<") opening "<<dir<<std::endl;
        return errno;
    }
    while((dirp = readdir(dp)))
        files.push_back(std::string(dirp->d_name));
    closedir(dp);
#else
	HANDLE dirh;
	WIN32_FIND_DATA file_data;

	if ((dirh = FindFirstFile((dir + "/*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
        return -1; /* No files found */

    do {
        const std::string file_name = file_data.cFileName;
        const std::string full_file_name = dir + file_name;
        const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        if (file_name[0] == '.')
            continue;

        if (is_directory)
            continue;

        files.push_back(file_name);
    } while (FindNextFile(dirh, &file_data));

    FindClose(dirh);
#endif // __WIN32__
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

	if (!in.is_open()) {
//		UserError("Error reading file " + (std::string)path + "!");
		return nullptr;
	}

	in.seekg(0,in.end);
	buf = new GLchar[(size = in.tellg()) + 1];
	in.seekg(0,in.beg);
	in.read(buf,size);
	buf[size] = '\0';

	in.close();
	return buf;
}

std::string readFile(const std::string& path)
{
	std::ifstream in (path, std::ios::in);
	std::string buffer;

	if (!in.is_open())
		UserError("Error reading file " + path);

	in.seekg(0, in.end);
	buffer.reserve(in.tellg());
	in.seekg(0, in.beg);
	in.read(&buffer[0], buffer.size());

	in.close();
	return buffer;
}

std::vector<std::string> readFileA(const std::string& path)
{
	std::ifstream in (path, std::ios::in);
	std::vector<std::string> lines;
	std::string line;

	if (!in.is_open())
		UserError("Error reading file " + path);

	while(std::getline(in, line))
		lines.push_back(line);

	in.close();
	return lines;
}

void UserError(std::string reason)
{
    std::cout << "User error: " << reason << "!\n";
    abort();
}
