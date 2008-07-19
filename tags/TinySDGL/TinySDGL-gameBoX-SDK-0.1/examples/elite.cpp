#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../include/GL/gl.h"
#include <SDL/SDL.h>
#include "../include/zfeatures.h"
#include "../include/zbuffer.h"


#define PI 3.141592653
static int timeINTERVAL = 50; //50

static int scrW = 800;
static int scrH = 600;
static int mX, mY, mB;

#include "./includes/font.h"
#include "./includes/random.h"
#include "./includes/definition.h"

struct univ_object universe[MAX_UNIV_OBJECTS];
static Matrix intro_ship_matrix;
static float mm_trailX[MAX_UNIV_OBJECTS][10];
static float mm_trailY[MAX_UNIV_OBJECTS][10];
static float mm_trailZ[MAX_UNIV_OBJECTS][10];
static int mm_t[MAX_UNIV_OBJECTS];
static int mm_current[MAX_UNIV_OBJECTS];

int ship_count[NO_OF_SHIPS + 10];  

char *economy_type[] = {"Rich Industrial",
"Average Industrial",
"Poor Industrial",
"Mainly Industrial",
"Mainly Agricultural",
"Rich Agricultural",
"Average Agricultural",
"Poor Agricultural"};

char *government_type[] = {	"Anarchy",
"Feudal",
"Dictatorship",
"Communist",
"Multi-Government",
"Confederacy",
"Democracy",
"Corporate State"};



static char *unit_name[] = {"t", "kg", "g"};

struct commander saved_cmdr =
{
"JAMESON",									/* Name 			*/
0,											/* Mission Number 	*/
0x14,0xAD,									/* Ship X,Y			*/
{0x4a, 0x5a, 0x48, 0x02, 0x53, 0xb7},		/* Galaxy Seed		*/
1000,										/* Credits * 10		*/
70,											/* Fuel	* 10		*/
0,
0,											/* Galaxy - 1		*/
PULSE_LASER,								/* Front Laser		*/
0,											/* Rear Laser		*/
0,											/* Left Laser		*/
0,											/* Right Laser		*/
0, 0,
20,											/* Cargo Capacity	*/
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},		/* Current Cargo	*/
0,											/* ECM				*/
0, 											/* Fuel Scoop		*/
0,											/* Energy Bomb		*/
0,											/* Energy Unit		*/
0,											/* Docking Computer */
0,											/* Galactic H'Drive	*/
1,											/* Escape Pod		*/
0,0,0,0,
3,											/* No. of Missiles	*/
0,											/* Legal Status		*/
{0x10, 0x0F, 0x11, 0x00, 0x03, 0x1C,		/* Station Stock	*/
0x0E, 0x00, 0x00, 0x0A, 0x00, 0x11,
0x3A, 0x07, 0x09, 0x08, 0x00},
0,											/* Fluctuation		*/
0,											/* Score			*/
0x80										/* Saved			*/
};



struct ship_data *ship_list[NO_OF_SHIPS + 1] =
{
NULL,
&missile_data,
&coriolis_data,
&esccaps_data,
&alloy_data,
&cargo_data,
&boulder_data,
&asteroid_data,
&rock_data,
&orbit_data,
&transp_data,
&cobra3a_data,
&pythona_data,
&boa_data,
&anacnda_data,
&hermit_data,
&viper_data,
&sidewnd_data,
&mamba_data,
&krait_data,
&adder_data,
&gecko_data,
&cobra1_data,
&worm_data,
&cobra3b_data,
&asp2_data,
&pythonb_data,
&ferdlce_data,
&moray_data,
&thargoid_data,
&thargon_data,
&constrct_data,
&cougar_data,
&dodec_data
};


static char *condition_txt[] =
{
"Docked",
"Green",
"Yellow",
"Red"
};




