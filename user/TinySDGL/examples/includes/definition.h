//RES_800_600

int kbd_F1_pressed;
int kbd_F2_pressed;
int kbd_F3_pressed;
int kbd_F4_pressed;
int kbd_F5_pressed;
int kbd_F6_pressed;
int kbd_F7_pressed;
int kbd_F8_pressed;
int kbd_F9_pressed;
int kbd_F10_pressed;

int kbd_fire_pressed;
int kbd_enter_pressed;
int kbd_ecm_pressed;
int kbd_scanner_zoom_pressed;
int kbd_energy_bomb_pressed;
int kbd_hyperspace_pressed;
int kbd_gal_hyperspace_pressed;
int kbd_ctrl_pressed;
int kbd_jump_pressed;
int kbd_escape_pressed;
int kbd_dock_pressed;
int kbd_find_pressed;
int kbd_origin_pressed;
int kbd_fire_missile_pressed;
int kbd_target_missile_pressed;
int kbd_unarm_missile_pressed;
int kbd_inc_speed_pressed;
int kbd_dec_speed_pressed;
int kbd_up_pressed;
int kbd_down_pressed;
int kbd_left_pressed;
int kbd_right_pressed;

int kbd_backspace_pressed;
int kbd_space_pressed;

#define NO_OF_RANKS	9

#define EQUIP_START_Y	202
#define EQUIP_START_X	50
#define EQUIP_MAX_Y		290
#define EQUIP_WIDTH		200
#define Y_INC			16
#define NO_OF_EQUIP_ITEMS	34


#define GFX_SCALE		(2)
#define GFX_X_OFFSET	(144)
#define GFX_Y_OFFSET	(44)
#define GFX_X_CENTRE	(256)
#define GFX_Y_CENTRE	(192)

#define GFX_VIEW_TX		1
#define GFX_VIEW_TY		1
#define GFX_VIEW_BX		509
#define GFX_VIEW_BY		381



#define GFX_COL_BLACK		0
#define GFX_COL_DARK_RED	28
#define GFX_COL_WHITE		255
#define GFX_COL_GOLD		39
#define GFX_COL_RED			49
#define GFX_COL_CYAN		11

#define GFX_COL_GREY_0		247
#define GFX_COL_GREY_1		248
#define GFX_COL_GREY_2		235
#define GFX_COL_GREY_3		234
#define GFX_COL_GREY_4		237
#define GFX_COL_GREY_5		238
#define GFX_COL_GREY_6		239

#define GFX_COL_BLUE_0		44
#define GFX_COL_BLUE_1		45
#define GFX_COL_BLUE_2		46
#define GFX_COL_BLUE_3		133
#define GFX_COL_BLUE_4		4
#define GFX_COL_BLUE_5		5
#define GFX_COL_BLUE_6		6

#define GFX_COL_RED_1		49
#define GFX_COL_RED_2		1
#define GFX_COL_RED_3		28
#define GFX_COL_RED_4		71

#define GFX_COL_WHITE_2		242

#define GFX_COL_YELLOW_1	37
#define GFX_COL_YELLOW_2	39
#define GFX_COL_YELLOW_3	89
#define GFX_COL_YELLOW_4	160
#define GFX_COL_YELLOW_5	251

#define GFX_COL_ORANGE_1	76
#define GFX_COL_ORANGE_2	77
#define GFX_COL_ORANGE_3	122
#define GFX_COL_ORANGE_4	123
#define GFX_COL_ORANGE_5	124

#define GFX_COL_GREEN_1		2
#define GFX_COL_GREEN_2		17
#define GFX_COL_GREEN_3		86
#define GFX_COL_GREEN_4		87
#define GFX_COL_GREEN_0		88

#define GFX_COL_PINK_1		183
#define GFX_COL_PINK_2		184
#define GFX_COL_PINK_3		185


