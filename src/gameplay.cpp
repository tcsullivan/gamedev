#include <common.h>
#include <world.h>
#include <ui.h>
#include <entities.h>

extern World *currentWorld;
extern std::vector<Entity*>entity;
extern std::vector<NPC>npc;
extern std::vector<Structures *>build;
extern Player *player;

extern void mainLoop(void);

int compTestQuest(NPC *speaker){
	ui::dialogBox(speaker->name,"Ooo, that's a nice quest you got there. Lemme finish that for you ;).");
	player->qh.finish("Test",player);
	return 0;
}

int giveTestQuest(NPC *speaker){
	static bool done=false;
	if(!done){
		ui::dialogBox(speaker->name,"Here, have a quest!");
		player->qh.assign("Test");
		done=true;
	}
	/*while(ui::dialogBoxExists){	
		mainLoop();
	}*/
	NPCp(entity[2])->addAIFunc(compTestQuest);
	return 0;
}

int giveStuff(NPC *speaker){
	ui::dialogBox(speaker->name,"Take my stuff you ugly whore");
	player->inv->addItem(SWORD_ITEM,1);
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
	
	build[0]->spawn(STRUCTURET,0,10);
	IndoorWorld *iw=new IndoorWorld();
	iw->generate(200);
	build[0]->inside=iw;
	
	NPCp(entity[1])->addAIFunc(giveTestQuest);
	for(i=0;i<entity.size()+1;i++){
		entity[i]->inWorld=test;
		if(entity[i]->type==NPCT&&i>1)NPCp(entity[i])->addAIFunc(giveStuff);
	}
}
