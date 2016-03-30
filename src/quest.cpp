#include <algorithm>

#include <quest.hpp>
#include <entities.hpp>

extern Player *player;

int QuestHandler::assign(std::string title,std::string desc,std::string req){
	Quest tmp;
	char *tok;

	tmp.title = title;
	tmp.desc = desc;

	tok = strtok( &req[0], "\n\r\t," );
	tmp.need.emplace_back( "", 0 );

	while ( tok ) {
		if ( !tmp.need.back().first.empty() ) {
			tmp.need.back().second = atoi( tok );
			tmp.need.emplace_back( "", 0 );
		} else
			tmp.need.back().first = tok;

		tok = strtok( NULL, "\n\r\t," );
	}

	tmp.need.pop_back();
	current.push_back( tmp );

	return 0;
}

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
				if ( player->inv->hasItem( n.first ) < n.second )
					return 0;
			}

			for ( auto &n : (*c).need )
				player->inv->takeItem( n.first, n.second );
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
