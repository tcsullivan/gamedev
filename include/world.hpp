/**
 * @file  world.hpp
 * @brief The world system
 */
#ifndef WORLD_HPP_
#define WORLD_HPP_

#include <string>
#include <vector>

#include <SDL2/SDL_mixer.h>
#include <entityx/entityx.h>

#include <tinyxml2.h>
using namespace tinyxml2;

#include <components.hpp>
#include <events.hpp>
#include <texture.hpp>
#include <thread.hpp>
#include <vector2.hpp>

/**
 * The background type enum.
 * Used to choose which set of background images should be used.
 */
enum class WorldBGType : unsigned int {
	Forest = 0	/**< A forest theme. */
};

/**
 * The line structure.
 * This structure is used to store the world's ground, stored in vertical
 * lines. Dirt color and grass properties are also kept track of here.
 */
struct WorldData {
    bool          grassUnpressed; /**< squishes grass if false */
    float         grassHeight[2]; /**< height of the two grass blades */
    float         groundHeight;   /**< height of the 'line' */
    unsigned char groundColor;    /**< a value that affects the ground's color */
};

/**
 * Defines how many game ticks it takes to go from day to night or vice versa.
 * Technically a half day cycle...
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

/**
 * World data.
 * Contains all necessary data for a world. An instance of this is kept in the
 * world system, and is populated through it's load() function.
 */
struct WorldData2 {

	// Data variables
	std::vector<WorldData> data; /**< The world's ground data. */
	float startX;                /**< The furthest left coordinate of the world. */

	// Indoor variables
	bool indoor;                 /**< Set to true if this is an indoor world. */
	Texture indoorTex;           /**< The house's inside texture. */
	std::string outdoor;         /**< The file name of the outdoor world. */
	vec2 outdoorCoords;          /**< The coordinates the player should spawn to when exiting. */

	// World linkage
	std::string toLeft, toRight; /**< File names of the worlds adjacent to this one. */

	// Style variables
	WorldBGType style;                /**< The style type of the world. */
	std::string styleFolder;          /**< The folder to get stylized textures from. */
	std::vector<std::string> sTexLoc; /**< File names of stylized textures for structures. */

	// Music
	std::string bgm;             /**< The path to the BGM file. */
};

/**
 * The world system
 * Does everything needed to take care of the world.
 */
class WorldSystem : public entityx::System<WorldSystem>, public entityx::Receiver<WorldSystem> {
private:

	/**
	 * The world's data.
	 */
	WorldData2 world;

	/**
	 * SDL's object for handling the background music.
	 */
	Mix_Music *bgmObj;
	std::string bgmCurrent;

	/**
	 * Paths of files to get stylized textures from.
	 */
	std::vector<std::string> bgFiles;

	/**
	 * Allows for iteration between background textures, for rendering.
	 */
	TextureIterator bgTex;

	/**
	 * An object to handle and parse world XML files.
	 */
	XMLDocument xmlDoc;

	/**
	 * The file path to the currently loaded world.
	 */
	std::string currentXMLFile;

public:
	std::thread thAmbient;

	explicit WorldSystem(void);
	~WorldSystem(void);

	void configure(entityx::EventManager &ev) {
		ev.subscribe<BGMToggleEvent>(*this);
	}

	inline XMLDocument* getXML(void)
	{ return &xmlDoc; }

	inline float getWidth(void) const
	{ return world.startX * -2.0f; }

	float isAboveGround(const vec2& p) const;

	void receive(const BGMToggleEvent &bte);

	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
	void render(void);

	inline const std::string& getXMLFile(void) const
	{ return currentXMLFile; }

	void detect(entityx::TimeDelta dt);

	void goWorldLeft(Position& p);
	void goWorldRight(Position& p, Solid &d);
	void goWorldPortal(Position& p);

	// worlddata2 stuff
	WorldData2 worldData;

	void generate(int width = 0);
	//void addHole(const unsigned int& start, const unsigned int& end);
	//void addHill(const ivec2& peak, const unsigned int& width);

	bool save(void);
	void load(const std::string& file);
};

#endif // WORLD_HPP_
