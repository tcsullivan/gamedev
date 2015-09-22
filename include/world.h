#ifndef WORLD_H
#define WORLD_H

#include <common.h>

class World {
private:
	struct line_t {
		float y;
		unsigned char color;
	} __attribute__ ((packed)) *line;
	unsigned int lineCount;
	int x_start;
	World *behind,*infront;
public:
	World *toLeft,*toRight;
	World(unsigned int width);
	~World(void);
	
	void addLayer(unsigned int width);
	void draw(vec2 *vec);
	void detect(vec2 *v,vec2 *vel,const float width);
	
	World *goWorldLeft(vec2 *loc,const float width);
	World *goWorldRight(vec2 *loc,const float width);
	World *goWorldBack(vec2 *loc,const float width);
	World *goWorldFront(vec2 *loc,const float width);
};

#endif // WORLD_H
