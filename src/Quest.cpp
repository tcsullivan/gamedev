#include <Quest.h>

/*const Quest QuestList[1] = {
	Quest("Not a quest","Stop",(struct item_t){0,0})
};*/


/*Quest::Quest(const char *t,const char *d,struct item_t r){
	title = new char[strlen(t)+1];
	desc = new char[strlen(d)+1];
	strcpy(title,t);
	strcpy(desc,d);
	memcpy(&reward,&r,sizeof(struct item_t));
}

Quest::~Quest(){
	delete[] title;
	delete[] desc;
	memset(&reward,0,sizeof(struct item_t));
}*/

int QuestHandler::assign(const char *t){
	/*unsigned char i;
	for(i=0;i<current.size();i++){				// Make sure we don't already have this quest
		if(!strcmp(current[i]->title,t)){
#ifdef DEBUG
			DEBUG_printf("The QuestHandler already has this quest: %s\n",t);
#endif // DEBUG
			return -2;
		}
	}
	for(i=0;i<0;i++){				// Add the quest (if it really exists)
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
#endif // DEBUG*/
	return strcmp(t,"h");
}

int QuestHandler::drop(const char *t){
	/*unsigned char i;
	for(i=0;i<current.size();i++){
		if(!strcmp(current[i]->title,t)){
			current.erase(current.begin()+i);
			return current.size();
		}
	}*/
	return strcmp(t,"h");
}

int QuestHandler::finish(const char *t,void *completer){
	/*unsigned char i;
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
#endif // DEBUG*/
	return strncmp(t,(char *)completer,1);
}

bool QuestHandler::hasQuest(const char *t){
	unsigned int i;
	for(i=0;i<current.size();i++){
		if(!strcmp(current[i].title.c_str(),t)){
			return true;
		}
	}
	return false;
}
