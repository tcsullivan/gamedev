#include <common.h>
#include <world.h>
#include <ui.h>
#include <entities.h>

extern World	*currentWorld;
extern Player	*player;
extern std::vector<Entity		*>	entity;
extern std::vector<Structures	*>	build;
extern std::vector<Mob			*>	mob;
extern std::vector<NPC			*>	npc;


extern void mainLoop(void);

int compTestQuest(NPC *speaker){
	ui::dialogBox(speaker->name,"Ooo, that's a nice quest you got there. Lemme finish that for you ;).");
	player->qh.finish("Test",player);
	return 0;
}

int giveTestQuest(NPC *speaker){
	unsigned int i;
	ui::dialogBox(speaker->name,"Here, have a quest!");
	player->qh.assign("Test");
	npc[1]->addAIFunc(compTestQuest,true);
	return 0;
}

void initEverything(void){
	unsigned int i;
	
	/*
	 *	Generate a new world. 
	*/
	
	World *test=new World();
	test->generate(SCREEN_WIDTH);
	
	/*
	 *	Add two layers, a platform, and a hole to the world. 
	*/
	
	test->addLayer(400);
	
	test->addPlatform(150,100,100,10);

	test->addHole(100,150);
	
	/*
	 *	Setup the current world, making the player initially spawn in `test`.
	*/
	
	currentWorld=test;
	
	/*
	 *	Create the player.
	*/
	
	player=new Player();
	player->spawn(0,100);
	
	/*
	 *	Create a structure (this will create villagers when spawned).
	*/
	
	build.push_back(new Structures());
	entity.push_back(build.back());
	build.back()->spawn(STRUCTURET,(rand()%120*HLINE),10);
	
	/*
	 *	Generate an indoor world and link the structure to it. 
	*/
	
	IndoorWorld *iw=new IndoorWorld();
	iw->generate(200);
	build.back()->inside=iw;

	/*
	 *	Spawn a mob. 
	*/
	
	mob.push_back(new Mob(MS_RABBIT));
	entity.push_back(mob.back());
	mob.back()->spawn(200,100);

	mob.push_back(new Mob(MS_BIRD));
	entity.push_back(mob.back());
	mob.back()->spawn(-500,500);
	
	/*
	 *	Link all the entities that were just created to the initial world, and setup a test AI function. 
	*/
	npc[0]->addAIFunc(giveTestQuest,false);
	
	for(i=0;i<entity.size();i++){
		entity[i]->inWorld=currentWorld;
	}
}
