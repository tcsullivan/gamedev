#ifndef COMPONENTS_SPRITE_HPP_
#define COMPONENTS_SPRITE_HPP_

#include "base.hpp"

#include <texture.hpp>
#include <vector2.hpp>

#include <vector>

struct SpriteData {
	SpriteData(void) = default;	
	SpriteData(Texture t);
	SpriteData(std::string path, vec2 off);
	SpriteData(std::string path, vec2 off, vec2 si);

	Texture tex;
	vec2 size;
	vec2 offset;
	
	vec2 offset_tex;
	vec2 size_tex;

	unsigned int limb;
};

using Frame = std::vector<std::pair<SpriteData, vec2>>;

std::vector<Frame> developFrame(XMLElement*);

/**
 * @struct Sprite
 * @brief If an entity is visible we want to be able to see it.
 * Each entity is given a sprite, a sprite can consist of manu frames or pieces to make one.
 */
struct Sprite : public Component {
	Sprite(bool left = false)
	 	: faceLeft(left) {}
	Sprite(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	inline Frame getSprite(void) {
		return sprite;
	}

	int clearSprite(void);
	int addSpriteSegment(SpriteData data, vec2 loc);
	int changeSpriteSegment(SpriteData data, vec2 loc);
	vec2 getSpriteSize(); 

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		auto frames = developFrame(def);
		if (!frames.empty())
			sprite = frames.at(0);
	}

	Frame sprite;
	bool faceLeft;
};


#endif // COMPONENTS_SPRITE_HPP_
