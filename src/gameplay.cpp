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

//static World *a;

void CUTSCENEEE(Mob *callee){
	player->vel.x = 0;
	
	ui::dialogBox(player->name,":K then","No way I\'m gettin\' up this hill.");
	ui::waitForDialog();

	//a = new World();//Arena(currentWorld,player);
	//a->generate(300);
	currentWorld = currentWorld->toRight;

	/*player->right = true;
	player->left  = false;
	player->loc.x += HLINE * 5;*/
	
	callee->alive = false;
}

void CUTSCENEEE2(Mob *callee){
	player->vel.x = 0;
	ui::dialogBox(player->name,":Yeah.",
	"What the fuck is this dead end supposed to mean, and why this place smell like soap.");
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
	iw->generate(200);
	iw->addMob(MS_TRIGGER,0,0,CUTSCENEEE2);
	
	/*
	 *	Spawn some entities.
	*/

	playerSpawnHill->addStructure(STRUCTURET,(rand()%120*HLINE),100,test,iw);
	playerSpawnHill->addMob(MS_TRIGGER,-1300,0,CUTSCENEEE);
	
	playerSpawnHill->addObject(SWORD_WOOD, false, "", 480,200);
	playerSpawnHill->addObject(FLASHLIGHT, false, "", 500,200);
	playerSpawnHill->addObject(PLAYER_BAG, false, "", 520,200);
	playerSpawnHill->addObject(TEST_ITEM, false, "", 540,200);
	//playerSpawnHill->addObject(FLASHLIGHT, true, "This looks important, do you want to pick it up?",600,200);
	
	test->addMob(MS_RABBIT,200,100);
	test->addMob(MS_BIRD,-500,500);

	/*currentWorld->addObject(DEBUG_ITEM, 500,200);
	currentWorld->addObject(TEST_ITEM,  550,200);
	currentWorld->addObject(PLAYER_BAG, 600,200);
	currentWorld->addObject(SWORD_WOOD, 650,200);
	currentWorld->addObject(FLASHLIGHT, true, "This looks important, do you want to pick it up?",700,200);
	*/
	
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

/*void story(void){
	for(int i=0;i<600;i++){
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho((offset.x-SCREEN_WIDTH/2),(offset.x+SCREEN_WIDTH/2),offset.y-SCREEN_HEIGHT/2,offset.y+SCREEN_HEIGHT/2,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glEnable(GL_STENCIL_TEST);
		glPushMatrix();

		glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT );
		glClear(GL_COLOR_BUFFER_BIT);

		glColor4f(0.0f,0.0f,0.0f,0.0f);
		glRectf(-SCREEN_WIDTH/2,0,SCREEN_WIDTH/2,SCREEN_HEIGHT);
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		ui::importantText("Oh hello, where are you?");
		//ui::setFontSize(16);
		//ui::putText(54,540,"BITC.");

		glPopMatrix();
		SDL_GL_SwapWindow(window);
	}
}*/
