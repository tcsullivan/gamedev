#include <algorithm>
#include <sstream>

#include <world.hpp>
#include <ui.hpp>

#include <tinyxml2.h>
using namespace tinyxml2;

/**
 * Defines how many HLINEs tall a blade of grass can be.
 */

#define GRASS_HEIGHT            4

/**
 * Defines the height of the floor in an IndoorWorld.
 */

#define INDOOR_FLOOR_THICKNESS      50
#define INDOOR_FLOOR_HEIGHTT        400

/**
 * Gravity thing
 */

#define GRAVITY_CONSTANT        0.001f

extern Player *player;						// main.cpp?
extern World  *currentWorld;				// main.cpp
extern World  *currentWorldToLeft;			// main.cpp
extern World  *currentWorldToRight;			// main.cpp
extern int     commonAIFunc(NPC *);			// entities.cpp
extern void    commonTriggerFunc(Mob *);	// gameplay.cpp
extern void    commonPageFunc(Mob *);		// gameplay.cpp
extern bool    inBattle;

extern unsigned int tickCount;				// main.cpp

extern std::string xmlFolder;

int worldShade = 0;

std::string currentXML;

std::vector<std::string> inside;		// tracks indoor worlds

std::vector<World *>     battleNest;	// tracks arenas
std::vector<vec2>        battleNestLoc;	// keeps arena locations

/**
 * Contains the current weather, used in many other places/files.
 */

WorldWeather weather = WorldWeather::Sunny;

const std::string bgPaths[2][9]={
    {"bg.png",					// Daytime background
     "bgn.png",					// Nighttime background
     "bgFarMountain.png",		// Furthest layer
     "forestTileFar.png",		// Furthest away Tree Layer
     "forestTileBack.png",		// Closer layer
     "forestTileMid.png",		// Near layer
     "forestTileFront.png",		// Closest layer
     "dirt.png",				// Dirt
     "grass.png"},				// Grass
    {"bgWoodTile.png",
     "bgWoodTile.png",
     "bgWoodTile.png",
     "bgWoodTile.png",
     "bgWoodTile.png",
     "bgWoodTile.png",
     "bgWoodTile.png",
     "bgWoodTile.png"}
};

const std::string buildPaths[] = {
    "townhall.png",
	"house1.png",
    "house2.png",
    "house1.png",
    "house1.png",
    "fountain1.png",
    "lampPost1.png",
	"brazzier.png"
};

/**
 * Constants used for layer drawing in World::draw(), releated to transparency.
 */

const float bgDraw[4][3]={
	{ 100, 240, 0.6  },
	{ 150, 250, 0.4  },
	{ 200, 255, 0.25 },
	{ 255, 255, 0.1  }
};

/**
 * Sets the desired theme for the world's background.
 *
 * The images chosen for the background layers are selected depending on the
 * world's background type.
 */

void World::
setBackground( WorldBGType bgt )
{
    // load textures with a limit check
	switch ( (bgType = bgt) ) {
	case WorldBGType::Forest:
		bgTex = new Texturec( bgFiles );
		break;

	case WorldBGType::WoodHouse:
		bgTex = new Texturec( bgFilesIndoors );
		break;

    default:
        UserError( "Invalid world background type" );
        break;
	}
}

/**
 * Sets the world's style.
 *
 * The world's style will determine what sprites are used for things like\
 * generic structures.
 */

void World::
setStyle( std::string pre )
{
    unsigned int i;

    // get folder prefix
	std::string prefix = pre.empty() ? "assets/style/classic/" : pre;

	for ( i = 0; i < arrAmt(buildPaths); i++ )
		sTexLoc.push_back( prefix + buildPaths[i] );

	prefix += "bg/";

	for ( i = 0; i < arrAmt(bgPaths[0]); i++ )
		bgFiles.push_back( prefix + bgPaths[0][i] );

	for ( i = 0; i < arrAmt(bgPaths[1]); i++ )
		bgFilesIndoors.push_back( prefix + bgPaths[1][i] );
}

/**
 * Creates a world object.
 *
 * Note that all this does is nullify pointers, to prevent as much disaster as
 * possible. Functions like setBGM(), setStyle() and generate() should be called
 * before the World is actually put into use.
 */

World::
World( void )
{
    bgmObj = NULL;
}

/**
 * The entity vector destroyer.
 *
 * This function will free all memory used by all entities, and then empty the
 * vectors they were stored in.
 */

void World::
deleteEntities( void )
{
    // free mobs
	while ( !mob.empty() ) {
		delete mob.back();
		mob.pop_back();
	}

	merchant.clear();
	while(!npc.empty()){
		delete npc.back();
		npc.pop_back();
	}

    // free structures
	while ( !build.empty() ) {
		delete build.back();
		build.pop_back();
	}

    // free objects
	while ( !object.empty() ) {
		delete object.back();
		object.pop_back();
	}

    // clear entity array
	entity.clear();

    // free particles
	particles.clear();

    // clear light array
	light.clear();

    // free villages
	while ( !village.empty() ) {
		delete village.back();
		village.pop_back();
	}
}

/**
 * The world destructor.
 *
 * This will free objects used by the world itself, then free the vectors of
 * entity-related objects.
 */

World::
~World( void )
{
    // sdl2_mixer's object
	if(bgmObj)
		Mix_FreeMusic(bgmObj);

	delete bgTex;
	deleteEntities();
}

/**
 * Generates a world of the specified width.
 *
 * This will mainly populate the WorldData array, mostly preparing the World
 * object for usage.
 */

void World::
generate( unsigned int width )
{
    // iterator for `for` loops
	std::vector<WorldData>::iterator wditer;

    // see below for description
    float geninc = 0;

    // check for valid width
    if ( (int)width <= 0 )
        UserError("Invalid world dimensions");

    // allocate space for world
    worldData = std::vector<WorldData> (width + GROUND_HILLINESS, WorldData { false, {0,0}, 0, 0 });
    lineCount = worldData.size();

    // prepare for generation
    worldData.front().groundHeight = GROUND_HEIGHT_INITIAL;
    wditer = worldData.begin();

    // give every GROUND_HILLINESSth entry a groundHeight value
    for(unsigned int i = GROUND_HILLINESS; i < worldData.size(); i += GROUND_HILLINESS, wditer += GROUND_HILLINESS)
        worldData[i].groundHeight = (*wditer).groundHeight + (randGet() % 8 - 4);

    // create slopes from the points that were just defined, populate the rest of the WorldData structure

    for(wditer = worldData.begin(); wditer != worldData.end(); wditer++){
        if ((*wditer).groundHeight)
			// wditer + GROUND_HILLINESS can go out of bounds (invalid read)
            geninc = ( (*(wditer + GROUND_HILLINESS)).groundHeight - (*wditer).groundHeight ) / (float)GROUND_HILLINESS;
        else
            (*wditer).groundHeight = (*(wditer - 1)).groundHeight + geninc;

        (*wditer).groundColor    = randGet() % 32 / 8;
        (*wditer).grassUnpressed = true;
        (*wditer).grassHeight[0] = (randGet() % 16) / 3 + 2;
        (*wditer).grassHeight[1] = (randGet() % 16) / 3 + 2;

        // bound checks
        if ( (*wditer).groundHeight < GROUND_HEIGHT_MINIMUM )
            (*wditer).groundHeight = GROUND_HEIGHT_MINIMUM;
        else if ( (*wditer).groundHeight > GROUND_HEIGHT_MAXIMUM )
			(*wditer).groundHeight = GROUND_HEIGHT_MAXIMUM;

		if( (*wditer).groundHeight <= 0 )
			(*wditer).groundHeight = GROUND_HEIGHT_MINIMUM;

    }

    // define x-coordinate of world's leftmost 'line'
    worldStart = (width - GROUND_HILLINESS) * HLINE / 2 * -1;

    // create empty star array, should be filled here as well...
	star = std::vector<vec2> (100, vec2 { 0, 400 } );
	for ( auto &s : star ) {
		s.x = (getRand() % (-worldStart * 2)) + worldStart;
		s.y = (getRand() % SCREEN_HEIGHT) + 100.0f;
	}
}

