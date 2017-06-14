#include <components/sprite.hpp>

#include <config.hpp>

SpriteData::SpriteData(std::string path, vec2 off)
	: offset(off)
{
	tex = Texture(path);
	size = tex.getDim();
	size_tex = vec2(1.0, 1.0);
	offset_tex.x = offset.x / size.x;
	offset_tex.y = offset.y / size.y;
}

SpriteData::SpriteData(std::string path, vec2 off, vec2 si)
	: size(si), offset(off)
{
	tex = Texture(path);
	vec2 tmpsize = tex.getDim();

	size_tex.x = size.x/tmpsize.x;
	size_tex.y = size.y/tmpsize.y;
			
	offset_tex.x = offset.x/tmpsize.x;
	offset_tex.y = offset.y/tmpsize.y;
}

SpriteData::SpriteData(Texture t)
	: tex(t)
{
	size_tex = 1;
	offset_tex = 0;
	size = tex.getDim();
	offset = 0;
}

int Sprite::clearSprite(void)
{
	if (sprite.empty())
		return 0;

	sprite.clear();
	return 1;
}

int Sprite::addSpriteSegment(SpriteData data, vec2 loc)
{
	//TODO if sprite is in this spot, do something
	sprite.push_back(std::make_pair(data, loc));
	return 1;
}

int Sprite::changeSpriteSegment(SpriteData data, vec2 loc)
{
	for (auto &s : sprite) {
		if (s.second == loc) {
			s.first = data;

			return 1;
		}
	}

	addSpriteSegment(data, loc);
	return 0;
}

vec2 Sprite::getSpriteSize()
{
	vec2 st; /** the start location of the sprite (bottom left)*/
	vec2 ed; /** the end ofthe location of the sprite (bottom right)*/
	vec2 dim; /** how wide the sprite is */

	if (sprite.size()) {
		st.x = sprite[0].second.x;
		st.y = sprite[0].second.y;

		ed.x = sprite[0].second.x + sprite[0].first.size.x;
		ed.y = sprite[0].second.y + sprite[0].first.size.y;
	} else {
		return vec2(0.0f, 0.0f);
	}

	for (auto &s : sprite) {
		if (s.second.x < st.x)
			st.x = s.second.x;
		if (s.second.y < st.y)
			st.y = s.second.y;

		if (s.second.x + s.first.size.x > ed.x)
			ed.x = s.second.x + s.first.size.x;
		if (s.second.y + s.first.size.y > ed.y)
			ed.y = s.second.y + s.first.size.y;
	}

	dim = vec2(ed.x - st.x, ed.y - st.y);
	dim.x *= game::HLINE;
	dim.y *= game::HLINE;
	return dim;
}

std::vector<Frame> developFrame(XMLElement* xml)
{
	Frame tmpf;
	std::vector<Frame> tmp;
	SpriteData sd;	

	unsigned int limb = 0;

	vec2 foffset;
	vec2 fsize;
	vec2 fdraw;

	tmp.clear();

	// this is the xml elements first child. It will only be the <frame> tag
	auto framexml = xml->FirstChildElement();
	while (framexml) {
		// this will always be frame. but if it isn't we don't wanna crash the game
		std::string defframe = framexml->Name();
		if (defframe == "frame") {
			tmpf.clear();
			// the xml element to parse each src of the frames
			auto sxml = framexml->FirstChildElement();
			while (sxml) {
				std::string sname = sxml->Name();
				if (sname == "src") {
					foffset = (sxml->Attribute("offset") != nullptr) ? 
						sxml->StrAttribute("offset") : vec2(0,0);
					fdraw = (sxml->Attribute("drawOffset") != nullptr) ?
						sxml->StrAttribute("drawOffset") : vec2(0,0);
					
					if (sxml->Attribute("size") != nullptr) {
						fsize = sxml->StrAttribute("size");
						sd = SpriteData(sxml->GetText(), foffset, fsize);
					} else {
						sd = SpriteData(sxml->GetText(), foffset);
					}
					if (sxml->QueryUnsignedAttribute("limb", &limb) == XML_NO_ERROR) 
						sd.limb = limb;
					tmpf.push_back(std::make_pair(sd, fdraw));
				}
				sxml = sxml->NextSiblingElement();
			}
			// we don't want segfault
			if (tmpf.size())
				tmp.push_back(tmpf);
		}
		// if it's not a frame we don't care

		// parse next frame
		framexml = framexml->NextSiblingElement();
	}

	return tmp;
}
