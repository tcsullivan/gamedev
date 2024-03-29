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
 * World data.
 * Contains all necessary data for a world. An instance of this is kept in the
 * world system, and is populated through it's load() function.
 */
struct WorldData {
	// Data variables
	ObjectTexture ground;
	float startX;                /**< The furthest left coordinate of the world. */

	// World linkage
	std::string toLeft, toRight; /**< File names of the worlds adjacent to this one. */
	std::string outdoor;         /**< The file name of the outdoor world. */

	// Style variables
	std::string styleFolder;          /**< The folder to get stylized textures from. */

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
	static WorldData world;

	/**
	 * SDL's object for handling the background music.
	 */
	static Mix_Music *bgmObj;
	static std::string bgmCurrent;

	/**
	 * Allows for iteration between background textures, for rendering.
	 */
	static TextureIterator bgTex;

	/**
	 * An object to handle and parse world XML files.
	 */
	static XMLDocument xmlDoc;

	/**
	 * The file path to the currently loaded world.
	 */
	static std::string currentXMLFile;

	static std::string toLoad;

	static std::vector<vec2> stars;

public:
	static std::thread thAmbient;

	explicit WorldSystem(void);
	~WorldSystem(void);

	void configure(entityx::EventManager &ev) {
		ev.subscribe<BGMToggleEvent>(*this);
	}

	static inline XMLDocument* getXML(void)
	{ return &xmlDoc; }

	static inline float getWidth(void) //const
	{ return world.startX * -2.0f; }

	static inline const std::string& getXMLFile(void) //const
	{ return currentXMLFile; }

	static float isAboveGround(const vec2& p); //const;

	bool receive(const BGMToggleEvent &bte);
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
	static void render(void);


	void detect(entityx::TimeDelta dt);

	static void goWorldLeft(Position& p);
	static void goWorldRight(Position& p, Solid &d);
	static void goWorldPortal(Position& p);

	static void generate(const char *file);
	static float getGroundHeight(float x);

	static bool save(void);
	static void load(const std::string& file);

	void fight(entityx::Entity entity);

	static void die(void);
	static void loader(void);

	static inline bool shouldLoad(void)
	{ return !toLoad.empty(); }
};

#endif // WORLD_HPP_
