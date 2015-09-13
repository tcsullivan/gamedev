#ifndef WORLD_H
#define WORLD_H

#include <common.h>

// Total amount of entities that can be bound to a layer
#define MAX_ENTITIES 16

// Easy shortcuts used in UIClass
#define goWorldLeft(w)  if(w->toLeft){w=w->toLeft;}
#define goWorldRight(w) if(w->toRight){w=w->toRight;}

class World {
private:
	struct line_t {
		double start; 				  // Where land begins, going down (i.e. y)
	} __attribute__ ((packed)) *line;
	unsigned int lineCount;			  // Size of line array, calculated in the constructor
	unsigned int entCount;			  // Count of currently bound entities
	void *entity[MAX_ENTITIES];
public:
	World *behind,*infront;							  // As in layers
	World *toLeft,*toRight;							  // 'new' worlds (off screen)
	World(void);									  // Creates an empty world
	World(const float width,World *l,World *r);		  // Creates a legit world
	void draw(void);								  // Draws the world as well as any bound entities
	void detect(vec2 *v,vec2 *vel,const float width); // Object / gravity detection
	float getWidth(void);							  // Get coordinate width of world
	void saveToFile(FILE *f,World *parent);			  // WIP: Save the world (seed) to a file?
	void loadFromFile(FILE *f,World *parent);		  // No
	void addLayer(const float width);				  // Creates a layer of said width behind the current one
	void addEntity(void *e);						  // Adds (binds) an entity to the world
};

#endif // WORLD_H