struct stock_item stock_market[NO_OF_STOCK_ITEMS]=
{
{"Food",		 0, 0,  19, -2,   6, 0x01, TONNES},
{"Textiles",	 0, 0,  20, -1,  10, 0x03, TONNES},
{"Radioactives", 0, 0,  65, -3,   2, 0x07, TONNES},
{"Slaves",		 0, 0,  40, -5, 226, 0x1F, TONNES},
{"Liquor/Wines", 0, 0,  83, -5, 251, 0x0F, TONNES},
{"Luxuries",	 0, 0, 196,  8,  54, 0x03, TONNES},
{"Narcotics",	 0, 0, 235, 29,   8, 0x78, TONNES},
{"Computers",	 0, 0, 154, 14,  56, 0x03, TONNES},
{"Machinery",	 0, 0, 117,  6,  40, 0x07, TONNES},
{"Alloys",		 0, 0,  78,  1,  17, 0x1F, TONNES},
{"Firearms",	 0, 0, 124, 13,  29, 0x07, TONNES},
{"Furs",		 0, 0, 176, -9, 220, 0x3F, TONNES},
{"Minerals",	 0, 0,  32, -1,  53, 0x03, TONNES},
{"Gold",		 0, 0,  97, -1,  66, 0x07, KILOGRAMS},
{"Platinum",	 0, 0, 171, -2,  55, 0x1F, KILOGRAMS},
{"Gem-Stones",	 0, 0,  45, -1, 250, 0x0F, GRAMS},
{"Alien Items",	 0, 0,  53, 15, 192, 0x07, TONNES},
};

char *mission_planet_desc (struct galaxy_seed planet);

struct star stars[15];
char str[100];

//struct ship_solid ship_solids[10];
struct univ_object univ_oM;



struct rank rating[NO_OF_RANKS] =
{
{0, "Harmless"},
{5000, "Mostly Harmless"},
{9000, "Poor"},
{15000, "Average"},
{30000, "Above Average"},
{50000, "Competent"},
{70000, "Dangerous"},
{90000, "Deadly"},
{150000, "-- E L I T E --"}
};

char *laser_name[5] = {"Pulse", "Beam", "Military", "Mining", "Custom"};

enum equip_types
{
EQ_FUEL, EQ_MISSILE, EQ_CARGO_BAY, EQ_ECM, EQ_FUEL_SCOOPS,
EQ_ESCAPE_POD, EQ_ENERGY_BOMB, EQ_ENERGY_UNIT, EQ_DOCK_COMP,
EQ_GAL_DRIVE, EQ_PULSE_LASER, EQ_FRONT_PULSE, EQ_REAR_PULSE,
EQ_LEFT_PULSE, EQ_RIGHT_PULSE, EQ_BEAM_LASER, EQ_FRONT_BEAM,
EQ_REAR_BEAM, EQ_LEFT_BEAM, EQ_RIGHT_BEAM, EQ_MINING_LASER,
EQ_FRONT_MINING, EQ_REAR_MINING, EQ_LEFT_MINING, EQ_RIGHT_MINING,
EQ_MILITARY_LASER, EQ_FRONT_MILITARY, EQ_REAR_MILITARY,
EQ_LEFT_MILITARY, EQ_RIGHT_MILITARY
};






