#include <components/animate.hpp>

void Limb::firstFrame(Frame& duckmyass)
{
	// loop through the spritedata of the sprite we wanna change
	for (auto &d : duckmyass) {
		// if the sprite data is the same limb as this limb
		if (d.first.limb == limbID) {
			// rotate through (for safety) the first frame to set the limb
			for (auto &fa : frame.at(0)) {
				if (fa.first.limb == limbID) {
					d.first = fa.first;
					d.second = fa.second;
				}
			}
		}
	}
}

void Limb::nextFrame(Frame& duckmyass, float dt) {
	updateCurrent -= dt;
	if (updateCurrent <= 0) {
		updateCurrent = updateRate;
	} else {
		return;
	}

	if (index < frame.size() - 1)
		index++;
	else
		index = 0;

	for (auto &d : duckmyass) {
		if (d.first.limb == limbID) {
			for (auto &fa : frame.at(index)) {
				if (fa.first.limb == limbID) {
					d.first = fa.first;
					d.second = fa.second;
				}
			}
		}
	}
}

void Animate::firstFrame(unsigned int updateType, Frame &sprite)
{
	unsigned int upid = updateType; //^see todo
	for (auto &l : limb) {
		if (l.updateType == upid) {
			l.firstFrame(sprite);
		}
	}
}

void Animate::updateAnimation(unsigned int updateType, Frame& sprite, float dt)
{
	unsigned int upid = updateType; //^see todo
	for (auto &l : limb) {
		if (l.updateType == upid) {
			l.nextFrame(sprite, dt);
		}
	}
}

void Animate::fromXML(XMLElement* imp, XMLElement* def) 
{
	(void)imp;

	auto animx = def->FirstChildElement();
	unsigned int limbid = 0;
	float limbupdate = 0;
	unsigned int limbupdatetype = 0;

	while (animx != nullptr) {
		if (std::string(animx->Name()) == "movement") {
			limbupdatetype = 1;

			auto limbx = animx->FirstChildElement();
			while (limbx != nullptr) {
				if (std::string(limbx->Name()) == "limb") {
					auto frames = developFrame(limbx);
					limb.push_back(Limb());
					auto& newLimb = limb.back();
					newLimb.updateType = limbupdatetype;
						if (limbx->QueryUnsignedAttribute("id", &limbid) == XML_NO_ERROR)
						newLimb.limbID = limbid;
					if (limbx->QueryFloatAttribute("update", &limbupdate) == XML_NO_ERROR)
						newLimb.updateRate = limbupdate;
							
					// place our newly developed frames in the entities animation stack
					for (auto &f : frames) {
						newLimb.addFrame(f);
						for (auto &fr : newLimb.frame) {
							for (auto &sd : fr)
								sd.first.limb = limbid;
						}
					}
				}
					limbx = limbx->NextSiblingElement();
			}
		}
			
		animx = animx->NextSiblingElement();
	}
}

