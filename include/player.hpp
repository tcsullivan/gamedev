#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include <entityx/entityx.h>

#include <events.hpp>
#include <components.hpp>
#include <common.hpp>

constexpr const float PLAYER_SPEED_CONSTANT = 0.15f;

class PlayerSystem : public entityx::System<PlayerSystem>, public entityx::Receiver<PlayerSystem> {
private:
    entityx::Entity player;

    bool moveLeft;
    bool moveRight;

    float speed;

public:
    PlayerSystem(void)
        : moveLeft(false), moveRight(false), speed(1.0f) {}

	void create(void);

    void configure(entityx::EventManager&);

    void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;

    void receive(const KeyUpEvent&);
    void receive(const KeyDownEvent&);

    vec2 getPosition(void) const;
	inline void setX(const float& x)
	{ player.component<Position>().get()->x = x; }
};

#endif // PLAYER_HPP_
