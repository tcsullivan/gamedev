#ifndef QUEST_H
#define QUEST_H

#include <vector>
#include <cstdlib>
#include <cstring>

#define TOTAL_QUESTS 1

class Quest {
public:
	char *title,*desc;
	unsigned int reward;
	Quest(const char *t,const char *d,unsigned int r);
	~Quest();
};

class QuestHandler {
public:
	std::vector<const Quest *>current;
	int assign(const char *t);
	int drop(const char *t);
	int finish(const char *t);
	bool hasQuest(const char *t);
};

#endif // QUEST_H
