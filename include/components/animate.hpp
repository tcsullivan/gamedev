#ifndef COMPONENTS_ANIMATE_HPP_
#define COMPONENTS_ANIMATE_HPP_

#include "base.hpp"

#include <components/sprite.hpp>

#include <vector>

/**
 * @struct Limb
 * @brief Storage of frames for the limbs of a sprite.
 * This will allow us to only update a certain limb. This was we can do mulitple animation types at once.
 */
struct Limb {
	Limb(void) {} // TODO necessary?

	// adds frame to the back of the frame stack
	inline void addFrame(Frame fr) {
		frame.push_back(fr);
	}

	void firstFrame(Frame& duckmyass);
	void nextFrame(Frame& duckmyass, float dt); 

	/**< How often we will change each frame. */
	float updateRate;

	/**< How much has been updated in the current frame. */
	float updateCurrent = 0; 

	/**< What the updateRate will base it's updates off of. ie: Movement, attacking, jumping. */
	unsigned int updateType;

	/**< The id of the limb we will be updating */
	unsigned int limbID;
	
	/**< The current sprite being used for the limb. */
	unsigned int index = 0;

	/**< The multiple frames of each limb. */
	std::vector<Frame> frame;
};

struct Animate : public Component {
	// COMMENT
	unsigned int index;
	// COMMENT	
	std::vector<Limb> limb;

	Animate(void)
		: index(0) {}
	Animate(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	// COMMENT
	void firstFrame(unsigned int updateType, Frame &sprite);
	//TODO make updateType an enum
	void updateAnimation(unsigned int updateType, Frame& sprite, float dt);
	void fromXML(XMLElement* imp, XMLElement* def) final;
};

#endif // COMPONENTS_ANIMATE_HPP_
