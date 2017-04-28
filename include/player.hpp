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
    entityx::Entity player;

    bool moveLeft;
    bool moveRight;

    float speed;

public:
    PlayerSystem(void)
        : moveLeft(false), moveRight(false), speed(1.0f) {}

	/**
	 * Creates the player, adding it to the entity system.
	 */
	void create(void);
	inline auto getId(void) const { return player.id(); }

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
    vec2 getPosition(void) const;

	/**
	 * Sets the player's X coordinate.
	 * @param x the x coordinate to give the player
	 */
	inline void setX(const float& x)
	{ player.component<Position>().get()->x = x; }

	/**
	 * Gets the width of the player.
	 * @return the player's width, according to its sprite
	 */
    inline float getWidth(void) const
    { return game::entities.component<Solid>(player.id())->width; }
};

#endif // PLAYER_HPP_
