/**
 * @file player.hpp
 * @brief The player system
 */
#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include <entityx/entityx.h>

#include <components.hpp>
#include <events.hpp>
#include <engine.hpp>
#include <vector2.hpp>

/**
 * The constant velocity the player is given when moved with the arrow keys.
 */
constexpr const float PLAYER_SPEED_CONSTANT = 0.03f;

/**
 * @class PlayerSystem
 * Controls a player, with keyboard and stuff.
 */
class PlayerSystem : public entityx::System<PlayerSystem>, public entityx::Receiver<PlayerSystem> {
private:
	static entityx::Entity player;

	static bool moveLeft;
	static bool moveRight;
	static float speed;

public:
	PlayerSystem(void);

	/**
	 * Creates the player, adding it to the entity system.
	 */
	static void create(void);
	static inline auto getId(void)
	{ return player.id(); }

	/**
	 * Configures events for use with the entity system.
	 */
    void configure(entityx::EventManager&);

	/**
	 * Updates the player, mainly the player's velocity.
	 */
    void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;

	/**
	 * Handles key up events for the player.
	 * @param kue key up event data
	 */
    void receive(const KeyUpEvent&);

	/**
	 * Handles key down events for the player.
	 * @param kde key down event data
	 */
    void receive(const KeyDownEvent&);

	void receive(const UseItemEvent&);

	/**
	 * Gets the player's position.
	 * @return the player's position
	 */
	static vec2 getPosition(void);

	/**
	 * Sets the player's X coordinate.
	 * @param x the x coordinate to give the player
	 */
	static inline void setX(const float& x)
	{ player.component<Position>().get()->x = x; }

	/**
	 * Gets the width of the player.
	 * @return the player's width, according to its sprite
	 */
    static inline float getWidth(void) 
    { return game::entities.component<Solid>(player.id())->width; }
};

#endif // PLAYER_HPP_
