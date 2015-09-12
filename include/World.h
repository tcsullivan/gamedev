#ifndef WORLD_H
#define WORLD_H

#include <common.h>

#define goWorldLeft(w)  if(w->toLeft){w=w->toLeft;}
#define goWorldRight(w) if(w->toRight){w=w->toRight;}

#define LAYER_SCALE 1

class World {
private:
	struct line_t {
		// x = 2.0 (window width) / HLINES
		double start; // Where to change to dirt, going down (y)
	} __attribute__ ((packed)) *line;
	unsigned int lineCount;
	bool root;
public:
	World *behind,*infront;
	World *toLeft,*toRight;
	World(void);
	World(const float width,World *l,World *r);
	void draw(void);
	void detect(vec2 *v,const float width);
	float getWidth(void);
	void saveToFile(FILE *f,World *parent);
	void loadFromFile(FILE *f,World *parent);
	void addLayer(void);
	void setRoot(void);
};

#endif // WORLD_H
