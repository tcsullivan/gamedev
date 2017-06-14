#ifndef COMPONENTS_FLASH_HPP_
#define COMPONENTS_FLASH_HPP_

#include <color.hpp>

struct Flash
{
	Flash(Color c, int _ms = 500)
		: color(c), ms(_ms), totalMs(_ms) {}

	Color color;
	int ms, totalMs;
};

#endif // COMPONENTS_FLASH_HPP_