struct equip_item equip_stock[NO_OF_EQUIP_ITEMS] =
{
{0, 0, 1, 1,     2, " Fuel",					EQ_FUEL},
{0, 0, 1, 1,   300, " Missile",					EQ_MISSILE},
{0, 0, 1, 1,  4000, " Large Cargo Bay",			EQ_CARGO_BAY},
{0, 0, 1, 2,  6000, " E.C.M. System",			EQ_ECM},
{0, 0, 1, 5,  5250, " Fuel Scoops",				EQ_FUEL_SCOOPS},
{0, 0, 1, 6, 10000, " Escape Pod",				EQ_ESCAPE_POD},
{0, 0, 1, 7,  9000, " Energy Bomb",				EQ_ENERGY_BOMB},
{0, 0, 1, 8, 15000, " Extra Energy Unit",		EQ_ENERGY_UNIT},
{0, 0, 1, 9, 15000, " Docking Computer",		EQ_DOCK_COMP},
{0, 0, 1,10, 50000, " Galactic Hyperdrive",		EQ_GAL_DRIVE},
{0, 0, 0, 3,  4000, "+Pulse Laser",				EQ_PULSE_LASER},
{0, 0, 1, 3,     0, "-Pulse Laser",				EQ_PULSE_LASER},
{0, 0, 1, 3,  4000, ">Front",					EQ_FRONT_PULSE},
{0, 0, 1, 3,  4000, ">Rear",					EQ_REAR_PULSE},
{0, 0, 1, 3,  4000, ">Left",					EQ_LEFT_PULSE},
{0, 0, 1, 3,  4000, ">Right",					EQ_RIGHT_PULSE},
{0, 0, 1, 4, 10000, "+Beam Laser",				EQ_BEAM_LASER},
{0, 0, 0, 4,     0, "-Beam Laser",				EQ_BEAM_LASER},
{0, 0, 0, 4, 10000, ">Front",					EQ_FRONT_BEAM},
{0, 0, 0, 4, 10000, ">Rear",					EQ_REAR_BEAM},
{0, 0, 0, 4, 10000, ">Left",					EQ_LEFT_BEAM},
{0, 0, 0, 4, 10000, ">Right",					EQ_RIGHT_BEAM},
{0, 0, 1,10,  8000, "+Mining Laser",			EQ_MINING_LASER},
{0, 0, 0,10,     0, "-Mining Laser",			EQ_MINING_LASER},
{0, 0, 0,10,  8000, ">Front",					EQ_FRONT_MINING},
{0, 0, 0,10,  8000, ">Rear",					EQ_REAR_MINING},
{0, 0, 0,10,  8000, ">Left",					EQ_LEFT_MINING},
{0, 0, 0,10,  8000, ">Right",					EQ_RIGHT_MINING},
{0, 0, 1,10, 60000, "+Military Laser",			EQ_MILITARY_LASER},
{0, 0, 0,10,     0, "-Military Laser",			EQ_MILITARY_LASER},
{0, 0, 0,10, 60000, ">Front",					EQ_FRONT_MILITARY},
{0, 0, 0,10, 60000, ">Rear",					EQ_REAR_MILITARY},
{0, 0, 0,10, 60000, ">Left",					EQ_LEFT_MILITARY},
{0, 0, 0,10, 60000, ">Right",					EQ_RIGHT_MILITARY}
};

// - - - - - - - - - - -  F I L E   E N D - - - - - - - - - - - 

static char *digrams="ABOUSEITILETSTONLONUTHNOALLEXEGEZACEBISOUSESARMAINDIREA?ERATENBERALAVETIEDORQUANTEISRION";

static char *inhabitant_desc1[] = {"Large ", "Fierce ", "Small "};

static char *inhabitant_desc2[] = {"Green ", "Red ", "Yellow ", "Blue ", "Black ", "Harmless "};

static char *inhabitant_desc3[] = {"Slimy ", "Bug-Eyed ", "Horned ", "Bony ", "Fat ", "Furry "};

static char *inhabitant_desc4[] = {"Rodent", "Frog", "Lizard", "Lobster", "Bird", "Humanoid", "Feline", "Insect"};


static char planet_description[300];
static char *desc_ptr;




