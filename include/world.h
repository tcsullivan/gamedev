#ifndef WORLD_H
#define WORLD_H

#include <common.h> // For HLINE, vec2, OpenGL utilities, etc.

typedef struct {
	vec2 p1,p2;
} __attribute__ ((packed)) Platform;

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
	struct line_t {
		float y;
		unsigned char color;
	} __attribute__ ((packed)) *line;
	unsigned int lineCount;	// Size of the array 'line' (aka the width of the world)
	std::vector<Platform> platform;	// An array (vector thing) of platforms
	int x_start;			// Worlds are centered on the x axis (0,n), this contains
							// where to start drawing the world to have it centered properly.
	World *behind,*infront;	// Pointers to other areas of land that are behind or in front of this one, respectively.
	void singleDetect(Entity *e);	// Handles an individual entity (gravity n' stuff)
public:
	World *toLeft,*toRight;		// Pointers to areas to the left and right of this world. These are made public
								// so that they can easily be set without a function.
								
	World(void);
	~World(void);				// Frees the 'line' array.
	
	virtual void generate(unsigned int width);			// Generate the world
	
	void addLayer(unsigned int width);					// Generates a new world and makes 'behind' point to it. If 'behind'
														// already points to a world, the new world will be set to be behind 'behind'.
														
	virtual void draw(vec2 *vec);						// Draws the world around the coordinates 'vec'
	
	
	void detect(Player *p);								// Insures objects/entities stored in an Entity class stay outside of the
														// ground (defined by array 'line'), and handles gravity for the object/entity
														// by modifying it's velocity
	
	World *goWorldLeft(Player *p);						// Returns the world to the left of this one if it exists and the player at
														// location 'loc' with width 'width' is at the left edge of this world.
	World *goWorldRight(Player *p);						// Functions the same as goWorldLeft(), but checks/returns the world to the right
														// of the player.
														
	World *goWorldBack(Player *p);						// Returns the address of the world behind this one if it exists and the player
														// at location 'loc' with width 'width' is within the area of it (i.e., when this
														// world is drawn the world has to appear directly behind the player)
	World *goWorldFront(Player *p);						// Functions the same as goWorldBack(), but checks/returns the world in front of
														// this one.
	World *goInsideStructure(Player *p);				// Returns the world contained in a structure if the player is requesting to enter
														// it and is standing in front of it.
	void addPlatform(float x,float y,float w,float h);	// Dynamically adds a platform to the platform array. These will be automatically
														// drawn and handled by the world.
	void addHole(unsigned int start,unsigned int end);	// Create a hole in the world
};

/*
 *	IndoorWorld - Indoor settings stored in a World class ;)
 */
class IndoorWorld : public World {
public:
	IndoorWorld(void);
	~IndoorWorld(void);
	
	void generate(unsigned int width);	// Generates a flat world of width 'width'
	void draw(vec2 *vec);				// Draws the world (ignores layers)
};

#endif // WORLD_H
