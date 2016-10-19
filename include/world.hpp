#ifndef WORLD_H
#define WORLD_H

/**
 * @file  world.hpp
 * @brief The world system
 */

// local game includes
#include <common.hpp>
#include <entities.hpp>
#include <coolarray.hpp>

/**
 * The background type enum.
 * This enum contains all different possibilities for world backgrounds; used
 * in World::setBackground() to select the appropriate images.
 */
enum class WorldBGType : unsigned int {
	Forest = 0	/**< A forest theme. */
};

/**
 * The weather type enum.
 * This enum contains every type of weather currently implemented in the game.
 * Weather is set by the world somewhere.
 */
enum class WorldWeather : unsigned char {
	None = 0,	/**< None (sunny) */
	Rain,		/**< Rain */
	Snowy		/**< Snow */
};

/**
 * The line structure.
 * This structure is used to store the world's ground, stored in vertical
 * lines. Dirt color and grass properties are also kept track of here.
 */
typedef struct {
    bool          grassUnpressed; /**< squishes grass if false */
    float         grassHeight[2]; /**< height of the two grass blades */
    float         groundHeight;   /**< height of the 'line' */
    unsigned char groundColor;    /**< a value that affects the ground's color */
} WorldData;

/**
 * Contains info necessary for switching worlds.
 * This pair contains a pointer to the new world, and the new set of
 * coordinates the player should be at in that world.
 */
using WorldSwitchInfo = std::pair<World *, vec2>;

/**
 * Alters how bright world elements are drawn.
 * This value is based off of the current time of day (tick count), set in
 * main.cpp.
 */
extern int worldShade;

/**
 * The file path to the currently loaded XML file.
 */
extern std::string currentXML;

/**
 * Defines how many game ticks it takes to go from day to night or vice versa.
 */
constexpr const unsigned int DAY_CYCLE = 10000;

/**
 * Defines the velocity of player when moved by the keyboard
 */
constexpr const float PLAYER_SPEED_CONSTANT = 0.15f;

/**
 * Defines the strongest pull gravity can have on an entity.
 * This is the most that can be subtracted from an entity's velocity in one
 * game tick.
 */
constexpr const float GRAVITY_CONSTANT = 0.001f;

/**
 * Defines the thickness of the floor in an indoor world.
 */
constexpr const unsigned int INDOOR_FLOOR_THICKNESS = 50;

/**
 * Defines how far each floor can be from the next (height).
 */
constexpr const unsigned int INDOOR_FLOOR_HEIGHTT = 400;

/**
 * Gets a combined height of the floor and the area before it.
 * This value is commonly used for operations like moving between floors.
 */
constexpr const unsigned int INDOOR_FLOOR_HEIGHT = (INDOOR_FLOOR_HEIGHTT + INDOOR_FLOOR_THICKNESS);

/**
 * The village class.
 * This class defines an area in a world that is considered to be a village,
 * and provides a welcome message when the player enters the area.
 */
class Village {
public:
	/**
	 * The name of the village.
	 */
	std::string name;

	/**
	 * The start and end coordinates of the village.
	 */
	vec2 start, end;

	/**
	 * A "player in village" flag.
	 * This flag is used to trigger displaying the welcome message.
	 */
	bool in;

	/**
	 * Constructs a village with the given name, inside the given world.
	 */
	Village(std::string meme, World *w);

	/**
	 * Destructs the village.
	 */
	~Village(void){}
};


#include <entityx/entityx.h>

constexpr const char* WorldWeatherString[3] = {
	"None",
	"Rainy",
	"Snowy"
};

class WorldSystem : public entityx::System<WorldSystem>, public entityx::Receiver<WorldSystem> {
private:
	World *world;
	World *outside;

	WorldWeather weather;

	Mix_Music *bgmObj;
	std::string bgmObjFile;

	std::vector<std::string> bgFiles;

	TextureIterator bgTex;

public:
	explicit WorldSystem(void);
	~WorldSystem(void);

	void configure(entityx::EventManager &ev) {
		ev.subscribe<BGMToggleEvent>(*this);
	}

	void receive(const BGMToggleEvent &bte);

	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
	void render(void);

