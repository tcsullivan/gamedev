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

/*
 *	Gens
 */

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
	player->vel.x = 0;
	player->loc.x = callee->loc.x + callee->width;
	ui::dialogBox(player->name,NULL,false,"This hill seems to steep to climb up...");
	callee->alive = true;
}

static Arena *a;
void worldSpawnHill2_infoSprint(Mob *callee){
	
	ui::dialogBox(player->name,":Sure:Nah",false,"This page would like to take you somewhere.");
	ui::waitForDialog();
	switch(ui::dialogOptChosen){
	case 1:
		ui::dialogBox(player->name,NULL,true,"Cool.");
		callee->alive = false;
		a = new Arena(currentWorld,player);
		a->setBackground(BG_FOREST);
		a->setBGM("assets/music/embark.wav");
		ui::toggleWhiteFast();
		ui::waitForCover();
		currentWorld = a;
		ui::toggleWhiteFast();
		break;
	case 2:
	default:
		ui::dialogBox(player->name,NULL,false,"Okay then.");
		break;
	}
	
	//ui::dialogBox("B-) ",NULL,true,"Press \'Shift\' to run!");
}

int worldSpawnHill2_Quest2(NPC *callee){
	ui::dialogBox(callee->name,NULL,false,"Yo.");
	ui::waitForDialog();
	return 0;
}

int worldSpawnHill2_Quest1(NPC *callee){
	ui::dialogBox(callee->name,":Cool.",false,"Did you know that I\'m the coolest NPC in the world?");
	ui::waitForDialog();
	if(ui::dialogOptChosen == 1){
		ui::dialogBox(callee->name,NULL,false,"Yeah, it is.");
		currentWorld->getAvailableNPC()->addAIFunc(worldSpawnHill2_Quest2,true);
		ui::waitForDialog();
		return 0;
	}
	return 1;
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

static World *worldFirstVillage;
/*
 *	initEverything() start
 */

void destroyEverything(void);
void initEverything(void){
	//static std::ifstream i ("world.dat",std::ifstream::in | std::ifstream::binary);
	
	worldSpawnHill1 = new World();
	worldSpawnHill1->setBackground(BG_FOREST);
	/*if(!i.fail()){
		worldSpawnHill1->load(&i);
		i.close();
	}else{*/
		worldSpawnHill1->generateFunc(400,gen_worldSpawnHill1);
		worldSpawnHill1->setBGM("assets/music/embark.wav");
	//}
	worldSpawnHill1->addMob(MS_TRIGGER,0,0,worldSpawnHill1_hillBlock);
	worldSpawnHill1->addNPC(300,100);

	worldSpawnHill2 = new World();
	worldSpawnHill2->setBackground(BG_FOREST);
	worldSpawnHill2->setBGM("assets/music/ozone.wav");
	worldSpawnHill2->generate(700);
	worldSpawnHill2->addMob(MS_PAGE,-400,0,worldSpawnHill2_infoSprint);
	
	worldSpawnHill3 = new World();
	worldSpawnHill3->generateFunc(1000,gen_worldSpawnHill3);
	worldSpawnHill3->setBackground(BG_FOREST);
	worldSpawnHill3->setBGM("assets/music/ozone.wav");
	
	worldFirstVillage = new World();
	worldFirstVillage->setBackground(BG_FOREST);
	worldFirstVillage->setBGM("assets/music/embark.wav");
	worldFirstVillage->generate(1000);
	
	worldSpawnHill1->toRight = worldSpawnHill2;
	worldSpawnHill2->toLeft = worldSpawnHill1;
	worldSpawnHill2->toRight = worldSpawnHill3;
	worldSpawnHill3->toLeft = worldSpawnHill2;
	worldSpawnHill3->toRight = worldFirstVillage;
	worldFirstVillage->toLeft = worldSpawnHill3;

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
	worldSpawnHill2->addLight({300,100},{1.0f,1.0f,1.0f});
	worldSpawnHill2->getAvailableNPC()->addAIFunc(worldSpawnHill2_Quest1,false);
	
	worldFirstVillage->addVillage(5,0,0,STRUCTURET,worldSpawnHill2_Building1);
	
	//worldSpawnHill2->addStructure(STRUCTURET,HOUSE,(rand()%120*HLINE),100,worldSpawnHill1,worldSpawnHill2);
	player = new Player();
	player->spawn(200,100);

	currentWorld = worldSpawnHill1;
	currentWorld->bgmPlay(NULL);
	atexit(destroyEverything);
}

extern std::vector<int (*)(NPC *)> AIpreload;
extern std::vector<NPC *> AIpreaddr;

void destroyEverything(void){
	/*static std::ofstream o;
	o.open("world.dat",std::ifstream::binary);
	worldSpawnHill2->save(&o);
	o.close();*/
	
	while(!AIpreload.empty())
		AIpreload.pop_back();
	while(!AIpreaddr.empty())
		AIpreaddr.pop_back();
}
