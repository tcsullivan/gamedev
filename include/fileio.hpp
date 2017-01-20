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

#endif // FILEIO_HPP_