static char *desc_list[36][5] =
{
/*  0	*/	{"fabled", "notable", "well known", "famous", "noted"},
/*  1	*/	{"very", "mildly", "most", "reasonably", ""},
/*  2	*/	{"ancient", "<20>", "great", "vast", "pink"},
/*  3	*/	{"<29> <28> plantations", "mountains", "<27>", "<19> forests", "oceans"},
/*  4	*/	{"shyness", "silliness", "mating traditions", "loathing of <5>", "love for <5>"},
/*  5	*/	{"food blenders", "tourists", "poetry", "discos", "<13>"},
/*  6	*/	{"talking tree", "crab", "bat", "lobst", "%R"},
/*  7	*/	{"beset", "plagued", "ravaged", "cursed", "scourged"},
/*  8	*/	{"<21> civil war", "<26> <23> <24>s", "a <26> disease", "<21> earthquakes", "<21> solar activity"},
/*  9	*/	{"its <2> <3>", "the %I <23> <24>","its inhabitants' <25> <4>", "<32>", "its <12> <13>"},
/* 10	*/	{"juice", "brandy", "water", "brew", "gargle blasters"},
/* 11	*/	{"%R", "%I <24>", "%I %R", "%I <26>", "<26> %R"},
/* 12	*/	{"fabulous", "exotic", "hoopy", "unusual", "exciting"},
/* 13	*/	{"cuisine", "night life", "casinos", "sit coms", " <32> "},
/* 14	*/	{"%H", "The planet %H", "The world %H", "This planet", "This world"},
/* 15	*/	{"n unremarkable", " boring", " dull", " tedious", " revolting"},
/* 16	*/	{"planet", "world", "place", "little planet", "dump"},
/* 17	*/	{"wasp", "moth", "grub", "ant", "%R"},
/* 18	*/	{"poet", "arts graduate", "yak", "snail", "slug"},
/* 19	*/	{"tropical", "dense", "rain", "impenetrable", "exuberant"},
/* 20	*/	{"funny", "wierd", "unusual", "strange", "peculiar"},
/* 21	*/	{"frequent", "occasional", "unpredictable", "dreadful", "deadly"},
/* 22	*/	{"<1> <0> for <9>", "<1> <0> for <9> and <9>", "<7> by <8>", "<1> <0> for <9> but <7> by <8>"," a<15> <16>"},
/* 23	*/	{"<26>", "mountain", "edible", "tree", "spotted"},
/* 24	*/	{"<30>", "<31>", "<6>oid", "<18>", "<17>"},
/* 25	*/	{"ancient", "exceptional", "eccentric", "ingrained", "<20>"},
/* 26	*/	{"killer", "deadly", "evil", "lethal", "vicious"},
/* 27	*/	{"parking meters", "dust clouds", "ice bergs", "rock formations", "volcanoes"},
/* 28	*/	{"plant", "tulip", "banana", "corn", "%Rweed"},
/* 29	*/	{"%R", "%I %R", "%I <26>", "inhabitant", "%I %R"},
/* 30	*/	{"shrew", "beast", "bison", "snake", "wolf"},
/* 31	*/	{"leopard", "cat", "monkey", "goat", "fish"},
/* 32	*/	{"<11> <10>", "%I <30> <33>","its <12> <31> <33>", "<34> <35>", "<11> <10>"},
/* 33	*/	{"meat", "cutlet", "steak", "burgers", "soup"},
/* 34	*/	{"ice", "mud", "Zero-G", "vacuum", "%I ultra"},
/* 35	*/	{"hockey", "cricket", "karate", "polo", "tennis"}
};


static int min_dist[NO_OF_SHIPS+1] = {0, 200, 800, 200,   200, 200, 300, 384,   200,
200, 200, 420, 900, 500, 800, 384, 384,
384, 384, 384, 200, 384, 384, 384,   0,
384,   0, 384, 384, 700, 384,   0,   0,
900};





int initial_flags[NO_OF_SHIPS + 1] =
{
0,								// NULL,
0,								// missile 
FLG_GOOD,						// coriolis
FLG_FLY_TO_PLANET,				// escape

FLG_INACTIVE,					// alloy
FLG_INACTIVE,					// cargo
FLG_INACTIVE,					// boulder
FLG_INACTIVE,					// asteroid
FLG_INACTIVE,					// rock
FLG_FLY_TO_PLANET,				// shuttle
FLG_FLY_TO_PLANET,				// transporter
0,								// cobra3
0,								// python
0,								// boa
0,								// anaconda
0,								// hermit
0,								// viper ex. FLG_POLICE 
0,								// sidewinder
0,								// mamba
0,								// krait
0,								// adder
0,								// gecko
0,								// cobra1
0,								// worm
0,								// cobra3
0,								// asp2
0,								// python
0,								// fer_de_lance
0,								// moray
FLG_ANGRY,						// thargoid
FLG_ANGRY,						// thargon
FLG_ANGRY,						// constrictor
FLG_ANGRY | FLG_CLOAKED,		// cougar
FLG_GOOD								// dodec
};



#include "./includes/shipface.h"
#include "./includes/gfx.h"


#include "./includes/stars.h"
#include "./includes/pilot.h"
#include "./includes/swat.h"
#include "./includes/planet.h"

#include "./includes/trade.h"
#include "./includes/space.h"

#include "./includes/intro.h"
#include "./includes/docked.h"
#include "./includes/missions.h"


#include "./includes/game.h"

int nova_speed = 1500;
float ax, ay, az;
static long tick=0;

int isRunning=1;






//***------------------------------------------
//***------------------------------------------
//***------------------------------------------
//***------------------------------------------

