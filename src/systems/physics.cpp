#include <systems/physics.hpp>

#include <components/direction.hpp>
#include <components/physics.hpp>

void PhysicsSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)ev;
	en.each<Direction, Physics>([dt](entityx::Entity entity, Direction &direction, Physics &physics) {
		(void)entity;
		// TODO GET GRAVITY FROM WORLD
		direction.y += physics.g * dt;
	});
}