/**
 * Updates all entity and player coordinates with their velocities.
 *
 * Also handles music fading, although that could probably be placed elsewhere.
 */

void World::
update( Player *p, unsigned int delta )
{
    // update player coords
	p->loc.y += p->vel.y			 * delta;
	p->loc.x +=(p->vel.x * p->speed) * delta;

	if ( p->loc.y > 5000 )
        UserError("Too high for me m8.");

	// update entity coords
	for ( auto &e : entity ) {

        // dont let structures move?
		if ( e->type != STRUCTURET && e->canMove ) {
			e->loc.x += e->vel.x * delta;
            e->loc.y += e->vel.y * delta;

            // update boolean directions
			if ( e->vel.x < 0 )
                e->left = true;
	   		else if ( e->vel.x > 0 )
                e->left = false;
		} else if ( e->vel.y < 0 )
            e->loc.y += e->vel.y * delta;
	}
    // iterate through particles
    particles.erase( std::remove_if( particles.begin(), particles.end(), [&delta](Particles &part){return part.kill(delta);}), particles.end());
    for ( auto part = particles.begin(); part != particles.end(); part++ ) {
		if ( (*part).canMove ) {
			(*part).loc.y += (*part).vely * delta;
			(*part).loc.x += (*part).velx * delta;

			for ( auto &b : build ) {
				if ( b->bsubtype == FOUNTAIN ) {
					if ( (*part).loc.x >= b->loc.x && (*part).loc.x <= b->loc.x + b->width ) {
						if ( (*part).loc.y <= b->loc.y + b->height * .25)
							particles.erase( part );

					}
				}
			}
		}
	}

    // handle music fades
	if ( ui::dialogImportant ) {
		//Mix_FadeOutMusic(2000);
	} else if( !Mix_PlayingMusic() )
		Mix_FadeInMusic(bgmObj,-1,2000);
}

/**
 * Set the world's BGM.
 *
 * This will load a sound file to be played while the player is in this world.
 * If no file is found, no music should play.
 */

void World::
setBGM( std::string path )
{
	if( !path.empty() )
		bgmObj = Mix_LoadMUS( (bgm = path).c_str() );
}

/**
 * Toggle play/stop of the background music.
 *
 * If new music is to be played a crossfade will occur, otherwise... uhm.
 */

void World::
bgmPlay( World *prev ) const
{
	if ( prev ) {
		if ( bgm != prev->bgm ) {
            // new world, new music
			Mix_FadeOutMusic( 800 );
			Mix_PlayMusic( bgmObj, -1 );
		}
	} else {
        // first call
		Mix_FadeOutMusic( 800 );
		Mix_PlayMusic( bgmObj, -1 );
	}
}

/**
 * The world draw function.
 *
 * This function will draw the background layers, entities, and player to the
 * screen.
 */

