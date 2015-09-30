#include <Quest.h>

const Quest QuestList[TOTAL_QUESTS]={
	Quest("Test","A test quest",0)
};

Quest::Quest(const char *t,const char *d,unsigned int r){
	size_t len;
	title=(char *)malloc((len=strlen(t)));
	strncpy(title,t,len);
	desc=(char *)malloc((len=strlen(d)));
	strncpy(desc,d,len);
	reward=r;
}

Quest::~Quest(){
	free(title);
	free(desc);
	reward=0;
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

int QuestHandler::finish(const char *t){
	unsigned char i;
	unsigned int r;
	for(;i<current.size();i++){
		if(!strcmp(current[i]->title,t)){
			r=current[i]->reward;
			current.erase(current.begin()+i);
			return r;
		}
	}
	return -1;
}
