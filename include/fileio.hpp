#ifndef FILEIO_HPP_
#define FILEIO_HPP_

#include <string>
#include <list>
#include <vector>

// reads the names of files in a directory into the given string vector
int getdir(std::string dir, std::list<std::string>& files);

// reads the given file into a buffer and returns a pointer to the buffer
std::string readFile(const std::string& path);
std::vector<std::string> readFileA(const std::string& path);

void copyFile(const std::string& to, const std::string& from);

bool fileExists(const std::string& file);

#endif // FILEIO_HPP_
