#include <common.h>
#include <world.h>
#include <ui.h>
#include <entities.h>

extern World	*currentWorld;
extern Player	*player;

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
	currentWorld->npc[1]->addAIFunc(compTestQuest,true);
	return 0;
}

void initEverything(void){
	unsigned int i;
	
	/*
	 *	World creation:
	*/
	
	World *test=new World();
	
	/*
	 *	Load the saved world if it exists, otherwise generate a new one.
	*/
	
	/*FILE *worldLoad;
	if((worldLoad=fopen("world.dat","r"))){
		std::cout<<"Yes"<<std::endl;
		char *buf;
		unsigned int size;
		fseek(worldLoad,0,SEEK_END);
		size=ftell(worldLoad);
		rewind(worldLoad);
		buf=(char *)malloc(size);
		fread(buf,1,size,worldLoad);
		test->load(buf);
	}else{*/
		test->generate(SCREEN_WIDTH * 2);
		test->addHole(100,150);
	//}
	
	test->addLayer(400);
	
	/*
	 *	Setup the current world, making the player initially spawn in `test`.
	*/
	
	currentWorld=test;
	
	/*
	 *	Create the player.
	*/
	
	player=new Player();
	player->spawn(0,5000);
	
	/*
	 *	Create a structure (this will create villagers when spawned).
	*/
	
	currentWorld->addStructure(STRUCTURET,(rand()%120*HLINE),10);
	
	/*
	 *	Generate an indoor world and link the structure to it. 
	*/
	
	IndoorWorld *iw=new IndoorWorld();
	iw->generate(200);
	currentWorld->build.back()->inside=iw;

	/*
	 *	Spawn a mob. 
	*/
	
	currentWorld->addMob(MS_RABBIT,200,100);
	currentWorld->addMob(MS_BIRD,-500,500);
	
	/*
	 *	Link all the entities that were just created to the initial world, and setup a test AI function. 
	*/
	
	currentWorld->npc[0]->addAIFunc(giveTestQuest,false);
	
}