void World::draw(Player *p){
    // iterators
    int i, iStart, iEnd;

    // shade value for draws -- may be unnecessary
	int shadeBackground = -worldShade;

    // player's offset in worldData[]
	int pOffset;

    // world width in pixels
	int width = worldData.size() * HLINE;

	// shade value for GLSL
	float shadeAmbient = -worldShade / 50.0f + 0.5f; // -0.5f to 1.5f
	if ( shadeAmbient < 0 )
		shadeAmbient = 0;
	else if ( shadeAmbient > 0.9f )
		shadeAmbient = 1;

	/*
     * Draw background images.
     */

	glEnable( GL_TEXTURE_2D );

	// the sunny wallpaper is faded with the night depending on tickCount

	bgTex->bind( 0 );
	safeSetColorA( 255, 255, 255, weather == WorldWeather::Snowy ? 150 : 255 - worldShade * 4);

	glBegin( GL_QUADS );
		glTexCoord2i( 0, 0 ); glVertex2i( offset.x - SCREEN_WIDTH/2-5, SCREEN_HEIGHT );
		glTexCoord2i( 1, 0 ); glVertex2i( offset.x + SCREEN_WIDTH/2+5, SCREEN_HEIGHT );
		glTexCoord2i( 1, 1 ); glVertex2i( offset.x + SCREEN_WIDTH/2+5, 0 );
		glTexCoord2i( 0, 1 ); glVertex2i( offset.x - SCREEN_WIDTH/2-5, 0 );
	glEnd();

	bgTex->bindNext();
	safeSetColorA( 255, 255, 255, worldShade * 4);

	glBegin( GL_QUADS );
		glTexCoord2i( 0, 0 ); glVertex2i(  worldStart, SCREEN_HEIGHT );
		glTexCoord2i( 1, 0 ); glVertex2i( -worldStart, SCREEN_HEIGHT );
		glTexCoord2i( 1, 1 ); glVertex2i( -worldStart, 0 );
		glTexCoord2i( 0, 1 ); glVertex2i(  worldStart, 0 );
	glEnd();

	glDisable( GL_TEXTURE_2D );

	// draw the stars if the time deems it appropriate

	//if (((( weather == WorldWeather::Dark  ) & ( tickCount % DAY_CYCLE )) < DAY_CYCLE / 2)   ||
	//    ((( weather == WorldWeather::Sunny ) & ( tickCount % DAY_CYCLE )) > DAY_CYCLE * .75) ){
	if ( worldShade > 0 ) {

		safeSetColorA( 255, 255, 255, 255 - (getRand() % 30 - 15) );

		for ( i = 0; i < 100; i++ ) {
			glRectf(star[i].x + offset.x * .9,
					star[i].y,
					star[i].x + offset.x * .9 + HLINE,
					star[i].y + HLINE
					);
		}
	}

	// draw remaining background items

	glEnable( GL_TEXTURE_2D );

	bgTex->bindNext();
	safeSetColorA( 150 + shadeBackground * 2, 150 + shadeBackground * 2, 150 + shadeBackground * 2, 255 );

	glBegin( GL_QUADS );
		for ( i = 0; i <= (int)(worldData.size() * HLINE / 1920); i++ ) {
			glTexCoord2i( 0, 1 ); glVertex2i( width / 2 * -1 + (1920 * i      ) + offset.x * .85, GROUND_HEIGHT_MINIMUM        );
			glTexCoord2i( 1, 1 ); glVertex2i( width / 2 * -1 + (1920 * (i + 1)) + offset.x * .85, GROUND_HEIGHT_MINIMUM        );
			glTexCoord2i( 1, 0 ); glVertex2i( width / 2 * -1 + (1920 * (i + 1)) + offset.x * .85, GROUND_HEIGHT_MINIMUM + 1080 );
			glTexCoord2i( 0, 0 ); glVertex2i( width / 2 * -1 + (1920 * i      ) + offset.x * .85, GROUND_HEIGHT_MINIMUM + 1080 );
		}
	glEnd();

	for ( i = 0; i < 4; i++ ) {
		bgTex->bindNext();
		safeSetColorA( bgDraw[i][0] + shadeBackground * 2, bgDraw[i][0] + shadeBackground * 2, bgDraw[i][0] + shadeBackground * 2, bgDraw[i][1] );

		glBegin( GL_QUADS );
			for( int j = worldStart; j <= -worldStart; j += 600 ){
				glTexCoord2i( 0, 1 ); glVertex2i(  j        + offset.x * bgDraw[i][2], GROUND_HEIGHT_MINIMUM       );
				glTexCoord2i( 1, 1 ); glVertex2i( (j + 600) + offset.x * bgDraw[i][2], GROUND_HEIGHT_MINIMUM       );
				glTexCoord2i( 1, 0 ); glVertex2i( (j + 600) + offset.x * bgDraw[i][2], GROUND_HEIGHT_MINIMUM + 400 );
				glTexCoord2i( 0, 0 ); glVertex2i(  j        + offset.x * bgDraw[i][2], GROUND_HEIGHT_MINIMUM + 400 );
			}
		glEnd();
	}

	glDisable( GL_TEXTURE_2D );

	// draw black under backgrounds

	glColor3ub( 0, 0, 0 );
	glRectf( worldStart, GROUND_HEIGHT_MINIMUM, -worldStart, 0 );

	pOffset = (offset.x + p->width / 2 - worldStart) / HLINE;

    /*
     * Prepare for world ground drawing.
     */

	// only draw world within player vision

	if ((iStart = pOffset - (SCREEN_WIDTH / 2 / HLINE) - GROUND_HILLINESS) < 0)
		iStart = 0;

	if ((iEnd = pOffset + (SCREEN_WIDTH / 2 / HLINE) + GROUND_HILLINESS + HLINE) > (int)worldData.size())
		iEnd = worldData.size();
	else if (iEnd < GROUND_HILLINESS)
		iEnd = GROUND_HILLINESS;

	// draw particles and buildings

	std::for_each( particles.begin(), particles.end(), [](Particles part) { if ( part.behind ) part.draw(); });

	for ( auto &b : build )
		b->draw();

	// draw light elements?

	glEnable( GL_TEXTURE_2D );

	glActiveTexture( GL_TEXTURE0 );
	bgTex->bindNext();

    for(auto &l : light){
        if(l.belongsTo){
            l.loc.x = l.following->loc.x + SCREEN_WIDTH/2;
            l.loc.y = l.following->loc.y;
        }
        if(l.flame){
            l.fireFlicker = .9+((rand()%2)/10.0f);
            l.fireLoc.x = l.loc.x + (rand()%2-1)*3;
            l.fireLoc.y = l.loc.y + (rand()%2-1)*3;
        }else{
            l.fireFlicker = 1.0f;
        }
    }

    std::unique_ptr<GLfloat[]> pointArrayBuf = std::make_unique<GLfloat[]> (2 * (light.size()));
	auto pointArray = pointArrayBuf.get();
    GLfloat flameArray[64];

	for (uint i = 0; i < light.size(); i++) {
        if(light[i].flame){
    		pointArray[2 * i    ] = light[i].fireLoc.x - offset.x;
    		pointArray[2 * i + 1] = light[i].fireLoc.y;
        }else{
            pointArray[2 * i    ] = light[i].loc.x - offset.x;
            pointArray[2 * i + 1] = light[i].loc.y;
        }
	}

    for(uint i = 0; i < light.size(); i++){
        flameArray[i] = light[i].fireFlicker;
    }

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	glUseProgram( shaderProgram );
	glUniform1i( glGetUniformLocation( shaderProgram, "sampler"), 0 );
	glUniform1f( glGetUniformLocation( shaderProgram, "amb"    ), shadeAmbient );

	if ( light.size() == 0)
		glUniform1i( glGetUniformLocation( shaderProgram, "numLight"), 0);
	else {
		glUniform1i ( glGetUniformLocation( shaderProgram, "numLight"     ), light.size());
		glUniform2fv( glGetUniformLocation( shaderProgram, "lightLocation"), light.size(), pointArray );
		glUniform3f ( glGetUniformLocation( shaderProgram, "lightColor"   ), 1.0f, 1.0f, 1.0f );
        glUniform1fv(glGetUniformLocation(shaderProgram,"fireFlicker"), light.size(),flameArray);
	}

    /*
     * Draw the dirt.
     */

	glBegin( GL_QUADS );

        // faulty
        /*glTexCoord2i(0 ,0);glVertex2i(pOffset - (SCREEN_WIDTH / 1.5),0);
        glTexCoord2i(64,0);glVertex2i(pOffset + (SCREEN_WIDTH / 1.5),0);
        glTexCoord2i(64,1);glVertex2i(pOffset + (SCREEN_WIDTH / 1.5),GROUND_HEIGHT_MINIMUM);
        glTexCoord2i(0 ,1);glVertex2i(pOffset - (SCREEN_WIDTH / 1.5),GROUND_HEIGHT_MINIMUM);*/

        for ( i = iStart; i < iEnd; i++ ) {
            if ( worldData[i].groundHeight <= 0 ) {
                worldData[i].groundHeight = GROUND_HEIGHT_MINIMUM - 1;
                glColor4ub( 0, 0, 0, 255 );
            } else
                safeSetColorA( 150, 150, 150, 255 );

            glTexCoord2i( 0, 0 ); glVertex2i(worldStart + i * HLINE         , worldData[i].groundHeight - GRASS_HEIGHT );
            glTexCoord2i( 1, 0 ); glVertex2i(worldStart + i * HLINE + HLINE , worldData[i].groundHeight - GRASS_HEIGHT );

            glTexCoord2i( 1, (int)(worldData[i].groundHeight / 64) + worldData[i].groundColor ); glVertex2i(worldStart + i * HLINE + HLINE, 0 );
            glTexCoord2i( 0, (int)(worldData[i].groundHeight / 64) + worldData[i].groundColor ); glVertex2i(worldStart + i * HLINE	      , 0 );

            if ( worldData[i].groundHeight == GROUND_HEIGHT_MINIMUM - 1 )
                worldData[i].groundHeight = 0;
        }

	glEnd();

	glUseProgram(0);
	glDisable(GL_TEXTURE_2D);

	/*
	 *	Draw the grass/the top of the ground.
	 */

	glEnable( GL_TEXTURE_2D );

	glActiveTexture( GL_TEXTURE0 );
	bgTex->bindNext();

	glUseProgram( shaderProgram );
	glUniform1i( glGetUniformLocation( shaderProgram, "sampler"), 0);

	float cgh[2];
	for ( i = iStart; i < iEnd - GROUND_HILLINESS; i++ ) {

		// load the current line's grass values
		if ( worldData[i].groundHeight )
			memcpy( cgh, worldData[i].grassHeight, 2 * sizeof( float ));
		else
			memset( cgh, 0 , 2 * sizeof( float ));

		// flatten the grass if the player is standing on it.
		if( !worldData[i].grassUnpressed ){
			cgh[0] /= 4;
			cgh[1] /= 4;
		}

		// actually draw the grass.

		safeSetColorA( 255, 255, 255, 255 );

		glBegin( GL_QUADS );
			glTexCoord2i( 0, 0 ); glVertex2i( worldStart + i * HLINE            , worldData[i].groundHeight + cgh[0] );
			glTexCoord2i( 1, 0 ); glVertex2i( worldStart + i * HLINE + HLINE / 2, worldData[i].groundHeight + cgh[0] );
			glTexCoord2i( 1, 1 ); glVertex2i( worldStart + i * HLINE + HLINE / 2, worldData[i].groundHeight - GRASS_HEIGHT );
			glTexCoord2i( 0, 1 ); glVertex2i( worldStart + i * HLINE		    , worldData[i].groundHeight - GRASS_HEIGHT );
			glTexCoord2i( 0, 0 ); glVertex2i( worldStart + i * HLINE + HLINE / 2, worldData[i].groundHeight + cgh[1] );
			glTexCoord2i( 1, 0 ); glVertex2i( worldStart + i * HLINE + HLINE    , worldData[i].groundHeight + cgh[1] );
			glTexCoord2i( 1, 1 ); glVertex2i( worldStart + i * HLINE + HLINE    , worldData[i].groundHeight - GRASS_HEIGHT );
			glTexCoord2i( 0, 1 ); glVertex2i( worldStart + i * HLINE + HLINE / 2, worldData[i].groundHeight - GRASS_HEIGHT );
		glEnd();
	}

	glUseProgram(0);
	glDisable(GL_TEXTURE_2D);

	/*
     * Draw remaining entities.
     */

	std::for_each( particles.begin(), particles.end(), [](Particles part) { if ( !part.behind ) part.draw(); });

	for ( auto &n : npc )
		n->draw();

	for ( auto &m : mob )
		m->draw();

	for ( auto &o : object )
		o->draw();

    /*
     * Handle grass-squishing.
     */

	// calculate the line that the player is on
	int ph = ( p->loc.x + p->width / 2 - worldStart ) / HLINE;

	// flatten grass under the player if the player is on the ground
	if ( p->ground ) {
		for ( i = 0; i < (int)(worldData.size() - GROUND_HILLINESS); i++ )
			worldData[i].grassUnpressed = !( i < ph + 6 && i > ph - 6 );
	} else {
		for ( i = 0; i < (int)(worldData.size() - GROUND_HILLINESS); i++ )
			worldData[i].grassUnpressed = true;
	}

	/*
     * Draw the player.
     */

	p->draw();
}

