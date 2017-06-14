#ifndef SYSTEM_PHYSICS_HPP_
#define SYSTEM_PHYSICS_HPP_

#include <entityx/entityx.h>

class PhysicsSystem : public entityx::System<PhysicsSystem> {
public:
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
};

#endif // SYSTEM_PHYSICS_HPP_
