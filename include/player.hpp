#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include <entityx/entityx.h>

#include <events.hpp>
#include <common.hpp>

constexpr const float PLAYER_SPEED_CONSTANT = 0.15f;

class PlayerSystem : public entityx::System<PlayerSystem>, public entityx::Receiver<PlayerSystem> {
private:
    entityx::Entity::Id pid;

    bool moveLeft;
    bool moveRight;

    float speed;

public:
    PlayerSystem(void)
        : moveLeft(false), moveRight(false), speed(1.0f) {}

    void configure(entityx::EventManager&);

    void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;

    void receive(const KeyUpEvent&);
    void receive(const KeyDownEvent&);

    inline void setPlayer(const entityx::Entity& e)
    { pid = e.id(); }

    vec2 getPosition(void) const;
};

#endif // PLAYER_HPP_