/**
 * Handles physics and such for a single entity.
 *
 * This function is kept private, as World::detect() should be used instead to
 * handle stuffs for all entities at once.
 */

void World::
singleDetect( Entity *e )
{
	std::string killed;
	unsigned int i,j;
	int l;

	/*
	 *	Kill any dead entities.
	*/

	if ( !e->alive || e->health <= 0 ) {
		for ( i = 0; i < entity.size(); i++) {
			if ( entity[i] == e ){
				switch ( e->type ) {
				case STRUCTURET:
					killed = "structure";
					for(j=0;j<build.size();j++){
						if(build[j]==e){
							delete build[j];
							build.erase(build.begin()+j);
							break;
						}
					}
					break;
				case NPCT:
					killed = "NPC";
					for(j=0;j<npc.size();j++){
						if(npc[j]==e){
							delete npc[j];
							npc.erase(npc.begin()+j);
							break;
						}
					}
					break;
				case MOBT:
					killed = "mob";
					/*for(j=0;j<mob.size();j++){
						if(mob[j]==e){
							delete mob[j];
							mob.erase(mob.begin()+j);
							break;
						}
					}*/
					break;
				case OBJECTT:
					killed = "object";
					for(j=0;j<object.size();j++){
						if(object[j]==e){
							delete object[j];
							object.erase(object.begin()+j);
							break;
						}
					}
					break;
				default:
					break;
				}
				std::cout << "Killed a " << killed << "..." << std::endl;
				entity.erase(entity.begin()+i);
				return;
			}
		}
		std::cout << "RIP " << e->name << "." << std::endl;
		exit( 0 );
	}

	// handle only living entities
	if ( e->alive ) {
		if ( e->type == MOBT && Mobp(e)->subtype == MS_TRIGGER )
			return;

		// calculate the line that this entity is currently standing on
		l = (e->loc.x + e->width / 2 - worldStart) / HLINE;
		if ( l < 0 )
            l = 0;
		i = l;
		if ( i > lineCount - 1 )
            i = lineCount - 1;

		// if the entity is under the world/line, pop it back to the surface
		if ( e->loc.y < worldData[i].groundHeight ) {
            int dir = e->vel.x < 0 ? -1 : 1;
            if ( worldData[i + (dir * 8)].groundHeight - 30 > worldData[i + dir].groundHeight ) {
                e->loc.x -= ( PLAYER_SPEED_CONSTANT + 2.7 ) * e->speed * 2 * dir;
                e->vel.x = 0;
            } else {
                e->loc.y = worldData[i].groundHeight - .001 * deltaTime;
		        e->ground = true;
		        e->vel.y = 0;
            }

		}

        // handle gravity if the entity is above the line
        else {

			if ( e->type == STRUCTURET ) {
				e->loc.y = worldData[i].groundHeight;
				e->vel.y = 0;
				e->ground = true;
				return;
			} else if ( e->vel.y > -2 )
                e->vel.y -= GRAVITY_CONSTANT * deltaTime;
		}

		/*
		 *	Insure that the entity doesn't fall off either edge of the world.
		*/

		if(e->loc.x < worldStart){												// Left bound
			e->vel.x=0;
			e->loc.x=(float)worldStart + HLINE / 2;
		}else if(e->loc.x + e->width + HLINE > worldStart + worldStart * -2){	// Right bound
			e->vel.x=0;
			e->loc.x=worldStart + worldStart * -2 - e->width - HLINE;
		}
	}
}

/**
 * Handle entity logic for the world.
 *
 * This function runs World::singleDetect() for the player and every entity
 * currently in a vector of this world. Particles and village entrance/exiting
 * are also handled here.
 */

