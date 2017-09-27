#include <fileio.hpp>

#include <error.hpp>
#include <iostream>
#include <fstream>

#ifndef __WIN32__

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#else
	
#include <windows.h>

#endif // __WIN32__

int getdir(std::string dir, std::list<std::string>& files)
{
#ifndef __WIN32__
	auto dp = opendir(dir.c_str());
	UserAssert(dp != nullptr, "Couldn\'t open folder: " + dir);

	auto dirp = readdir(dp);
	while (dirp != nullptr) {
		files.emplace_back(dirp->d_name);
		dirp = readdir(dp);
	}

    closedir(dp);
#else
	WIN32_FIND_DATA fileData;
	auto dirh = FindFirstFile((dir + "/*").c_str(), &fileData);
	UserAssert(dirh != INVALID_HANDLE_VALUE, "Couldn\'t open folder: " + dir);

	do {
		auto fileName = fileData.cFileName;

		if (fileName[0] == '.')
			continue;

		if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			files.emplace_back(fileName);
    } while (FindNextFile(dirh, &fileData));

    FindClose(dirh);
#endif // __WIN32__

	files.sort();
	return 0;
}

std::string readFile(const std::string& path)
{
	std::ifstream in (path, std::ios::in);
	std::string buffer;

	UserAssert(in.is_open(), "Error reading file " + path);

	in.seekg(0, in.end);
	buffer.resize(in.tellg());
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

	UserAssert(in.is_open(), "Error reading file " + path);

	while(std::getline(in, line))
		lines.push_back(line);

	in.close();
	return lines;
}

void copyFile(const std::string& to, const std::string& from)
{
	std::ifstream src (from, std::ios::binary);
	std::ofstream dst (to, std::ios::binary);
	dst << src.rdbuf();
}

bool fileExists(const std::string& file)
{
	std::ifstream f (file);
	return f.good();
}

