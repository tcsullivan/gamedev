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
public:
	World(unsigned int width);
	~World(void);
	void draw(vec2 *vec);
	void detect(vec2 *v,vec2 *vel,const float width);
};

#endif // WORLD_H
