#ifndef QUEST_H
#define QUEST_H

#include <cstdlib>
#include <cstring>

#define QUEST_LIMIT 5
#define TOTAL_QUESTS 1

class Quest {
public:
	char *title,*desc;
	unsigned int xp;
	Quest();
	Quest(const char *t,const char *d,unsigned int x);
	~Quest();
};

class QuestHandler {
private:
	unsigned char ccnt;
	Quest *current[QUEST_LIMIT];
public:
	QuestHandler();
	int assign(const char *t);
	int drop(const char *t);
	int finish(const char *t);
};

#endif // QUEST_H
