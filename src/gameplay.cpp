#include <common.h>
#include <world.h>
#include <ui.h>
#include <entities.h>

extern World	*currentWorld;
extern Player	*player;

int compTestQuest(NPC *speaker){
	ui::dialogBox(speaker->name,NULL,"Ooo, that's a nice quest you got there. Lemme finish that for you ;).");
	player->qh.finish("Test",player);
	return 0;
}

int giveTestQuest(NPC *speaker){
	ui::dialogBox(speaker->name,":Yes:No","Here, have a quest!");
	ui::waitForDialog();
	
	if(ui::dialogOptChosen == 1){
		
		ui::dialogBox(speaker->name,"","Have a good day! :)");
		ui::waitForDialog();

		player->qh.assign("Test");
		currentWorld->npc[1]->addAIFunc(compTestQuest,true);
		
	}else return 1;
	
	return 0;
}

void CUTSCENEEE(Mob *callee){
	player->vel.x = 0;
	
	ui::dialogBox(player->name,":K then","No way I\'m gettin\' up this hill.");
	ui::waitForDialog();

	player->right = true;
	player->left  = false;
	player->loc.x += HLINE * 5;
	
	callee->alive = true;
}

void CUTSCENEEE2(Mob *callee){
	player->vel.x = 0;
	ui::dialogBox(player->name,":Yeah.",
	"What the fuck is this dead end supposed to mean, and why this place smell like soap.");
	ui::waitForDialog();
	callee->alive = false;
}

void story(Mob *callee){
	player->vel.x = 0;
	Mix_FadeOutMusic(0);
	ui::importantText("It was a dark and stormy night...");
	ui::waitForDialog();
	callee->alive = false;
}

float playerSpawnHillFunc(float x){
	return (float)(pow(2,(-x+200)/5) + 80);
}

static World *test;
static World *playerSpawnHill;
static IndoorWorld *iw;

void destroyEverything(void);

void initEverything(void){
	//FILE *load;
	
	/*
	 *	World creation:
	*/
	
	test=new World();
	
	test->generate(SCREEN_WIDTH*2);
	test->setBackground(BG_FOREST);
	test->setBGM("assets/music/embark.wav");
	
	test->addHole(100,150);
	test->addLayer(400);
	
	playerSpawnHill=new World();
	playerSpawnHill->setBackground(BG_FOREST);
	playerSpawnHill->setBGM("assets/music/embark.wav");
	
	/*if((load=fopen("world.dat","rb"))){
		playerSpawnHill->load(load);
		fclose(load);
	}else{*/
		playerSpawnHill->generateFunc(1280,playerSpawnHillFunc);
	//}

	/*
	 *	Setup the current world, making the player initially spawn in `test`.
	*/
	
	currentWorld=playerSpawnHill;
	
	playerSpawnHill->toRight=test;
	test->toLeft=playerSpawnHill;
	
	/*
	 *	Create the player.
	*/
	
	player=new Player();
	player->spawn(-1000,200);
	
	/*
	 *	Create a structure (this will create villagers when spawned).
	*/
	
	iw=new IndoorWorld();
	iw->setBackground(BG_WOODHOUSE);
	iw->setBGM(NULL);
	iw->generate(200);
	iw->addMob(MS_TRIGGER,0,0,CUTSCENEEE2);
	
	/*
	 *	Spawn some entities.
	*/

	playerSpawnHill->addMob(MS_TRIGGER,player->loc.x,0,story);

	playerSpawnHill->addStructure(STRUCTURET,(rand()%120*HLINE),100,test,iw);
	playerSpawnHill->addMob(MS_TRIGGER,-1300,0,CUTSCENEEE);
	
	playerSpawnHill->addObject(SWORD_WOOD, false, "", 480,200);
	playerSpawnHill->addObject(FLASHLIGHT, false, "", 500,200);
	playerSpawnHill->addObject(PLAYER_BAG, false, "", 520,200);
	playerSpawnHill->addObject(TEST_ITEM, false, "", 540,200);
	
	test->addMob(MS_RABBIT,200,100);
	test->addMob(MS_BIRD,-500,500);
	
	playerSpawnHill->npc[0]->addAIFunc(giveTestQuest,false);
	
	currentWorld->bgmPlay();
	atexit(destroyEverything);
}

extern std::vector<int (*)(NPC *)> AIpreload;
extern std::vector<NPC *> AIpreaddr;

void destroyEverything(void){
	//FILE *save;
	
	/*save = fopen("world.dat","wb");
	playerSpawnHill->save(save);
	fclose(save);*/
	
	delete test;
	delete playerSpawnHill;
	
	while(!AIpreload.empty()){
		AIpreload.pop_back();
	}
	while(!AIpreaddr.empty()){
		AIpreaddr.pop_back();
	}
	
	//delete iw;	// segfaults
}
