#ifndef ERROR_HPP_
#define ERROR_HPP_

#include <string>
#include <iostream>

inline void UserError(const std::string& why)
{
	std::cout << "User error: " << why << "!\n";
	abort();
}

#endif // ERROR_HPP_
