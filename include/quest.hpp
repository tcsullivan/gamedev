/**
 * @file quest.hpp
 * Quest handling.
 */
#ifndef QUEST_HPP_
#define QUEST_HPP_

#include <entityx/entityx.h>

#include <forward_list>
#include <string>
#include <vector>

/**
 * The Quest structure.
 * Contains information necessary for a quest. 
 */
struct Quest
{
	Quest(std::string n = "", std::string d = "")
		: name(n), desc(d) {}

	std::string name; /**< the quest's title */
	std::string desc; /**< the quest's description */

	using Req = std::pair<std::string, int>;
	std::forward_list<Req> reqs; /**< the quest's item requirements */
	std::forward_list<Req> rewards; /**< the quest's rewards */
};

/**
 * @class QuestSystem
 * The quest system, handles active quests.
 */
class QuestSystem : public entityx::System<QuestSystem> {
private:
	/**
	 * A list of all quests that are currently active.
	 */
	static std::vector<Quest> current;

public:
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;

	/**
	 * Adds a quest to the current quest vector by its title.
	 * @param title the quest's title
	 * @param desc the quest's description
	 * @param req retrieved from XML, list of what the quest wants
	 * @return a possible error code
	 */
	static int assign(std::string title, std::string desc, std::string req);

	/**
	 * Drops a quest through its title.
	 * @param title the quest's title
	 * @return a possible error code
	 */
	static int drop(std::string title);

	/**
	 * Finishes a quest through it's title.
	 * @param title the quest's title
	 * @return a possible error code
	 */
	static int finish(std::string title);

	/**
	 * Returns true if the system is currently taking the quest.
	 * @param title the quest's title
	 * @return if the quest is active.
	 */
	static bool hasQuest(std::string title);
};

#endif // QUEST_HPP_