void draw()
{

int i;

int x1= GFX_X_OFFSET;
int y1= GFX_Y_OFFSET;
int x2= x1 + 511;
int y2= y1 + 384;

glClearColor(0.0, 0.0, 0.0, 0.0);

glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


switch (current_screen)
{

case SCR_BREAK_PATTERN: 

// Draw a break pattern (for launching, docking and hyperspacing).
// Just draw a very simple one for the moment.

glEnable(GL_SCISSOR_TEST);


for(i=0;i< tick;i++)
gfx_draw_circle (256, 192, 50 + i * (45-i*2), GFX_COL_WHITE);


tick++;
if(tick>20) 
{

if (docked)
check_mission_brief();
else
current_screen = SCR_FRONT_VIEW;

tick= 0;
}

glDisable(GL_SCISSOR_TEST);

update_console();
break;


case SCR_GAME_OVER: 


    update_universe();

    glColor3f(1,0.3,0);
    if( razaar_refugees >=100 && (docked_planet.d == 112) && 
	(docked_planet.b == 95) && (cmdr.galaxy_number == 0) )
	gfx_display_colour_text (8,358, "Let's try it for real now. Charged 30.0 Cr.", GFX_COL_BLUE_4);
    else
	gfx_display_colour_text (8,358, "Escape Pod launched. Charged 30.0 Cr.", GFX_COL_BLUE_4);

    zpino= 0;
    cougar_pod= 0;
    
    
    tick++;
    if(tick>100) 
    {
	tick=0; 

	cmdr.credits-= 300;
	if(cmdr.credits < 30)
	{
	    cmdr.credits= 0;
	    current_screen=	SCR_MISSION_4;
	    return;
	}
	else
	{

		cmdr.galaxy= LAST_GALAXY;
		docked_planet= LAST_DOCKED;

		hyperspace_planet= docked_planet;
		destination_planet= docked_planet;
		complete_hyperspace();

		cmdr.galaxy_number= last_glx;

		current_screen = SCR_ESCAPE_POD;
		
		    finish = FALSE;
		    game_over = FALSE;


	    if(razaar_refugees ==101)
	    {

		int i;
		for (i = 0; i < 17; i++)
		    cmdr.current_cargo[i]= 0;

		cmdr.cargo_capacity= 8;

		cmdr.missiles= 1;

		cmdr.left_laser = 0;
		cmdr.right_laser = 0;
		cmdr.rear_laser = 0;
		cmdr.front_laser = PULSE_LASER;

		cmdr.ecm= 0;
		cmdr.energy_bomb= 0;
		cmdr.docking_computer = 0;
		cmdr.galactic_hyperdrive= 0;
		cmdr.energy_unit= 0;
		cmdr.fuel= 9;
		cmdr.fuel_scoop= 1;
		cmdr.legal_status= 0;

		docked_planet= RAZAAR;
		razaar_refugees= 100;
		hyperspace_planet= docked_planet;
		destination_planet= docked_planet;
		complete_hyperspace();
		current_screen= SCR_ESCAPE_NOVA;

	    }
	    else
	    {
		if(razaar_refugees ==100)
		    cmdr.mission= -4;

    	    }

	}

    }

break;
    

case SCR_INTRO_ONE: 
update_intro1();

tick++;
if(kbd_space_pressed) 
{
tick=0; 
current_screen= SCR_INTRO_TWO; 
initialise_intro2();

kbd_space_pressed=FALSE;
}
break;


case SCR_INTRO_TWO: 

update_intro2();

tick++;
if(kbd_space_pressed || kbd_fire_pressed) 
{
tick=0; 
docked=0;
initialise_game();
kbd_space_pressed= FALSE;
}
break;

case SCR_MISSION_4:

thargoid_mission_debrief();

if(kbd_space_pressed || kbd_fire_pressed) 
{
tick=0; 
initialise_game();
initialise_intro1();
kbd_space_pressed= FALSE;
}

break;



case SCR_ESCAPE_POD:


    tick++;
    auto_dock();

    warp_stars= 1;
    update_universe();
    


    gfx_display_colour_text (8,358, "Escape Pod Auto-Navigation activated.", GFX_COL_BLUE_4);

    for(i = 0; i < MAX_UNIV_OBJECTS-1; i++)
    {
	if (universe[i].type != 0)
	universe[i].location.z -= 2500;
    }


    if((ship_count[SHIP_CORIOLIS] != 0) || (ship_count[SHIP_DODEC] != 0))
    {
	abandon_ship();
	tick=0;
    }


    
break;



case SCR_ESCAPE_NOVA:

tick++;

warp_stars= 1;
for(i = 0; i < MAX_UNIV_OBJECTS; i++)
{
if (universe[i].type != 0)
universe[i].location.z += nova_speed;
}



if(tick<150)
{

//small explosions

nova_speed+= 300;
if(nova_speed>2000) nova_speed= 2000;		


if(tick>100)
flight_roll= 10- (tick-105);
else
flight_roll= 0;

if(flight_roll < 0) flight_roll= 0;



if(tick> 99)
{

if(tick<120) 
nova_speed= 30000;
else
nova_speed= 1000;

float col= (float)1-((float)(tick-95))/55; 
glColor3f(col, col, col);

glBegin(GL_QUADS);
glVertex2f(x1, y1);
glVertex2f(x2, y1);
glVertex2f(x2, y2);
glVertex2f(x1, y2);
glEnd();

}else
gfx_display_colour_text (8,358, "Solar waves detected, time to SURF Commander.", GFX_COL_BLUE_4);


if(tick> 90 && tick< 94 && tick%2==0)
{


ax= universe[0].location.x +100+rand255()*100;
ay= universe[0].location.y -700+rand255()*100; 
az= universe[0].location.z +1000+rand255()*10 ;
i = add_new_ship (SHIP_CARGO, ax, ay, az, universe[0].rotmat, 0, 0);				
universe[i].bravery= 2; //radioactives
explode_object(i);
}			
else	
if(tick< 103 && tick> 95 )
{




if(tick%3==0)
{
ax= universe[0].location.x +100+rand255()*100;
ay= universe[0].location.y -700+rand255()*100; 
az= universe[0].location.z +1000+rand255()*10 ;
i = add_new_ship (SHIP_CARGO, ax, ay, az, universe[0].rotmat, 0, 0);				
explode_object(i);
}

if(tick%2==0)
{
ax= universe[1].location.x +100+rand255()*100;
ay= universe[1].location.y -700+rand255()*100; 
az= universe[1].location.z +1000+rand255()*10 ;
i = add_new_ship (SHIP_ALLOY, ax, ay, az, universe[1].rotmat, 0, 0);	

explode_object(i);

}

if(tick ==101)
{
razaar_refugees= 101;
universe[1].type= SHIP_SUN;

}

}	



}
else 
if(tick<330)
{

/*	
if(tick>200)
flight_roll= 30- (tick-200);
else
flight_roll= 30;


if(flight_roll < 0) flight_roll= 0;
*/

//big fast white explosion, coming closer

nova_speed-= 1000;
if(nova_speed<10) nova_speed= 10;

gfx_display_colour_text (8,358, "Reverse Warp Jump boost activated, slowing down.", GFX_COL_BLUE_4);

if(tick>220) warp_stars=0;


}
else
{
energy= 1;
flight_speed = 0;	

front_shield= 0;
aft_shield= 0;

cmdr.fuel= 1;
current_screen= SCR_FRONT_VIEW;
tick=0;
}

update_universe();
//		update_starfield();

break;



default:
main_loop();

}

glColor3f(1, 1, 1);

glBegin(GL_LINE_LOOP);
glVertex2f(x1, y1);
glVertex2f(x2, y1);
glVertex2f(x2, y2);
glVertex2f(x1, y2);
glEnd();
}



