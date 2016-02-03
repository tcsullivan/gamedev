/** @file Quest.h
 * @brief The quest handling system.
 * 
 * This file contains Quest and QuestHandler, used to manage quests inside the
 * game.
 */
 
#ifndef QUEST_H
#define QUEST_H

#include <cstring>

#include <common.h>
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
} Quest;

/*class Quest {
public:
	char *title;
	char *desc;
	struct item_t reward;
	Quest(const char *t,const char *d,struct item_t r);
	~Quest();
};*/

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
	
	int assign(const char *t);
	
	/**
	 * Drops a quest through its title.
	 */
	
	int drop(const char *t);
	
	/**
	 * Finishes a quest through it's title, also giving a pointer to the Entity
	 * that gave the quest originally.
	 */
	
	int finish(const char *t,void *completer);
	
	/**
	 * Returns true if this handler is currently taking the quest.
	 */
	
	bool hasQuest(const char *t);
};

#endif // QUEST_H
