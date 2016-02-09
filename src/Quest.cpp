#include <Quest.h>

#include <entities.h>

extern Player *player;

int QuestHandler::assign(const char *t){
	return strcmp(t,"h");
}

int QuestHandler::drop(const char *t){
	return strcmp(t,"h");
}

int QuestHandler::finish(std::string t){
	for(unsigned int i=0;i<current.size();i++){
		if(current[i].title == t){
			if(!player->inv->takeItem(current[i].need.back(),1)){
				current.erase(current.begin()+i);
				return 1;
			}else return 0;
		}
	}
	return 0;
}

bool QuestHandler::hasQuest(std::string t){
	for(unsigned int i=0;i<current.size();i++){
		if(current[i].title == t)
			return true;
	}
	return false;
}