void setupViewport(int w, int h)
{

glViewport(0, 0, w, h);

glMatrixMode(GL_PROJECTION);
glLoadIdentity();
//glOrtho (0, w, h, 0, 0, 1);

    glFrustum( -1.0, 1.0, -1, 1, 0, 1 );
    

glMatrixMode(GL_MODELVIEW);
glLoadIdentity ();


glCullFace(GL_BACK);
glShadeModel(GL_FLAT);
glDisable(GL_LIGHTING);

glDisable(GL_DITHER);

//glDisable(GL_SCISSOR_TEST);
//glEnable(GL_DEPTH_TEST);

//glScissor(GFX_X_OFFSET, 128+GFX_Y_OFFSET, 511, 384);



initFONT();

finished_once= FALSE;

initialise_game();
initialise_intro1();

glClearColor(0.0, 0.0, 0.0, 0.0);
glColor3f(1, 1, 1);

}











void keyPress(int key)
{



switch (key)
{
case SDLK_ESCAPE:             // ESCAPE key
       isRunning=0;
break;

//*** cheats
//case SDLK_-: cheat= TRUE; dock_player();
//break;
//case SDLK_=: cheat= TRUE; detonate_bomb = 1;
//break;

case SDLK_p: 
    
    //enter_witchspace();
    //auto_pilot=1;

break;

/*
case SDLK_`: cheat= TRUE; 
	cmdr.fuel=350;
	energy= 250;
	front_shield= 220;
	aft_shield= 240;
break;
*/
//*** end cheats


break; //this is toggle
case SDLK_z: kbd_scanner_zoom_pressed= TRUE; 
break;
case SDLK_f: kbd_find_pressed = TRUE; 


break;

case SDLK_o: kbd_origin_pressed= TRUE; 
break;

case SDLK_h: kbd_hyperspace_pressed = TRUE; 
break;
case SDLK_g: kbd_gal_hyperspace_pressed = TRUE; 
break;
case SDLK_t: 
case SDLK_k: kbd_target_missile_pressed = TRUE; 
break;
case SDLK_u: kbd_unarm_missile_pressed = TRUE; 
break;
case SDLK_m: kbd_fire_missile_pressed = TRUE; 
break;
case SDLK_d: kbd_dock_pressed = TRUE; 
break;
case  SDLK_TAB: kbd_energy_bomb_pressed = TRUE; 
break;
case SDLK_e: kbd_ecm_pressed = TRUE; 
break;
case SDLK_q: kbd_escape_pressed = TRUE; 
break;

case  SDLK_RETURN: kbd_enter_pressed = TRUE; 
break;


case SDLK_a: kbd_fire_pressed = TRUE; 
break;
//case SDLK_;: kbd_inc_speed_pressed = TRUE; 
//break;
//case SDLK_/: kbd_dec_speed_pressed = TRUE; 
//break;

case SDLK_UP:
case SDLK_s: kbd_up_pressed = TRUE; 
break;
case SDLK_DOWN:
case SDLK_x: kbd_down_pressed = TRUE; 
break;
case SDLK_LEFT:
//case ',': kbd_left_pressed = TRUE; 
break;
case SDLK_RIGHT:
//case '.': kbd_right_pressed = TRUE; 
break;
case SDLK_SPACE: 
kbd_space_pressed = TRUE; 
kbd_inc_speed_pressed = TRUE;
break;
case SDLK_j: kbd_jump_pressed = TRUE; 
break;




case SDLK_1: kbd_F1_pressed = TRUE; 
break;
case SDLK_2: kbd_F2_pressed = TRUE; 
break;
case SDLK_3: kbd_F3_pressed = TRUE; 
break;
case SDLK_4: kbd_F4_pressed = TRUE; 
break;
case SDLK_5: kbd_F5_pressed = TRUE; 
break;
case SDLK_6: kbd_F6_pressed = TRUE; 
break;
case SDLK_7: kbd_F7_pressed = TRUE; 
break;
case SDLK_8: kbd_F8_pressed = TRUE; 
break;
case SDLK_9: kbd_F9_pressed = TRUE; 
break;
case SDLK_0: kbd_F10_pressed = TRUE; 
break;

}


}