#define SCR_INTRO_ONE		-1
#define SCR_INTRO_TWO		-2
#define SCR_EQUIP_SHIP		1
#define SCR_GALACTIC_CHART	2
#define SCR_SHORT_RANGE		3
#define	SCR_PLANET_DATA		4
#define SCR_MARKET_PRICES	5
#define SCR_CMDR_STATUS		6
#define SCR_INVENTORY		7
#define SCR_FRONT_VIEW		8
#define SCR_REAR_VIEW		9
#define SCR_LEFT_VIEW		10
#define SCR_RIGHT_VIEW		11
#define SCR_BREAK_PATTERN	12
#define SCR_OPTIONS			15
#define SCR_LOAD_CMDR		16
#define SCR_SAVE_CMDR		17
#define SCR_QUIT			18
#define SCR_GAME_OVER		19
#define SCR_SETTINGS		20
#define SCR_ESCAPE_POD		21
#define SCR_ESCAPE_NOVA		22

#define SCR_MISSION_A		107
#define SCR_MISSION_B		110
#define SCR_MISSION_B2		111
#define SCR_MISSION_C		120
#define SCR_MISSION_C2		121
#define SCR_MISSION_D		130
#define SCR_MISSION_D2		131

#define SCR_MISSION_GENESIS		140
#define SCR_LAST_MISSION	160

#define SCR_MISSION_1		101
#define SCR_MISSION_2		102
#define SCR_MISSION_3		103
#define SCR_MISSION_4		104
#define SCR_MISSION_5		105


#define PULSE_LASER		0x0F
#define BEAM_LASER		0x8F
#define MILITARY_LASER	0x97
#define MINING_LASER	0x32


#define FLG_DEAD			(1)
#define	FLG_REMOVE			(2)
#define FLG_EXPLOSION		(4)
#define FLG_ANGRY			(8)
#define FLG_FIRING			(16)
#define FLG_HAS_ECM			(32)
#define FLG_HOSTILE			(64)
#define FLG_CLOAKED			(128)
#define FLG_FLY_TO_PLANET	(256)
#define FLG_FLY_TO_STATION	(512)
#define FLG_INACTIVE		(1024)

#define FLG_GOOD			(2048) 
#define FLG_BAD				(4096) 
#define FLG_DOCKED			(8192) 


#define SHIP_SUN				-2
#define SHIP_PLANET				-1
#define SHIP_MISSILE			1
#define SHIP_CORIOLIS			2

#define SHIP_ESCAPE_CAPSULE		3
#define SHIP_ALLOY				4
#define SHIP_CARGO				5
#define SHIP_BOULDER			6
#define SHIP_ASTEROID			7
#define SHIP_ROCK				8

#define SHIP_SHUTTLE			9
#define SHIP_TRANSPORTER		10

#define SHIP_COBRA3				11

#define SHIP_PYTHON				12
#define SHIP_BOA				13
#define SHIP_ANACONDA			14

#define SHIP_HERMIT				15
#define SHIP_VIPER				16

#define SHIP_SIDEWINDER			17
#define SHIP_MAMBA				18
#define SHIP_KRAIT				19
#define SHIP_ADDER				20
#define SHIP_GECKO				21
#define SHIP_COBRA1				22
#define SHIP_WORM				23

#define SHIP_COBRA3_LONE		24
#define SHIP_ASP2				25
#define SHIP_PYTHON_LONE		26
#define SHIP_FER_DE_LANCE		27
#define SHIP_MORAY				28
#define SHIP_THARGOID			29

#define SHIP_THARGLET			30
#define SHIP_CONSTRICTOR		31
#define SHIP_COUGAR				32
#define SHIP_DODEC				33


#define MAX_UNIV_OBJECTS	128
#define NO_OF_SHIPS		33
#define NO_OF_STOCK_ITEMS	17
#define ALIEN_ITEMS_IDX		16


#define MISSILE_UNARMED	-2
#define MISSILE_ARMED	-1

#define SLAVES		3
#define NARCOTICS	6
#define FIREARMS	10

#define FALSE 0
#define TRUE 1