void World::
detect( Player *p )
{
	int l;

	// handle the player
	std::thread( &World::singleDetect, this, p).detach();

    // handle other entities
	for ( auto &e : entity )
		std::thread(&World::singleDetect,this,e).detach();

    // handle particles
	for ( auto &part : particles ) {

		// get particle's current world line
		l = (part.loc.x + part.width / 2 - worldStart) / HLINE;

		if ( l < 0 )
			l = 0;

		if ( l > (int)(lineCount - 1) )
			l = lineCount - 1;

		// handle ground collision
		if ( part.loc.y < worldData[l].groundHeight ) {
			part.loc.y = worldData[l].groundHeight;
			part.vely = 0;
			part.velx = 0;
			part.canMove = false;
		} else if ( part.gravity && part.vely > -2 )
			part.vely -= GRAVITY_CONSTANT * deltaTime;
	}

	// handle particle creation
	for ( auto &b : build ) {
		switch ( b->bsubtype ) {
		case FOUNTAIN:
			for ( unsigned int r = (randGet() % 25) + 11; r--; ) {
				addParticle(randGet() % HLINE * 3 + b->loc.x + b->width / 2,	// x
							b->loc.y + b->height,								// y
							HLINE * 1.25,										// width
							HLINE * 1.25,										// height
							randGet() % 7 * .01 * (randGet() % 2 == 0 ? -1 : 1),	// vel.x
							(4 + randGet() % 6) * .05,							// vel.y
							{ 0, 0, 255 },										// RGB color
							2500												// duration (ms)
							);

				particles.back().fountain = true;
			}
			break;

		case FIRE_PIT:
			for(unsigned int r = (randGet() % 20) + 11; r--; ) {
				addParticle(randGet() % (int)(b->width / 2) + b->loc.x + b->width / 4,	// x
							b->loc.y + 3 * HLINE,										// y
							HLINE,														// width
							HLINE,														// height
							randGet() % 3 * .01 * (randGet() % 2 == 0 ? -1 : 1),		// vel.x
							(4 + randGet() % 6) * .005,									// vel.y
							{ 255, 0, 0 },												// RGB color
							400															// duration (ms)
							);

				particles.back().gravity = false;
				particles.back().behind  = true;
			}
			break;

		default:
			break;
		}
	}

	// draws the village welcome message if the player enters the village bounds
	for ( auto &v : village ) {
		if ( p->loc.x > v->start.x && p->loc.x < v->end.x ) {
			if ( !v->in ) {
				ui::passiveImportantText( 5000, "Welcome to %s", v->name.c_str() );
				v->in = true;
			}
		} else
			v->in = false;
	}
}

void World::addStructure(BUILD_SUB sub, float x,float y, std::string tex, std::string inside){
	build.push_back(new Structures());
	build.back()->inWorld = this;
	build.back()->textureLoc = tex;

	build.back()->spawn(sub,x,y);

	build.back()->inside = inside;

	entity.push_back(build.back());
}

void World::addMob(int t,float x,float y){
	mob.push_back(new Mob(t));
	mob.back()->spawn(x,y);

	entity.push_back(mob.back());
}

void World::addMob(int t,float x,float y,void (*hey)(Mob *)){
	mob.push_back(new Mob(t));
	mob.back()->spawn(x,y);
	mob.back()->hey = hey;

	entity.push_back(mob.back());
}

void World::addNPC(float x,float y){
	npc.push_back(new NPC());
	npc.back()->spawn(x,y);

	entity.push_back(npc.back());
}

void World::addMerchant(float x, float y){
	merchant.push_back(new Merchant());
	merchant.back()->spawn(x,y);

	npc.push_back(merchant.back());
	entity.push_back(npc.back());
}

void World::addObject( std::string in, std::string p, float x, float y){
	object.push_back(new Object(in,p));
	object.back()->spawn(x,y);

	entity.push_back(object.back());
}

void World::
addParticle( float x, float y, float w, float h, float vx, float vy, Color color, int d )
{
	particles.emplace_back( x, y, w, h, vx, vy, color, d );
	particles.back().canMove = true;
}

void World::
addParticle( float x, float y, float w, float h, float vx, float vy, Color color, int d, bool gravity )
{
	particles.emplace_back( x, y, w, h, vx, vy, color, d );
	particles.back().canMove = true;
    particles.back().gravity = gravity;
}

void World::
addLight( vec2 loc, Color color )
{
	if ( light.size() < 64 )
        light.push_back( Light( loc, color, 1 ) );
}

std::string World::
setToLeft( std::string file )
{
    return (toLeft = file);
}

std::string World::
setToRight( std::string file )
{
	return (toRight = file);
}

World *World::
goWorldLeft( Player *p )
{
	World *tmp;

    // check if player is at world edge
	if( !toLeft.empty() && p->loc.x < worldStart + HLINE * 15.0f ) {

        // load world (`toLeft` conditional confirms existance)
	    tmp = loadWorldFromPtr( currentWorldToLeft );

        // adjust player location
		p->loc.x = tmp->worldStart + HLINE * 20;
		p->loc.y = tmp->worldData[tmp->lineCount - 1].groundHeight;

		return tmp;
	}

	return this;
}

bool World::
goWorldLeft( NPC *e )
{
	// check if entity is at world edge
	if( !toLeft.empty() && e->loc.x < worldStart + HLINE * 15.0f ) {

        currentWorldToLeft->addNPC(e->loc.x,e->loc.y);
        e->alive = false;

		currentWorldToLeft->npc.back()->loc.x = 0;
		currentWorldToLeft->npc.back()->loc.y = GROUND_HEIGHT_MAXIMUM;

		return true;
	}

	return false;
}

World *World::
goWorldRight( Player *p )
{
	World *tmp;

	if( !toRight.empty() && p->loc.x + p->width > -worldStart - HLINE * 15 ) {
		tmp = loadWorldFromPtr( currentWorldToRight );

		p->loc.x = tmp->worldStart - HLINE * -15.0f;
		p->loc.y = GROUND_HEIGHT_MINIMUM;

		return tmp;
	}

	return this;
}

World *World::
goInsideStructure( Player *p )
{
	World *tmp;
	std::string current;

	if ( inside.empty() ) {
		for ( auto &b : build ) {
			if ( p->loc.x            > b->loc.x            &&
			     p->loc.x + p->width < b->loc.x + b->width ) {

                if ( b->inside.empty() )
                    return this;

				inside.push_back(currentXML.c_str() + xmlFolder.size());

				tmp = loadWorldFromXML( b->inside );

				ui::toggleBlackFast();
				ui::waitForCover();
				ui::toggleBlackFast();

                glClearColor(0,0,0,1);

				return tmp;
			}
		}
	} else {
        current = currentXML.c_str() + xmlFolder.size();
		tmp = loadWorldFromXML( inside.back() );
		for ( auto &b : tmp->build ) {
			if ( current == b->inside ) {
				inside.pop_back();

				ui::toggleBlackFast();
				ui::waitForCover();

				p->loc.x = b->loc.x + (b->width / 2);

				ui::toggleBlackFast();

                glClearColor(1,1,1,1);

				return tmp;
			}
		}
	}

	return this;
}

void World::
addHole( unsigned int start, unsigned int end )
{
	for ( unsigned int i = end; i-- > start; )
		worldData[i].groundHeight = 0;
}

void World::
addHill( const ivec2 peak, const unsigned int width )
{
	int start = peak.x - width / 2, end = start + width, offset;
	const float thing = peak.y - worldData[start].groundHeight;
    const float period = PI / width;

	if ( start < 0 ) {
        offset = -start;
        start = 0;
    }

	if ( end > (signed)worldData.size() )
	  end = worldData.size();

	for ( int i = start; i < end; i++ ) {
		worldData[i].groundHeight += thing * sin((i - start + offset) * period);
		if ( worldData[i].groundHeight > peak.y )
			worldData[i].groundHeight = peak.y;
	}
}

