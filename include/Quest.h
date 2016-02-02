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

/**
 * The Quest class.
 * 
 * This contains information for a single quest, and should only really be interacted
 * with through QuestHandler.
 */

class Quest {
public:
	
	/**
	 * Contains the title of the quest.
	 */

	char *title;
	
	/**
	 * Contains the description of the quest.
	 */
	
	char *desc;
	
	/**
	 * Contains the single item that's given as a reward upon quest completion.
	 */
	
	struct item_t reward;
	
	/**
	 * Populates the values contained in this class.
	 */
	
	Quest(const char *t,const char *d,struct item_t r);
	
	/**
	 * Frees memory allocated for the title and description text.
	 */
	
	~Quest();
};

/**
 * The Quest Handler class.
 * 
 * This class handles quests, including the assigning, dropping, and completing
 * of the quests.
 */

class QuestHandler {
public:

	/**
	 * A vector containing all quests currently being taken by the handler.
	 */

	std::vector<const Quest *>current;
	
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

#include <entities.h>

#endif // QUEST_H
