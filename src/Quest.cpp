#include <Quest.h>
#include <entities.h>

const Quest QuestList[TOTAL_QUESTS]={
	Quest("Test","A test quest",(struct item_t){1,TEST_ITEM})
};

Quest::Quest(const char *t,const char *d,struct item_t r){
	size_t len;
	title=(char *)malloc((len=strlen(t)));
	strncpy(title,t,len);
	desc=(char *)malloc((len=strlen(d)));
	strncpy(desc,d,len);
	memcpy(&reward,&r,sizeof(struct item_t));
}

Quest::~Quest(){
	free(title);
	free(desc);
	memset(&reward,0,sizeof(struct item_t));
}

int QuestHandler::assign(const char *t){
	unsigned char i;
	for(i=0;i<current.size();i++){
		if(!strcmp(current[i]->title,t)){
			return -2;
		}
	}
	for(i=0;i<TOTAL_QUESTS;i++){
		if(!strcmp(QuestList[i].title,t)){
			current.push_back(&QuestList[i]);
			return current.size();
		}
	}
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
			((Entity *)completer)->inv->addItem(current[i]->reward.id,current[i]->reward.count);
			current.erase(current.begin()+i);
			return 0;
		}
	}
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