int World::
getTheWidth( void ) const
{
	return worldStart * -2;
}

void World::save(void){
	std::string data;

	std::string save = (std::string)currentXML + ".dat";
	std::ofstream out (save,std::ios::out | std::ios::binary);

	std::cout<<"Saving to "<<save<<" ..."<<std::endl;

	for(auto &n : npc){
		data.append(std::to_string(n->dialogIndex) + "\n");
		data.append(std::to_string((int)n->loc.x) + "\n");
		data.append(std::to_string((int)n->loc.y) + "\n");
	}

	for(auto &b : build){
		data.append(std::to_string((int)b->loc.x) + "\n");
		data.append(std::to_string((int)b->loc.y) + "\n");
	}

	for(auto &m : mob){
		data.append(std::to_string((int)m->loc.x) + "\n");
		data.append(std::to_string((int)m->loc.y) + "\n");
		data.append(std::to_string((int)m->alive) + "\n");
	}

	data.append("dOnE\0");
	out.write(data.c_str(),data.size());
	out.close();
}

void World::load(void){
	std::string save,data,line;
	const char *filedata;

	save = std::string(currentXML + ".dat");
	filedata = readFile(save.c_str());
	data = filedata;
	std::istringstream iss (data);

	for(auto &n : npc){
		std::getline(iss,line);
		if(line == "dOnE")return;
		if((n->dialogIndex = std::stoi(line)) != 9999)
			n->addAIFunc(commonAIFunc,false);
		else n->clearAIFunc();

		std::getline(iss,line);
		if(line == "dOnE")return;
		n->loc.x = std::stoi(line);
		std::getline(iss,line);
		if(line == "dOnE")return;
		n->loc.y = std::stoi(line);
	}

	for(auto &b : build){
		std::getline(iss,line);
		if(line == "dOnE")return;
		b->loc.x = std::stoi(line);
		std::getline(iss,line);
		if(line == "dOnE")return;
		b->loc.y = std::stoi(line);
	}

	for(auto &m : mob){
		std::getline(iss,line);
		if(line == "dOnE")return;
		m->loc.x = std::stoi(line);
		std::getline(iss,line);
		if(line == "dOnE")return;
		m->loc.y = std::stoi(line);
		std::getline(iss,line);
		if(line == "dOnE")return;
		m->alive = std::stoi(line);
	}

	while(std::getline(iss,line)){
		if(line == "dOnE")
			break;
	}

	delete[] filedata;
}

float getIndoorWorldFloorHeight( void )
{
    return INDOOR_FLOOR_HEIGHTT + INDOOR_FLOOR_THICKNESS;
}

bool isCurrentWorldIndoors( void ) {
    return !inside.empty();
}

IndoorWorld::IndoorWorld(void){
}

IndoorWorld::~IndoorWorld(void){
	delete bgTex;

	deleteEntities();
}

void IndoorWorld::
addFloor( unsigned int width )
{
    if ( floor.empty() )
        generate( width );
    floor.emplace_back( width, floor.size() * INDOOR_FLOOR_HEIGHTT + INDOOR_FLOOR_THICKNESS );
    fstart.push_back( 0 );
}


void IndoorWorld::
addFloor( unsigned int width, unsigned int start )
{
    if ( floor.empty() )
        generate( width );
    floor.emplace_back( width, floor.size() * INDOOR_FLOOR_HEIGHTT + INDOOR_FLOOR_THICKNESS );
    fstart.push_back( start );
}

bool IndoorWorld::
moveToFloor( Entity *e, unsigned int _floor )
{
    if ( _floor > floor.size() )
        return false;

    e->loc.y = floor[_floor - 1][0];
    return true;
}

bool IndoorWorld::
isFloorAbove( Entity *e )
{
    for ( unsigned int i = 0; i < floor.size(); i++ ) {
        if ( floor[i][0] + INDOOR_FLOOR_HEIGHTT - 100 > e->loc.y )
            return (i + 1) != floor.size();
    }
    return false;
}

bool IndoorWorld::
isFloorBelow( Entity *e )
{
    for ( unsigned int i = 0; i < floor.size(); i++ ) {
        if ( floor[i][0] + INDOOR_FLOOR_HEIGHTT - 100 > e->loc.y )
            return i > 0;
    }
    return false;
}

void IndoorWorld::
singleDetect( Entity *e )
{
    unsigned int floornum = 0;
    float start, end;

    if ( !e->alive )
        return;
    if ( e->type == MOBT && Mobp(e)->subtype == MS_TRIGGER )
        return;

    for ( ; floornum < floor.size(); floornum++ ) {
        if ( floor[floornum][0] + INDOOR_FLOOR_HEIGHTT - 100 > e->loc.y ) {
            if ( e->loc.y < floor[floornum][0] ) {
                e->loc.y = floor[floornum][0];
                e->vel.y = 0;
                e->ground = true;
            }
            break;
        }
    }

    if ( e->vel.y > -2 )
        e->vel.y -= GRAVITY_CONSTANT * deltaTime;

    if ( e->ground ) {
        e->loc.y = ceil( e->loc.y );
        e->vel.y = 0;
    }

    start = worldStart + fstart[floornum] * HLINE;
    end = start + floor[floornum].size() * HLINE;

    if ( e->loc.x < start ) {
        e->vel.x = 0;
        e->loc.x = start + HLINE / 2;
    } else if ( e->loc.x + e->width + HLINE > end ) {
        e->vel.x = 0;
        e->loc.x = end - e->width - HLINE;
    }

}

