#ifndef ERROR_HPP_
#define ERROR_HPP_

#include <string>
#include <iostream>

#define UserError(w) _UserError(__FILE__, __LINE__, w)
#define UserAssert(c, e) if (!(c)) { UserError(e); }

inline void _UserError(const char* file, int line, const std::string& why)
{
	std::cout << file << ':' << line << ": error: " << why << "!\n";
	abort();
}

#endif // ERROR_HPP_
