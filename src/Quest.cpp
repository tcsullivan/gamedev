#include <Quest.h>

#define TITLE	Quest(
#define DESC	,
#define REWARD	,(struct item_t){
#define x		,
#define END		}),

const Quest QuestList[TOTAL_QUESTS]={
//	Quest("Test","A test quest",(struct item_t){1,TEST_ITEM}),

// Get quest list
#include "../config/quest_list.txt"

};


Quest::Quest(const char *t,const char *d,struct item_t r){
	title = new char[strlen(t)+1];	//(char *)calloc(safe_strlen(t),sizeof(char));
	desc = new char[strlen(d)+1];		//(char *)calloc(safe_strlen(d),sizeof(char));
	strcpy(title,t);
	strcpy(desc,d);
	memcpy(&reward,&r,sizeof(struct item_t));
}

Quest::~Quest(){
	delete[] title;	//free(title);
	delete[] desc;	//free(desc);
	memset(&reward,0,sizeof(struct item_t));
}

int QuestHandler::assign(const char *t){
	unsigned char i;
	for(i=0;i<current.size();i++){				// Make sure we don't already have this quest
		if(!strcmp(current[i]->title,t)){
#ifdef DEBUG
			DEBUG_printf("The QuestHandler already has this quest: %s\n",t);
#endif // DEBUG
			return -2;
		}
	}
	for(i=0;i<TOTAL_QUESTS;i++){				// Add the quest (if it really exists)
		if(!strcmp(QuestList[i].title,t)){
			current.push_back(&QuestList[i]);
#ifdef DEBUG
			DEBUG_printf("Added quest %s, now have %u active quests.\n",t,current.size());
#endif // DEBUG
			return current.size();
		}
#ifdef DEBUG
		DEBUG_printf("Finding quest: %s != %s\n",t,QuestList[i].title);
#endif // DEBUG
	}
#ifdef DEBUG
	DEBUG_printf("Quest %s does not exist.\n",t);
#endif // DEBUG
	return -1;
}

int QuestHandler::drop(const char *t){
	unsigned char i;
	for(i=0;i<current.size();i++){
		if(!strcmp(current[i]->title,t)){
			current.erase(current.begin()+i);
			return current.size();
		}
	}
	return -1;
}

int QuestHandler::finish(const char *t,void *completer){
	unsigned char i;
	unsigned int r;
	for(i=0;i<current.size();i++){
		if(!strcmp(current[i]->title,t)){
#ifdef DEBUG
			DEBUG_printf("Completing quest %s.\n",t);
#endif // DEBUG
			((Entity *)completer)->inv->addItem(current[i]->reward.id,current[i]->reward.count);
			current.erase(current.begin()+i);
#ifdef DEBUG
			DEBUG_printf("QuestHandler now has %u active quests.\n",current.size());
#endif // DEBUG
			return 0;
		}
	}
#ifdef DEBUG
	DEBUG_printf("QuestHandler never had quest %s.\n",t);
#endif // DEBUG
	return -1;
}

bool QuestHandler::hasQuest(const char *t){
	unsigned int i;
	for(i=0;i<current.size();i++){
		if(!strcmp(current[i]->title,t)){
			return true;
		}
	}
	return false;
}
