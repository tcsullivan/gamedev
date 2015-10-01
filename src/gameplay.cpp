#include <common.h>
#include <world.h>
#include <ui.h>
#include <entities.h>

extern World *currentWorld;
extern std::vector<Entity*>entity;
extern std::vector<NPC>npc;
extern std::vector<Structures *>build;
extern Player *player;

int giveTestQuest(NPC *speaker){
	ui::dialogBox(speaker->name,"Here, have a quest!");
	player->qh.assign("Test");
	return 0;
}

int compTestQuest(NPC *speaker){
	if(player->qh.hasQuest("Test")){
		ui::dialogBox(speaker->name,"Ooo, that's a nice quest you got there. Lemme finish that for you ;).");
		player->qh.finish("Test");
		return 0;
	}else{
		ui::dialogBox(speaker->name,"You need to get a quest from %s first.",entity[1]->name);
		return 1;
	}
}

void initEverything(void){
	unsigned int i;
	
	World *test=new World();
	test->generate(SCREEN_WIDTH/2);
	test->addLayer(400);
	test->addLayer(100);
	test->addPlatform(150,100,100,10);
	test->addHole(100,150);
	currentWorld=test;
	
	// Make the player
	player=new Player();
	player->spawn(0,100);
	
	// Make structures
	entity.push_back(new Entity());
	build.push_back(new Structures());
	entity[0]=build[0];
	
	build[0]->spawn(STRUCTURET,0,10);
	IndoorWorld *iw=new IndoorWorld();
	iw->generate(200);
	build[0]->inside=iw;
	
	for(i=0;i<entity.size()+1;i++){
		entity[i]->inWorld=test;
		switch(i){
		case 1:
			NPCp(entity[i])->addAIFunc(giveTestQuest);
			break;
		case 2:
			NPCp(entity[i])->addAIFunc(compTestQuest);
			break;
		default:
			break;
		}
	}
}
