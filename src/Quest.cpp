#include <Quest.h>

int QuestHandler::assign(const char *t){
	return strcmp(t,"h");
}

int QuestHandler::drop(const char *t){
	return strcmp(t,"h");
}

int QuestHandler::finish(const char *t,void *completer){
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
