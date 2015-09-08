#include <Quest.h>

Quest QuestList[TOTAL_QUESTS]={
	Quest("Test","A test quest",0)
};

Quest::Quest(){
}
Quest::Quest(const char *t,const char *d,unsigned int x){
	size_t len;
	title=(char *)malloc((len=strlen(t)));
	strncpy(title,t,len);
	desc=(char *)malloc((len=strlen(d)));
	strncpy(desc,d,len);
	xp=x;
}
Quest::~Quest(){
	free(title);
	free(desc);
	xp=0;
}

QuestHandler::QuestHandler(){
	ccnt=0;
}
int QuestHandler::assign(const char *t){
	unsigned int i=0;
	if(ccnt==QUEST_LIMIT)
		return -1;
	for(;i<TOTAL_QUESTS;i++){
		if(!strcmp(QuestList[i].title,t)){
			current[ccnt++]=&QuestList[i];
			return ccnt;
		}
	}
	return -1;
}
int QuestHandler::drop(const char *t){
	unsigned char i=0;
	for(;i<ccnt;i++){
		if(!strcmp(current[i]->title,t)){
			for(i++;i<ccnt;i++){
				current[i-1]=current[i];
			}
			return (--ccnt);
		}
	}
	return -1;
}
int QuestHandler::finish(const char *t){
	unsigned char i=0;
	unsigned int j;
	for(;i<ccnt;i++){
		if(!strcmp(current[i]->title,t)){
			j=current[i]->xp;
			for(i++;i<ccnt;i++){
				current[i-1]=current[i];
			}
			ccnt--;
			return j;
		}
	}
	return -1;
}
