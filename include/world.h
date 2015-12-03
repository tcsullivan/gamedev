#ifndef WORLD_H
#define WORLD_H

#include <common.h>		// For HLINE, vec2, OpenGL utilities, etc.
#include <entities.h>

#define GEN_INC 10		// Defines at what interval y values should be calculated for the array 'line'.
						// As explained in World(), the last few lines in the array 'line' are incorrectly calculated
						// or not calculated at all, so GEN_INC is also used to decrease 'lineCount' in functions like draw()
						// and detect().

#define DAY_CYCLE 3000

typedef enum {
	BG_FOREST,
	BG_WOODHOUSE
} WORLD_BG_TYPE;

typedef enum {
	SUNNY = 0,
	DARK,
	RAIN
} WEATHER;

struct line_t {
	bool gs;
	float y,gh[2];
	unsigned char color;
} __attribute__ ((packed));

/*
 *	World - creates and handles an area of land
*/

class World {
protected:
	
	/*
	 *	struct line_t
	 * 
	 *	The world is stored in an array of lines. Example:
	 * 
	 *		 	||       
	 *			|||  || |
	 *	 		|||||||||
	 * line no. 123456789...
	 * 
	 */
	 
	struct line_t *line;
	
	/*
	 *	Contains the starting x-coordinate to draw the world at. This should be equal to
	 *	- getWidth() (see world.cpp) / 2
	*/
	
	int x_start;
	
	/*
	 *	Runs world detection for a single entity. This function is used in World->detect()
	 *	to detect the player and all entities in the world.
	*/
	
	void singleDetect(Entity *e);
	
	/*
	 *	Deletes all entities in the world.
	*/
	
	void deleteEntities(void);
	
	/*
	 *	The size of the line array. This is set once by World->generate().
	*/
	
	unsigned int lineCount;
	
	/*
	 *	Contains the background image layers (including the background image).
	*/
	
	vec2 *star;
	
	Texturec *bgTex;
	
	Mix_Music *bgmObj;
	char *bgm;
	
public:

	/*
	 *	These pointers keep track of worlds that are adjacent to this one. Used in ui.cpp
	 *	for world jumping.
	*/

	World *toLeft,
		  *toRight,
		  *behind,
		  *infront;
	
	/*
	 *	Entity arrays.
	*/
	
	std::vector<NPC			*>	npc;
	std::vector<Structures	*>	build;
	std::vector<Mob			*>	mob;
	std::vector<Entity		*>	entity;
	std::vector<Object		*>	object;
	
	void addStructure(_TYPE t,float x,float y,World *outside,World *inside);
	void addMob(int t,float x,float y);
	void addMob(int t,float x,float y,void (*hey)(Mob *));
	void addNPC(float x,float y);
	void addObject(ITEM_ID, bool, const char *, float, float);
	
	void update(Player *p,unsigned int delta);
	
	/*
	 *	Constructor and deconstructor, these do what you would expect.
	*/
						
	World(void);
	virtual ~World(void);				// Frees the 'line' array.
	
	/*
	 *	Generate a world of width `width`. This function is virtual so that other world
	 *	classes that are based on this one can generate themselves their own way.
	*/
	
	virtual void generate(unsigned int width);
	void generateFunc(unsigned int width,float(*func)(float));
	
	/*
	 *	Adds images to using for the background.
	*/
	
	void setBackground(WORLD_BG_TYPE bgt);
	
	/*
	 *	Start/stop background music. 
	*/
	
	void setBGM(const char *path);
	void bgmPlay(void);
	void bgmStop(void);
	
	/*
	 *	Looks for the furthest back layer in this world and adds a new layer of width `width` behind it.
	*/
	
	void addLayer(unsigned int width);
	
	/*
	 *	Draw the world and entities based on the player's coordinates. Virtual for the same
	 *	reason generate() is.
	*/
														
	virtual void draw(Player *p);
	
	
	/*
	 *	Detect the player and any entities in the current world.
	*/
	
	void detect(Player *p);
	
	/*
	 *	These functions return the pointer to the world in the direction that is requested if it
	 *	exists and the player is in a condition that it can make the switch, otherwise they
	 *	return the current world.
	*/
	
	World *goWorldLeft(Player *p);
	World *goWorldRight(Player *p);					
	World *goWorldBack(Player *p);
	World *goWorldFront(Player *p);
	
	/*
	 *	Called to enter/exit a structure.
	*/
	
	World *goInsideStructure(Player *p);
	
	/*
	 *	These functions add features to the world.
	*/
	
	void addHole(unsigned int start,unsigned int end);
	
	/*
	 *	Get's the world's width.
	*/
	
	int getTheWidth(void);
	
	void save(FILE *);
	void load(FILE *);
};

/*
 *	Gets a good base y value for background rendering.
*/

float worldGetYBase(World *w);

/*
 *	IndoorWorld - Indoor settings stored in a World class ;)
 */
 
class IndoorWorld : public World {
public:
	World *outside;
	IndoorWorld(void);
	~IndoorWorld(void);
	
	void generate(unsigned int width);	// Generates a flat world of width 'width'
	void draw(Player *p);				// Draws the world (ignores layers)
};

class Arena : public World {
private:
	vec2	 pxy;
	vec2	 door;
	World	*exit;
public:
	Arena(World *leave,Player *p);
	~Arena(void);
	World *exitArena(Player *p);
};

extern int worldShade;

#endif // WORLD_H
