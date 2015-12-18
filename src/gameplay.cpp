#include <common.h>
#include <entities.h>
#include <world.h>
#include <ui.h>

extern World	*currentWorld;
extern Player	*player;

/*
 * 	int (npc*)
 * 
 *	dialog
 *	wait...
 * 
 * 	switch optchosen
 * 
 * 	qh.assign
 *	addAIFunc?
 * 
 * 	return 1 = repeat
 */
 
void story(Mob *callee){
	player->vel.x = 0;
	Mix_FadeOutMusic(0);
	ui::importantText("It was a dark and stormy night...");
	ui::waitForDialog();
	callee->alive = false;
}

float gen_worldSpawnHill1(float x){
	return (float)(pow(2,(-x+200)/5) + GEN_MIN);
}

float gen_worldSpawnHill3(float x){
	float tmp = 60*atan(-(x/30-20))+GEN_MIN*2;
	return tmp>GEN_MIN?tmp:GEN_MIN;
}

/*
 *	Thing-thangs
 */

void worldSpawnHill1_hillBlock(Mob *callee){
	std::cout<<"oi";
	player->vel.x = 0;
	player->loc.x = callee->loc.x + callee->width;
	ui::dialogBox(player->name,NULL,false,"This hill seems to steep to climb up...");
	callee->alive = true;
}

static Arena *a;
void worldSpawnHill2_infoSprint(Mob *callee){
	callee->alive = false;
	a = new Arena(currentWorld,player);
	a->setBackground(BG_FOREST);
	a->setBGM("assets/music/embark.wav");
	ui::toggleWhiteFast();
	ui::waitForCover();
	currentWorld = a;
	ui::toggleWhiteFast();
	//ui::dialogBox("B-) ",NULL,true,"Press \'Shift\' to run!");
}

void worldSpawnHill3_itemGet(Mob *callee){
	ui::dialogBox("B-) ",NULL,true,"Right click to pick up items!");
	callee->alive = false;
}

void worldSpawnHill3_itemSee(Mob *callee){
	ui::dialogBox("B-) ",NULL,true,"Press \'e\' to open your inventory!");
	callee->alive = false;
}

void worldSpawnHill3_leave(Mob *callee){
	ui::dialogBox("B-) ",NULL,true,"Now jump in this hole, and let your journey begin :)");
	callee->alive = false;
}

/*
 *	new world 
 *	gen
 * 	setbackground
 * 	setbgm
 * 	add...
 * 
 */

/*
 *	World definitions
 */

static World *worldSpawnHill1;
static World *worldSpawnHill2;
static World *worldSpawnHill3;

static IndoorWorld *worldSpawnHill2_Building1;

/*
 *	initEverything() start
 */

void destroyEverything(void);
void initEverything(void){

	worldSpawnHill1 = new World();
	worldSpawnHill1->generateFunc(400,gen_worldSpawnHill1);
	worldSpawnHill1->setBackground(BG_FOREST);
	worldSpawnHill1->setBGM("assets/music/embark.wav");
	worldSpawnHill1->addMob(MS_TRIGGER,0,0,worldSpawnHill1_hillBlock);

	worldSpawnHill2 = new World();
	worldSpawnHill2->generate(700);
	worldSpawnHill2->setBackground(BG_FOREST);
	worldSpawnHill2->setBGM("assets/music/ozone.wav");
	worldSpawnHill2->addMob(MS_TRIGGER,-400,0,worldSpawnHill2_infoSprint);

	worldSpawnHill3 = new World();
	worldSpawnHill3->generateFunc(1000,gen_worldSpawnHill3);
	worldSpawnHill3->setBackground(BG_FOREST);
	worldSpawnHill3->setBGM("assets/music/ozone.wav");
	worldSpawnHill3->addMob(MS_TRIGGER,-500,0,worldSpawnHill3_itemGet);
	worldSpawnHill3->addMob(MS_TRIGGER,0,0,worldSpawnHill3_itemSee);
	worldSpawnHill3->addObject(TEST_ITEM,false,"",-200,300);
	worldSpawnHill3->addMob(MS_TRIGGER,650,0,worldSpawnHill3_leave);
	worldSpawnHill3->addHole(800,1000);
	
	worldSpawnHill1->toRight = worldSpawnHill2;
	worldSpawnHill2->toLeft = worldSpawnHill1;
	worldSpawnHill2->toRight = worldSpawnHill3;
	worldSpawnHill3->toLeft = worldSpawnHill2;

	/*
	 *	Spawn some entities.
	*/

	//playerSpawnHill->addMob(MS_TRIGGER,player->loc.x,0,story);

	//playerSpawnHill->addStructure(STRUCTURET,FOUNTAIN,(rand()%120*HLINE)+100*HLINE,100,test,iw);
	//playerSpawnHill->addStructure(STRUCTURET,HOUSE2,(rand()%120*HLINE)+300*HLINE,100,test,iw);

	//playerSpawnHill->addVillage(5,1,4,STRUCTURET,rand()%500+120,(float)200,playerSpawnHill,iw);
	//playerSpawnHill->addMob(MS_TRIGGER,-1300,0,CUTSCENEEE);*/


	worldSpawnHill2_Building1 = new IndoorWorld();
	worldSpawnHill2_Building1->generate(300);
	worldSpawnHill2_Building1->setBackground(BG_WOODHOUSE);
	worldSpawnHill2_Building1->setBGM("assets/music/theme_jazz.wav");

	worldSpawnHill2->addStructure(STRUCTURET,HOUSE,(rand()%120*HLINE),100,worldSpawnHill2_Building1);
	
	player = new Player();
	player->spawn(200,100);

	currentWorld = worldSpawnHill1;	
	currentWorld->bgmPlay(NULL);
	atexit(destroyEverything);
}

extern std::vector<int (*)(NPC *)> AIpreload;
extern std::vector<NPC *> AIpreaddr;

void destroyEverything(void){
	while(!AIpreload.empty())
		AIpreload.pop_back();
	while(!AIpreaddr.empty())
		AIpreaddr.pop_back();
}
