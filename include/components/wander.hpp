#ifndef COMPONENTS_WANDER_HPP_
#define COMPONENTS_WANDER_HPP_

/**
 * Causes the entity to wander about.
 */
struct Wander {
	Wander(float ix = 0, float r = 0)
		: initialX(ix), range(r), countdown(0) {}

	float initialX;
	float range;
	int countdown;
};

#endif // COMPONENTS_WANDER_HPP_
