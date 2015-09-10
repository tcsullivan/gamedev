#ifndef WORLD_H
#define WORLD_H

#include <common.h>

#define goWorldLeft(w)  if(w->toLeft){w=w->toLeft;}
#define goWorldRight(w) if(w->toRight){w=w->toRight;}

class World {
private:
	struct line_t {
		// x = 2.0 (window width) / HLINES
		double start; // Where to change to dirt, going down (y)
	} *line;
	unsigned int lineCount;
public:
	World *toLeft,*toRight;
	World(void);
	World(const float width,World *l,World *r);
	void draw(void);
	void detect(vec2 *v,const float width);
};

#endif // WORLD_H
