#ifndef BRICE_H_
#define BRICE_H_

#include <string>

namespace game {
	extern bool canJump;
	extern bool canSprint;

	std::string getValue(const std::string& id);
	
	bool setValue(const std::string& id, const std::string& value);

	void briceClear(void);
	void briceSave(void);
	void briceLoad(void);

	void briceUpdate(void);
}

#endif // BRICE_H_
