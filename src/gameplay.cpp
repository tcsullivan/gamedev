#include <common.h>
#include <world.h>
#include <ui.h>
#include <entities.h>

extern World *currentWorld;
extern std::vector<Entity*>entity;
extern std::vector<NPC>npc;
extern std::vector<Structures *>build;
extern Player *player;
extern std::vector<Mob>mob;


extern void mainLoop(void);

int compTestQuest(NPC *speaker){
	ui::dialogBox(speaker->name,"Ooo, that's a nice quest you got there. Lemme finish that for you ;).");
	player->qh.finish("Test",player);
	return 0;
}

int giveTestQuest(NPC *speaker){
	ui::dialogBox(speaker->name,"Here, have a quest!");
	player->qh.assign("Test");
	NPCp(entity[2])->addAIFunc(compTestQuest,true);
	return 0;
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

	build[0]->spawn(STRUCTURET,(rand()%120*HLINE),10);
	IndoorWorld *iw=new IndoorWorld();
	iw->generate(200);
	build[0]->inside=iw;

	entity.push_back(new Mob(1)); //create a new entity of NPC type
	mob.push_back(Mob(1)); //create new NPC
	entity[entity.size()] = &mob[mob.size()-1]; //set the new entity to have the same traits as an NPC
	entity[entity.size()-1]->spawn(200,100); //sets the position of the villager around the village
	entity.pop_back();
	
	
	NPCp(entity[1])->addAIFunc(giveTestQuest,false);
	for(i=0;i<entity.size()+1;i++){
		entity[i]->inWorld=test;
	}
}