void IndoorWorld::
draw( Player *p )
{
	unsigned int i,f;
	int x;

    // draw lights
    for ( auto &l : light ) {
        if ( l.belongsTo ) {
            l.loc.x = l.following->loc.x + SCREEN_WIDTH / 2;
            l.loc.y = ( l.following->loc.y > SCREEN_HEIGHT / 2 ) ? SCREEN_HEIGHT / 2 : l.following->loc.y;
        }
        if ( l.flame ) {
            l.fireFlicker = .9 + ( (rand() % 2) / 10.0f );
            l.fireLoc.x = l.loc.x + (rand() % 2 - 1) * 3;
            l.fireLoc.y = l.loc.y + (rand() % 2 - 1) * 3;
        } else
            l.fireFlicker = 1.0f;
    }

    std::unique_ptr<GLfloat[]> pointArrayBuf = std::make_unique<GLfloat[]> (2 * (light.size()));
	auto pointArray = pointArrayBuf.get();
    GLfloat flameArray[64];

	for (i = 0; i < light.size(); i++) {
        if(light[i].flame){
    		pointArray[2 * i    ] = light[i].fireLoc.x - offset.x;
    		pointArray[2 * i + 1] = light[i].fireLoc.y;
        }else{
            pointArray[2 * i    ] = light[i].loc.x - offset.x;
            pointArray[2 * i + 1] = light[i].loc.y;
        }
	}

    for(i = 0; i < light.size(); i++)
        flameArray[i] = light[i].fireFlicker;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glUseProgram( shaderProgram );
	glUniform1i(glGetUniformLocation(shaderProgram, "sampler"), 0);
	glUniform1f(glGetUniformLocation(shaderProgram, "amb"    ), 0.02f + light.size()/50.0f);

	if ( light.empty() )
		glUniform1i(glGetUniformLocation(shaderProgram, "numLight"), 0);
	else {
		glUniform1i (glGetUniformLocation(shaderProgram, "numLight"     ), light.size());
		glUniform2fv(glGetUniformLocation(shaderProgram, "lightLocation"), light.size(), pointArray);
		glUniform3f (glGetUniformLocation(shaderProgram, "lightColor"   ), 1.0f, 1.0f, 1.0f);
        glUniform1fv(glGetUniformLocation(shaderProgram, "fireFlicker"), light.size(), flameArray);
	}

	bgTex->bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //for the s direction
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //for the t direction
	glColor4ub(255,255,255,255);

    glBegin(GL_QUADS);
        glTexCoord2i(0,1);							     glVertex2i( worldStart - SCREEN_WIDTH / 2,0);
		glTexCoord2i((-worldStart*2+SCREEN_WIDTH)/512,1);glVertex2i(-worldStart + SCREEN_WIDTH / 2,0);
		glTexCoord2i((-worldStart*2+SCREEN_WIDTH)/512,0);glVertex2i(-worldStart + SCREEN_WIDTH / 2,SCREEN_HEIGHT);
		glTexCoord2i(0,0);							     glVertex2i( worldStart - SCREEN_WIDTH / 2,SCREEN_HEIGHT);
    glEnd();

    glUseProgram(0);

	/*
	 *	Draw the ground.
	*/

	glUseProgram( shaderProgram );
	glUniform1i( glGetUniformLocation(shaderProgram, "sampler"), 0 );
	glBegin( GL_QUADS );
        safeSetColor( 150, 100, 50 );
        for ( f = 0; f < floor.size(); f++ ) {
            i = 0;
    		for ( h : floor[f] ) {
    			x = worldStart + fstart[f] * HLINE + (i * HLINE);
    			glVertex2i( x        , h                          );
    			glVertex2i( x + HLINE, h                          );
    			glVertex2i( x + HLINE, h - INDOOR_FLOOR_THICKNESS );
    			glVertex2i( x        , h - INDOOR_FLOOR_THICKNESS );
                i++;
    		}
        }
	glEnd();
	glUseProgram(0);

	/*
	 *	Draw all entities.
	*/

	for ( auto &part : particles )
		part.draw();

	for ( auto &e : entity )
		e->draw();

	p->draw();
}

Arena::Arena(World *leave,Player *p,Mob *m){
	generate(800);
	addMob(MS_DOOR,100,100);

	inBattle = true;
	mmob = m;
	mmob->aggressive = false;

	mob.push_back(m);
	entity.push_back(m);

	battleNest.push_back(leave);
	battleNestLoc.push_back(p->loc);
}

Arena::~Arena(void){
	deleteEntities();
}

World *Arena::exitArena(Player *p){
	World *tmp;
	if(p->loc.x + p->width / 2 > mob[0]->loc.x				&&
	   p->loc.x + p->width / 2 < mob[0]->loc.x + HLINE * 12 ){
		tmp = battleNest.front();
		battleNest.erase(battleNest.begin());

		inBattle = !battleNest.empty();
		ui::toggleBlackFast();
		ui::waitForCover();

		p->loc = battleNestLoc.back();
		battleNestLoc.pop_back();

		mob.clear();
		mmob->alive = false;

		return tmp;
	}else{
		return this;
	}
}

std::string getWorldWeatherStr( WorldWeather ww )
{
    switch ( ww ) {
    case WorldWeather::Sunny:
        return "Sunny";
        break;
    case WorldWeather::Dark:
        return "Darky";
        break;
    case WorldWeather::Rain:
        return "Rainy";
        break;
    default:
        return "Snowy";
        break;
    }
}

static bool loadedLeft = false;
static bool loadedRight = false;

World *loadWorldFromXML(std::string path){
	if ( !currentXML.empty() )
		currentWorld->save();

	return loadWorldFromXMLNoSave(path);
}

World *loadWorldFromPtr( World *ptr )
{
    World *tmp = ptr;

    loadedLeft = true;
    currentWorldToLeft = loadWorldFromXML( tmp->toLeft );
    loadedLeft = false;

    loadedRight = true;
    currentWorldToRight = loadWorldFromXML( tmp->toRight );
    loadedRight = false;

    return tmp;
}

/**
 * Loads a world from the given XML file.
 */