#define TONNES		0
#define	KILOGRAMS	1
#define GRAMS		2


struct star
{
	float x;
	float y;
	float z;
};



struct point
{
	int x;
	int y;
	int z;
};

#include "vector.h"


struct univ_object
{
	int type;
	Vector location;
	Matrix rotmat;
	int rotx;
	int rotz;
	int flags;
	int energy;
	int velocity;
	int acceleration;
	int missiles;
	int target;
	int bravery;
	int exp_delta;
	int exp_seed;
	int distance;
};


struct random_seed
{
	int a;
	int b;
	int c;
	int d;
};



struct galaxy_seed
{
	unsigned char a;	/* 6c */
	unsigned char b;	/* 6d */
	unsigned char c;	/* 6e */
	unsigned char d;	/* 6f */
	unsigned char e;	/* 70 */
	unsigned char f;	/* 71 */
};


struct planet_data
{
	int government;
	int economy;
	int techlevel;
	int population;
	int productivity;
	int radius;
};

 
struct stock_item
{
	char name[16];
	int current_quantity;
	int current_price;
	int base_price;
	int eco_adjust;
	int base_quantity;
	int mask;
	int units;
};



struct ship_point
{
	int x;
	int y;
	int z;
	int dist;
	int face1;
	int face2;
	int face3;
	int face4;
};


struct ship_line
{
	int dist;
	int face1;
	int face2;
	int start_point;
	int end_point;
};


struct ship_face_normal
{
	int dist;
	int x;
	int y;
	int z;
};



struct ship_data
{
	char name[32];
	int num_points;
	int num_lines;
	int num_faces;
	int max_loot;
	int scoop_type;
	float size;
	int front_laser;
	int bounty;
	int vanish_point;
	int energy;
	int velocity;
	int missiles;
	int laser_strength;
	struct ship_point *points;
	struct ship_line *lines;
	struct ship_face_normal *normals;
};
#include "shipdata.h"





struct rank
{
	int score;
	char *title;
};

struct equip_item
{
	int canbuy;
	int y;
	int show;
	int level;
	int price;
	char *name;
	int type;
};


struct commander
{
	char name[32];
	int mission;
	int ship_x;
	int ship_y;
	struct galaxy_seed galaxy;
	int credits;
	int fuel;
	int unused1;
	int	galaxy_number;
	int front_laser;
	int rear_laser;
	int left_laser;
	int right_laser;
	int unused2;
	int unused3;
	int cargo_capacity;
	int current_cargo[NO_OF_STOCK_ITEMS];
	int ecm;
	int fuel_scoop;
	int energy_bomb;
	int energy_unit;
	int docking_computer;
	int galactic_hyperdrive;
	int escape_pod;
	int unused4;
	int unused5;
	int unused6;
	int unused7;
	int missiles;
	int legal_status;
	int station_stock[NO_OF_STOCK_ITEMS];
	int market_rnd;
	int score;
	int saved;
};

struct player_ship
{
	int max_speed;
	int max_roll;
	int max_climb;
	int max_fuel;
	int altitude;
	int cabtemp;
};





struct ship_face
{
	int colour;
	int norm_x;
	int norm_y;
	int norm_z;
	int points;	
	int p1;
	int p2;
	int p3;
	int p4;
	int p5;
	int p6;
	int p7;
	int p8;
};


struct ship_solid
{
	int num_faces;
	struct ship_face *face_data;
};












void move_univ_object (struct univ_object *obj);
void restore_saved_commander (void);
void launch_enemy(int un, int type, int flags, int bravery);


// - - - - - - - - - - -  F I L E   E N D - - - - - - - - - - - 


int curr_galaxy_num = 1;
int curr_fuel = 70;
int carry_flag = 0;
int current_screen = 0;
int witchspace;

int wireframe = 0;
int anti_alias_gfx = 0;
int hoopy_casinos = 0;
int speed_cap = 75;
int instant_dock = 0;


