#ifndef SYSTEM_MOVEMENT_HPP_
#define SYSTEM_MOVEMENT_HPP_

#include <entityx/entityx.h>

class MovementSystem : public entityx::System<MovementSystem> {
public:
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
};

#endif // SYSTEM_MOVEMENT_HPP_
