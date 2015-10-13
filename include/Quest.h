#ifndef QUEST_H
#define QUEST_H

#include <vector>
#include <cstdlib>
#include <cstring>

#include <inventory.h>

#define DEBUG

#define TOTAL_QUESTS 1

class Quest {
public:
	char *title,*desc;
	struct item_t reward;
	Quest(const char *t,const char *d,struct item_t r);
	~Quest();
};

class QuestHandler {
public:
	std::vector<const Quest *>current;
	int assign(const char *t);
	int drop(const char *t);
	int finish(const char *t,void *completer);
	bool hasQuest(const char *t);
};

#endif // QUEST_H
