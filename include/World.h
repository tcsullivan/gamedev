#ifndef WORLD_H
#define WORLD_H

#include <common.h>

#define HLINE (2.0f/(SCREEN_WIDTH/4))

class World {
private:
	struct line_t {
		// x = 2.0 (window width) / HLINES
		double start; // Where to change to dirt, going down (y)
	} *line;
	unsigned int lineCount;
public:
	World(float width);
	void draw(void);
	void detect(vec2 *v,const float width);
};

#endif // WORLD_H
