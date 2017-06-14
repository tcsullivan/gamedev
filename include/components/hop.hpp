#ifndef COMPONENTS_HOP_HPP_
#define COMPONENTS_HOP_HPP_

/**
 * Causes the entity to hop around.
 */
struct Hop {
	Hop(float r = 0)
		: hopRatio(r) {}

	float hopRatio;
};

#endif // COMPONENTS_HOP_HPP