	void setWorld(World *w);

	inline const std::string getWeatherStr(void) const
	{ return WorldWeatherString[static_cast<int>(weather)]; }

	inline const WorldWeather& getWeatherId(void) const
	{ return weather; }

	void setWeather(const std::string &s);

	void singleDetect(Entity *e, entityx::TimeDelta dt);
	void detect(entityx::TimeDelta dt);

	void detect2(entityx::TimeDelta dt);

	void enterWorld(World *w);
	void leaveWorld(void);
};


/**
 * The world class.
 * This class handles entity creation, management, and deletion. Most
 * world-related operations have to be done through this class, such as
 * drawing.
 */
class World {
private:
	bool m_Indoor;

public:

	float HouseWidth;
	GLuint houseTex;

	inline bool isIndoor(void) const
	{ return m_Indoor; }

	WorldBGType bgType;

	std::string styleFolder;
	
	/**
	 * An array of all the world's ground data, populated through
	 * World::generate().
	 * @see generate()
	 */
	std::vector<WorldData> worldData;

	/**
	 * Contains the size of the 'worldData' array.
	 */
	unsigned int lineCount;

	/**
	 * The starting x-coordinate of the world.
	 */
	float worldStart;

	/**
	 * The path to the XML file of the world to the left.
	 *
	 * @see setToLeft()
	 */
	std::string toLeft;

	/**
	 * The path to the XML file of the world to the right.
	 *
	 * @see setToRight()
	 */
	std::string toRight;

	/**
	 * A vector of paths for the structure textures.
	 * The appearance of structures depends on the world's theme.
	 *
	 * @see setStyle()
	 */
	std::vector<std::string> sTexLoc;

	/**
	 * Contains randomly generated coordinates for stars.
	 */
	std::vector<vec2> star;

	/**
	 * A vector of all light elements in the world.
	 *
	 * @see addLight()
	 * @see getLastLight()
	 */
	std::vector<Light>        light;

	/**
	 * A vector of all villages in the world.
	 *
	 * @see addVillage()
	 */
	std::vector<Village>      village;

	std::vector<Entity *> entityPending;

	/**
	 * Destroys entities and clears vectors that contain them.
	 * This function is only called in the world destructor.
	 */
	void deleteEntities(void);

	/**
	 * The filename of the world's BGM file.
	 *
	 * @see setBGM()
	 */
	std::string bgm;

	CoolArray<Particles>    particles;

	/**
	 * A vector of pointers to all entities from the other vectors.
	 * This is used to mass-manage entities, or operate on entities
	 * outside of what the world does.
	 *
	 * @see getNearInteractable()
	 */
	std::vector<Entity *> entity;

	/**
	 * Constructs the world, resets variables.
	 */
	World(bool indoor = false);

	/**
	 * Destructs the world, frees memory.
	 */
	virtual ~World(void);

	/**
	 * Generates a world of the specified width.
	 * This will populate the 'worldData' array and create star coordinates.
	 * It's necessary to call this function to actually use the world.
	 */
	void generate(int width);

	/**
	 * Draws everything the world handles to the screen (and the player).
	 * Drawing is based off of the player so that off-screen elements are not
	 * drawn.
	 */
	virtual void draw(Player *p);

	/**
	 * Gets the width of the world, presumably in pixels.
	 * TODO
	 */
	int getTheWidth(void) const;

	/**
	 * Gets the starting x-coordinate of the world.
	 *
	 * @see worldStart
	 */
	float getWorldStart(void) const;

	inline unsigned int getEntityCount(void) const {
		return entity.size();
	}

	/**
	 * Gets a pointer to the most recently created light.
	 * This is used to update properties of the light outside of the
	 * world class.
	 */
	Light& getLastLight(void);

	/**
	 * Gets a pointer ot the most recently created mob.
	 * This is used to update properties of the mob outside of the
	 * world class.
	 */
	Mob* getLastMob(void);

	/**
	 * Finds the entity nearest to the provided one.
	 */
	Entity* getNearInteractable(Entity &e);

	/**
	 * Finds the mob nearest to the given entity.
	 */
	Mob* getNearMob(Entity &e);