void keyRelease(int key)
{



switch (key)
{

case 16777216:             // ESCAPE key
exit (0);
break;


case 'A': kbd_fire_pressed = FALSE; 
break;
case ';': kbd_inc_speed_pressed = FALSE; 
break;
case '/': kbd_dec_speed_pressed = FALSE; 
break;
case 'S': kbd_up_pressed = FALSE; 
break;
case 'X': kbd_down_pressed = FALSE; 
break;
case ',': kbd_left_pressed = FALSE; 
break;
case '.': kbd_right_pressed = FALSE; 
break;

case 'T': 
case 'K': kbd_target_missile_pressed = FALSE; 
break;

case ' ': kbd_space_pressed = FALSE; 
kbd_inc_speed_pressed = FALSE;
break;
case 'J': kbd_jump_pressed = FALSE; 
break;
case 'E': kbd_ecm_pressed = FALSE; 
break;

case 'O': kbd_origin_pressed= FALSE; 
break;


case '1': kbd_F1_pressed = FALSE; 
break;
case '2': kbd_F2_pressed = FALSE; 
break;
case '3': kbd_F3_pressed = FALSE; 
break;
case '4': kbd_F4_pressed = FALSE; 
break;
case '5': kbd_F5_pressed = FALSE; 
break;
case '6': kbd_F6_pressed = FALSE; 
break;
case '7': kbd_F7_pressed = FALSE; 
break;
case '8': kbd_F8_pressed = FALSE; 
break;
case '9': kbd_F9_pressed = FALSE; 
break;
case '0': kbd_F10_pressed = FALSE; 
break;

}

}





