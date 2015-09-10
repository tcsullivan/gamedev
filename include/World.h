#ifndef WORLD_H
#define WORLD_H

#include <common.h>

#define HLINE (2.0f/ 200 )

class World {
private:
	struct line_t {
		// x = 2.0 (window width) / HLINES
		float start; // Where to change to dirt, going down (y)
	} *line;
	unsigned int lineCount;
public:
	World(float width);
	void draw(void);
};

#endif // WORLD_H
