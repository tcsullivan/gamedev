#ifndef WORLD_H
#define WORLD_H

#include <common.h>
#include <cstring>

#define LAYER0_Y (-0.8f)
#define TEX_SIZE ( 0.2f)

class World {
private:
	struct layer_t {
		unsigned int tex;
		float offset; 
	} layer[4];
public:
	World(const char *l1,const char *l2,const char *l3,const char *bg);
	void draw(void);
	void update(int player_accel);
};

#endif // WORLD_H
