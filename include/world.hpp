#ifndef WORLD_H
#define WORLD_H

/**
 * @file  world.hpp
 * @brief The world system
 */

// local game includes
#include <common.hpp>
#include <coolarray.hpp>
#include <events.hpp>
#include <texture.hpp>
#include <tinyxml2.h>
#include <components.hpp>
using namespace tinyxml2;

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

#include <entityx/entityx.h>

constexpr const char* WorldWeatherString[3] = {
	"None",
	"Rainy",
	"Snowy"
};

struct WorldData2 {
	// data
	std::vector<WorldData> data;
	float startX;

	// indoor
	bool indoor;
	float indoorWidth;
	GLuint indoorTex;

	// links
	std::string toLeft, toRight;

	// style
	WorldBGType style;
	std::string styleFolder;
	std::vector<std::string> sTexLoc;

	// music
	std::string bgm;

	// village
	float villageStart, villageEnd;
};

class WorldSystem : public entityx::System<WorldSystem>, public entityx::Receiver<WorldSystem> {
private:
	WorldData2 world;

	WorldWeather weather;

	Mix_Music *bgmObj;

	std::vector<std::string> bgFiles;

	TextureIterator bgTex;

	XMLDocument xmlDoc;

public:
	explicit WorldSystem(void);
	~WorldSystem(void);

	void configure(entityx::EventManager &ev) {
		ev.subscribe<BGMToggleEvent>(*this);
	}

	inline float getWidth(void) const
	{ return world.startX * -2.0f; }

	void receive(const BGMToggleEvent &bte);

	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
	void render(void);

	inline const std::string getWeatherStr(void) const
	{ return WorldWeatherString[static_cast<int>(weather)]; }

	inline const WorldWeather& getWeatherId(void) const
	{ return weather; }

	void setWeather(const std::string &s);

	void detect(entityx::TimeDelta dt);

	void goWorldLeft(Position& p);
	void goWorldRight(Position& p);
	
	// worlddata2 stuff
	WorldData2 worldData;

	void generate(unsigned int width = 0);
	void addHole(const unsigned int& start, const unsigned int& end);
	void addHill(const ivec2& peak, const unsigned int& width);

	bool save(const std::string& file);
	void load(const std::string& file);
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

#endif // WORLD_H