int main(int argc, char **argv) {
    // initialize SDL video:

    if(SDL_Init(SDL_INIT_VIDEO)<0) {
        fprintf(stderr,"ERROR: cannot initialize SDL video.\n");
        return 1;
    }
    SDL_Surface* screen = NULL;
    if((screen=SDL_SetVideoMode( scrW, scrH, 16, SDL_SWSURFACE)) == 0 ) {
        fprintf(stderr,"ERROR: Video mode set failed.\n");
        return 1;
    }
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_SetCaption(argv[0],0);

    // initialize TinyGL:
    unsigned int pitch;
    int	mode;
    switch( screen->format->BitsPerPixel ) {
    case  8:
        fprintf(stderr,"ERROR: Palettes are currently not supported.\n");
        return 1;
    case 16:
        pitch = screen->pitch;
        mode = ZB_MODE_5R6G5B;
        break;
    case 24:
        pitch = ( screen->pitch * 2 ) / 3;
        mode = ZB_MODE_RGB24;
        break;
    case 32:
        pitch = screen->pitch / 2;
        mode = ZB_MODE_RGBA;
        break;
    default:
        return 1;
        break;
    }
    ZBuffer *frameBuffer = ZB_open( scrW, scrH, mode, 0, 0, 0, 0);
    glInit( frameBuffer );

    // initialize GL:
    setupViewport(scrW, scrH);



    // variables for timing:
    unsigned int frames=0;
    unsigned int tNow=SDL_GetTicks();
    unsigned int tLastFps=tNow;

    // main loop:
    while(isRunning) {
        ++frames;
        tNow=SDL_GetTicks();

        // do event handling:
        SDL_Event evt;
        while( SDL_PollEvent( &evt ) ) switch(evt.type) {
	        case SDL_KEYDOWN:
			keyPress(evt.key.keysym.sym);

        	break;
	        case SDL_QUIT:
	            isRunning=0;
        	break;
        }


        // draw scene:
        draw();

        // swap buffers:
        if ( SDL_MUSTLOCK(screen) && (SDL_LockSurface(screen)<0) ) {
            fprintf(stderr, "SDL ERROR: Can't lock screen: %s\n", SDL_GetError());
            return 1;
        }
        ZB_copyFrameBuffer(frameBuffer, screen->pixels, pitch);
        if ( SDL_MUSTLOCK(screen) ) SDL_UnlockSurface(screen);
        SDL_Flip(screen);




        // check for error conditions:
        char* sdl_error = SDL_GetError( );
        if( sdl_error[0] != '\0' ) {
            fprintf(stderr,"SDL ERROR: \"%s\"\n",sdl_error);
            SDL_ClearError();
        }
        // update fps:
        if(tNow>=tLastFps+5000) {
            printf("%i frames in %f secs, %f frames per second.\n",frames,(float)(tNow-tLastFps)*0.001f,(float)frames*1000.0f/(float)(tNow-tLastFps));
            tLastFps=tNow;
            frames=0;
        }
    }



//*** EXIT
    printf("%i frames in %f secs, %f frames per second.\n",frames,(float)(tNow-tLastFps)*0.001f,(float)frames*1000.0f/(float)(tNow-tLastFps));
    // cleanup:
    ZB_close(frameBuffer);
    if(SDL_WasInit(SDL_INIT_VIDEO))
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
    return 0;
}
