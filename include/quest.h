/** @file Quest.h
 * @brief The quest handling system.
 *
 * This file contains Quest and QuestHandler, used to manage quests inside the
 * game.
 */

#ifndef QUEST_H
#define QUEST_H

#include <string>

#include <inventory.h>

/**
 * When defined, DEBUG allows extra messages to be printed to the terminal for
 * debugging purposes.
 */

#define DEBUG

typedef struct {
	std::string title;
	std::string desc;
	struct item_t reward;
	std::vector<std::pair<std::string,int>> need;
} Quest;

/**
 * The Quest Handler class.
 *
 * This class handles quests, including the assigning, dropping, and completing
 * of the quests.
 */

class QuestHandler {
public:
	std::vector<Quest>current;

	/**
	 * Adds a quest to the current quest vector by its title.
	 */

	int assign(std::string title,std::string desc,std::string req);

	/**
	 * Drops a quest through its title.
	 */

	int drop(std::string title);

	/**
	 * Finishes a quest through it's title, also giving a pointer to the Entity
	 * that gave the quest originally.
	 */

	int finish(std::string t);

	/**
	 * Returns true if this handler is currently taking the quest.
	 */

	bool hasQuest(std::string t);
};

#endif // QUEST_H
