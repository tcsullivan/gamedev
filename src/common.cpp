#include <common.hpp>

#include <cstdarg>
#include <cstdio>
#include <chrono>

unsigned int millis(void) {
	using namespace std::chrono;

	auto now = system_clock::now();
	return duration_cast<milliseconds>(now.time_since_epoch()).count();
}

void DEBUG_prints(const char* file, int line, const char *s,...)
{
	va_list args;
	printf("%s:%d: ", file, line);
	va_start(args, s);
	vprintf(s, args);
	va_end(args);
}