	/**
	 * Gets the coordinates of the `index`th structure.
	 */
	vec2 getStructurePos(int index);

	/**
	 * Gets the texture path of the `index`th structure
	 */
	std::string getSTextureLocation(unsigned int index) const;

	// saves the world's data to an XML file, either the one provided or the current path
	void save(const std::string& s="");

	// sets the world's background theme
	void setBackground(WorldBGType bgt);

	// sets the folder to collect entity textures from
	void setStyle(std::string pre);

	// gets the string that represents the current weather
	std::string getWeatherStr(void) const;
	const WorldWeather& getWeatherId(void) const;

	// sets the weatherrrr
	void setWeather(const std::string& w);

	// sets / gets pathnames of XML files for worlds to the left and right
	std::string setToLeft(std::string file);
	std::string setToRight(std::string file);
	std::string getToLeft(void) const;
	std::string getToRight(void) const;

	// attempts to enter the left/right adjacent world, returning either that world or this
	WorldSwitchInfo goWorldLeft(Player *p);
	WorldSwitchInfo goWorldRight(Player *p);

	/**
	 * Attempts to move an NPC to the left adjacent world, returning true on success.
	 */
	bool goWorldLeft(NPC *e);

	/**
	 * Attempts to move an NPC to the world to the right, returning true on success.
	 */
	bool goWorldRight(NPC *e);

	// attempts to enter a structure that the player would be standing in front of
	WorldSwitchInfo goInsideStructure(Player *p);

	/**
	 * Adopts an NPC from another world, taking its ownership.
	 */
	void adoptNPC(NPC *e);

	/**
	 * Adopts a mob from another world, taking its ownership.
	 */
	void adoptMob(Mob *e);

	// adds a hole at the specified start and end x-coordinates
	void addHole(unsigned int start,unsigned int end);

	// adds a hill that peaks at the given coordinate and is `width` HLINEs wide
	void addHill(ivec2 peak, unsigned int width);

	// functions to add entities to the world
	void addLight(vec2 xy, float radius, Color color);

	void addMerchant(float x, float y, bool housed);

	void addMob(Mob *m, vec2 coord);

	void addNPC(NPC *n);

	void addObject(std::string in, std::string pickupDialog, float x, float y);

	void addParticle(float x, float y, float w, float h, float vx, float vy, Color color, int dur);
	void addParticle(float x, float y, float w, float h, float vx, float vy, Color color, int dur, unsigned char flags);

	void addStructure(Structures *s);

	Village *addVillage(std::string name, World *world);
};

/**
 * The arena class - creates an arena.
 *
 * This world, when created, expects a pointer to a Mob. This mob will be
 * transported to a temporary world with the player, and the Mob will be
 * killed upon exiting the arena.
 */

class Arena : public World {
private:

	// the mob that the player is fighting
	Mob *mmob;

public:

	// creates the arena with the world being left for it
	Arena(void);

	// frees memory
	~Arena(void);

	// starts a new fight??
	void fight(World *leave, const Player *p, Mob *m);

	// attempts to exit the arena, returning what world the player should be in
	WorldSwitchInfo exitArena(Player *p);
};

/**
 * Constructs an XML object for accessing/modifying the current world's XML
 * file.
 */
const XMLDocument& loadWorldXML(void);

/**
 * Loads the player into the world created by the given XML file. If a world is
 * already loaded it will be saved before the transition is made.
 */
World *loadWorldFromXML(std::string path);

/**
 * Loads the player into the XML-scripted world, but does not save data from the
 * previous world if one was loaded.
 */
World *loadWorldFromXMLNoSave(std::string path);
World *loadWorldFromXMLNoTakeover(std::string path);

/**
 * Loads a world using a pointer to the current world (used for loading adjacent
 * worlds that have already been read into memory.
 */
World *loadWorldFromPtr(World *ptr);

/**
 * Casts a normal world to an indoor world, to access IndoorWorld-exclusive
 * elements.
 */
constexpr IndoorWorld *Indoorp(World *w)
{
    return (IndoorWorld *)w;
}

#endif // WORLD_H