World *
loadWorldFromXMLNoSave( std::string path ) {
	XMLDocument xml;
	XMLElement *wxml;
	XMLElement *vil;

	World *tmp;
	float spawnx, randx;
	bool dialog,Indoor;
    unsigned int flooor;

	const char *ptr;
	std::string name, sptr;

    // no file? -> no world
    if ( path.empty() )
        return NULL;

	currentXML = std::string(xmlFolder + path);
	xml.LoadFile( currentXML.c_str() );

    // attempt to load a <World> tag
	if ( (wxml = xml.FirstChildElement("World")) ) {
		wxml = wxml->FirstChildElement();
		vil = xml.FirstChildElement("World")->FirstChildElement("village");
		tmp = new World();
        Indoor = false;
	}

    // attempt to load an <IndoorWorld> tag
    else if( (wxml = xml.FirstChildElement("IndoorWorld")) ) {
		wxml = wxml->FirstChildElement();
		vil = NULL;
		tmp = new IndoorWorld();
        Indoor = true;
	}

    // error: can't load a world...
    else
        UserError("XML Error: Cannot find a <World> or <IndoorWorld> tag in " + currentXML + "!");

    // iterate through world tags
	while ( wxml ) {
		name = wxml->Name();

        // world linkage
		if ( name == "link" ) {

            // links world to the left
			if ( (ptr = wxml->Attribute("left")) ) {
				tmp->setToLeft( ptr );

                // load the left world if it isn't
                if ( !loadedLeft ) {
                    loadedLeft = true;
                    currentWorldToLeft = loadWorldFromXMLNoSave( ptr );
                    loadedLeft = false;
                }
			}

            // links world to the right
            else if ( (ptr = wxml->Attribute("right")) ) {
				tmp->setToRight( ptr );

                // load the right world if it isn't
                if ( !loadedRight ) {
                    loadedRight = true;
                    currentWorldToRight = loadWorldFromXMLNoSave( ptr );
                    loadedRight = false;
                }
			}

            // error, invalid link tag
            else
                UserError("XML Error: Invalid <link> tag in " + currentXML + "!");

		}

        // style tags
        else if ( name == "style" ) {
            // set style folder
			tmp->setStyle( wxml->StrAttribute("folder") );

            // set background folder
            if ( wxml->QueryUnsignedAttribute("background", &flooor) != XML_NO_ERROR )
                UserError("XML Error: No background given in <style> in " + currentXML + "!");
			tmp->setBackground( (WorldBGType)flooor );

            // set BGM file
            tmp->setBGM( wxml->StrAttribute("bgm") );
		}

        // world generation (for outdoor areas)
        else if ( name == "generation" ) {
            // random gen.
			if ( !Indoor && wxml->StrAttribute("type") == "Random" )
				tmp->generate( wxml->UnsignedAttribute("width") );
            else {
                if ( Indoor )
                    UserError("XML Error: <generation> tags can't be in <IndoorWorld> tags (in " + currentXML + ")!");
                else
                    UserError("XML Error: Invalid <generation> tag in " + currentXML + "!");
            }
		}

        // mob creation
        else if ( name == "mob" ) {
            // type info
            if ( wxml->QueryUnsignedAttribute("type", &flooor) != XML_NO_ERROR )
                UserError("XML Error: Invalid type value in <mob> in " + currentXML + "!");

            // spawn at coordinate if desired
			if ( wxml->QueryFloatAttribute( "x", &spawnx ) == XML_NO_ERROR )
				tmp->addMob( flooor, spawnx, wxml->FloatAttribute("y"));
			else
				tmp->addMob( flooor, 0, 100 );

            // aggressive tag
			if ( wxml->QueryBoolAttribute( "aggressive", &dialog ) == XML_NO_ERROR )
				tmp->mob.back()->aggressive = dialog;

            // indoor spawning floor selection
            if ( Indoor && wxml->QueryUnsignedAttribute( "floor", &flooor ) == XML_NO_ERROR )
                Indoorp(tmp)->moveToFloor( tmp->npc.back(), flooor );
		}

        // npc creation
        else if ( name == "npc" ) {
			const char *npcname;

            // spawn at coordinates if desired
			if ( wxml->QueryFloatAttribute( "x", &spawnx ) == XML_NO_ERROR)
				tmp->addNPC( spawnx, wxml->FloatAttribute("y") );
			else
				tmp->addNPC( 0, 100 );

            // name override
			if ( (npcname = wxml->Attribute("name")) ) {
                delete[] tmp->npc.back()->name;
				tmp->npc.back()->name = new char[strlen(npcname) + 1];
				strcpy( tmp->npc.back()->name, npcname );
			}

            // dialog enabling
			dialog = false;
			if ( wxml->QueryBoolAttribute( "hasDialog", &dialog ) == XML_NO_ERROR && dialog )
				tmp->npc.back()->addAIFunc( commonAIFunc, false );
			else
                tmp->npc.back()->dialogIndex = 9999;

            if ( Indoor && wxml->QueryUnsignedAttribute( "floor", &flooor ) == XML_NO_ERROR )
                Indoorp(tmp)->moveToFloor( tmp->npc.back(), flooor );
		}

        // structure creation
        else if ( name == "structure" ) {
			tmp->addStructure( (BUILD_SUB) wxml->UnsignedAttribute("type"),
							   wxml->QueryFloatAttribute( "x", &spawnx ) != XML_NO_ERROR ?
							       getRand() % tmp->getTheWidth() / 2.0f : spawnx,
							   100,
							   wxml->StrAttribute("texture"),
							   wxml->StrAttribute("inside")
                             );
		} else if ( name == "trigger" ) {
			tmp->addMob(MS_TRIGGER,wxml->FloatAttribute("x"),0,commonTriggerFunc);
			tmp->mob.back()->heyid = wxml->Attribute("id");
		} else if ( name == "page" ) {
			tmp->addMob( MS_PAGE, wxml->FloatAttribute("x"), 0, commonPageFunc );
			tmp->mob.back()->heyid = wxml->Attribute("id");
		} else if ( name == "hill" ) {
			tmp->addHill( ivec2 { wxml->IntAttribute("peakx"), wxml->IntAttribute("peaky") }, wxml->UnsignedAttribute("width") );
		} else if ( name == "time" ) {
            tickCount = std::stoi( wxml->GetText() );
        } else if ( Indoor && name == "floor" ) {
            if ( wxml->QueryFloatAttribute("start",&spawnx) == XML_NO_ERROR )
                Indoorp(tmp)->addFloor( wxml->UnsignedAttribute("width"), spawnx );
            else
                Indoorp(tmp)->addFloor( wxml->UnsignedAttribute("width") );
        }

		wxml = wxml->NextSiblingElement();
	}

	Village *vptr;

	if(vil){
		tmp->village.push_back(new Village(vil->Attribute("name"), tmp));
		vptr = tmp->village.back();

		vil = vil->FirstChildElement();
	}

	while(vil){
		name = vil->Name();
		randx = 0;
		//static BuySell bs;

		/**
		 * 	READS DATA ABOUT STRUCTURE CONTAINED IN VILLAGE
		 */

		if ( name == "structure" ) {
			tmp->addStructure((BUILD_SUB)vil->UnsignedAttribute("type"),
							   vil->QueryFloatAttribute("x", &spawnx) != XML_NO_ERROR ? randx : spawnx,
							   100,
							   vil->StrAttribute("texture"),
							   vil->StrAttribute("inside"));
		}else if ( name == "stall" ) {
			if(!strcmp(vil->Attribute("type"),"market")){
				tmp->addStructure((BUILD_SUB)70,
							   vil->QueryFloatAttribute("x", &spawnx) != XML_NO_ERROR ?
							   randx : spawnx,
							   100,
							   vil->StrAttribute("texture"),
							   vil->StrAttribute("inside"));
				tmp->addMerchant(0,100);
                tmp->merchant.back()->inside = tmp->build.back();
				if(vil->FirstChildElement("buy")){
				}else if(vil->FirstChildElement("sell")){
				}else if(vil->FirstChildElement("trade")){
					tmp->merchant.back()->trade.push_back(Trade(vil->FirstChildElement("trade")->IntAttribute("quantity"),
																vil->FirstChildElement("trade")->Attribute("item"),
																vil->FirstChildElement("trade")->IntAttribute("quantity1"),
																vil->FirstChildElement("trade")->Attribute("item1")));
					tmp->merchant.back()->trade.push_back(Trade(1,"Wood Sword", 420, "Dank MayMay"));
				}
				strcpy(tmp->merchant.back()->name,"meme");

			}else if(!strcmp(vil->Attribute("type"),"trader")){
				tmp->addStructure((BUILD_SUB)71,
							   vil->QueryFloatAttribute("x", &spawnx) != XML_NO_ERROR ?
							   randx : spawnx,
							   100,
							   vil->StrAttribute("texture"),
							   vil->StrAttribute("inside"));
			}
		}

		vptr->build.push_back(tmp->build.back());

		if(vptr->build.back()->loc.x < vptr->start.x){
			vptr->start.x = vptr->build.back()->loc.x;
		}

		if(vptr->build.back()->loc.x + vptr->build.back()->width > vptr->end.x){
			vptr->end.x = vptr->build.back()->loc.x + vptr->build.back()->width;
		}

		//go to the next element in the village block
		vil = vil->NextSiblingElement();
	}

	std::ifstream dat (((std::string)currentXML + ".dat").c_str());
	if(dat.good()){
		dat.close();
		tmp->load();
	}

	return tmp;
}

Village::Village(const char *meme, World *w){
	name = meme;
	start.x = w->getTheWidth() / 2.0f;
	end.x = -start.x;
	in = false;
}
