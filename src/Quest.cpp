#include <Quest.h>

#include <entities.h>

extern Player *player;

int QuestHandler::assign(std::string title,std::string desc,std::string req){
	Quest tmp;
	char *tok;
	
	tmp.title = title;
	tmp.desc = desc;

	std::unique_ptr<char[]> buf (new char[req.size()]);

	strcpy(buf.get(),req.c_str());
	tok = strtok(buf.get(),"\n\r\t,");
	tmp.need.push_back({"\0",0});
	
	while(tok){
		if(tmp.need.back().name != "\0"){
			tmp.need.back().n = atoi(tok);
			tmp.need.push_back({"\0",0});
		}else
			tmp.need.back().name = tok;
		
		tok = strtok(NULL,"\n\r\t,");
	}
	
	tmp.need.pop_back();
	current.push_back(tmp);

	return 0;
}

#include <algorithm>

int QuestHandler::drop(std::string title){
	current.erase( std::remove_if( current.begin(),
								   current.end(),
								   [&](Quest q){ return q.title == title; }),
				   current.end() );
	
	return 0;
}

int QuestHandler::finish(std::string t){
	for ( auto c = current.begin(); c != current.end(); c++ ) {
		if ( (*c).title == t ) {
			for ( auto &n : (*c).need ) {
				if ( player->inv->hasItem( n.name ) < n.n )
					return 0;
			}
			
			for ( auto &n : (*c).need )
				player->inv->takeItem( n.name, n.n );
			
			current.erase( c );
			return 1;
		}
	}
	
	return 0;
}

bool QuestHandler::hasQuest(std::string t){
	for ( auto &c : current ) {
		if ( c.title == t )
			return true;
	}
	
	return false;
}