int scanner_cx= 253;
int scanner_cy= 461; 
int scanner_w= 100; 
int scanner_h= 40; 
int scanner_zoom= 1; 

int compass_centre_x= 382;
int compass_centre_y= 417; 

int planet_render_style = 0;

int game_over;
int docked;
int finish;
int flight_speed;
float flight_roll;
float flight_climb;
int front_shield;
int aft_shield;
int energy;
int laser_temp;
int detonate_bomb;
int auto_pilot;


int old_cross_x, old_cross_y;
int cross_timer;
int condition;

int draw_lasers;
int mcount;
int message_count;
int rolling;
int climbing;
int game_paused;
int have_joystick;

int find_input;
char find_name[50];
char message_string[100];
char hyper_name[50];





int hyper_ready;
int hyper_countdown;
int hyper_distance;
int hyper_galactic;

int asteroid_wave;

int tharg_wave;
int cross_color;

int laser_counter;
int laser, laser2;
int laser_x;
int laser_y;

int ecm_active;
int missile_target;
int ecm_ours;
int in_battle;

int warp_stars;

int zpino;
int razaar_refugees;
int razaar_g_x, razaar_g_y;
int univ_obj_currently_being_updated;
int nova_timer;
int cougar_pod;
int cargo_item_count;
int current_jettison;

int real_cargo_capacity;
int closest_distance;
int closest_missile_distance;

int n_good, n_bad;
int im_good;
int last_good, last_bad;
int is_melee;
int n_angry_at_me;
int n_attacking_station;
int st_attacker;

int station_is_exploding;
int st_exp_timer;

int gal_fam;
int cheat;
int police_is_angry;
int traders_are_angry;
int convoy_present;
int finished_once;
int global_bounty;


static unsigned char remb_db;

static struct random_seed rnd_seed;
struct galaxy_seed docked_planet, 
	   hyperspace_planet, 
	   destination_planet;

struct planet_data current_planet_data;
struct commander cmdr;
struct player_ship myship;




struct galaxy_seed RAZAAR, LAST_DOCKED, LAST_GALAXY;

int last_glx;

int draw_laser;
Matrix station_mat;




int cross_x = 0;
int cross_y = 0;




static int hilite_item;



void update_screen (void);
void switch_to_view (struct univ_object *flip);


// - - - - - - - - - - -  F I L E   E N D - - - - - - - - - - - 


void info_message (char *message, int cnt)
{

if(message_count>20) return;

	strcpy (message_string, message);
	
	if(cnt==0)
		message_count = 40;
	else 
		message_count = cnt;

}


/*
 * Game Over...
 */

void do_game_over (void)
{
//	snd_play_sample (SND_GAMEOVER);
	game_over = 1;
}

void decrease_energy (int amount)
{
	energy += amount;

	if(energy <= 0)
	{
		do_game_over();
		energy= 0;
	}
}


/*
 * Deplete the shields.  Drain the energy banks if the shields fail.
 */

void damage_ship (int damage, int front)
{

	int shield;

	if (damage <= 0)	/* sanity check */
		return;
	
	shield = front ? front_shield : aft_shield;
	
	shield -= damage;
	if (shield < 0)
	{
		decrease_energy (shield);
		shield = 0;
	}
	
	if (front)
		front_shield = shield;
	else
		aft_shield = shield;
}

 
/*
 * Regenerate the shields and the energy banks.
 */

void regenerate_shields (void)
{
	if (energy > 127)
	{
		if (front_shield < 255)
		{
			front_shield+= 1 +cmdr.energy_unit;
			energy--;
		}
	
		if (aft_shield < 255)
		{
			aft_shield+= 1 +cmdr.energy_unit;
			energy--;
		}
	}
		

	energy+= 1 +cmdr.energy_unit;
	if (energy > 255)
		energy = 255;
}


int carrying_contraband (void)
{
	return (cmdr.current_cargo[SLAVES] + cmdr.current_cargo[NARCOTICS])*2 +
			cmdr.current_cargo[FIREARMS];
}




