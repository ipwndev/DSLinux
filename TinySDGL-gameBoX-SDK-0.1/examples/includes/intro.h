
static int ship_no;
static int show_time;
static int direction;




void initialise_intro1 (void)
{
	current_screen= SCR_INTRO_ONE; 


	clear_universe();
	set_init_matrix (intro_ship_matrix);
	add_new_ship (SHIP_SUN, -95000, 70000, 400000, intro_ship_matrix, 0, 0);

	if(!finished_once)
		add_new_ship (SHIP_COBRA3, 0, 0, 4500, intro_ship_matrix, 1, -1);
	else
		add_new_ship (SHIP_THARGOID, 0, 0, 4500, intro_ship_matrix, 1, -1);

}


void initialise_intro2 (void)
{
	ship_no = 0;
	show_time = 0;
	direction = 100;

	clear_universe();
	create_new_stars();
	set_init_matrix (intro_ship_matrix);
	add_new_ship (1, 0, 0, 5000, intro_ship_matrix, -1, -1);
}



void update_intro1 (void)
{

	update_starfield();
	flight_speed= 10;

	universe[3].location.z -= 100;

	if(!finished_once)
	{
		if(universe[3].location.z < 450)
			universe[3].location.z = 450;
	}
	else
	{
		if(universe[3].location.z < 750)
			universe[3].location.z = 750;
	}

	if(!finished_once && mcount==150) universe[3].rotx*= -1;

	update_universe();

	glColor3f(1,1,1);
	txtOut (325, 62, "-ELITE-", 5);
	txtOut (280, 110, "Press space to Start.", 3);

	txtOut (151, 385, "Original Game (C) I.Bell & D.Braben, 1984.", 2);
	txtOut (151, 400, "Re-engineered to C-language by C.J.Pinder, 2001.", 2);



	glColor3f(0,1,1);
	txtOut (250, 300, "Mission ELITE v0.98.01", 3);

	glColor3f(1,1,0);
	txtOut (205, 315, "Modified Elite-TNK by Z.Aksentijevic, 2007.", 2);



	glColor3f(1,0.5,0);
	txtOut (190, 330, "Linux/Qt-4.4.0 port by me... again, March-2008.", 2);

	glColor3f(1,0.3,0);
	txtOut (230, 345, "SDL/TinySDGL port... again, June-2008.", 2);

	glColor3f(0,1,0);
	txtOut (265, 357, "www.polarelephant.blogspot.com", 2);
	glColor3f(0.7,0,1);
	txtOut (215, 90, "The Battle For Birera", 4);


}


void update_intro2 (void)
{

	update_starfield();
	flight_speed= 5;

	show_time++;

	if ((show_time >= 35) && (direction < 0))
		direction = -direction;

	universe[3].location.z += direction*3;

	if (universe[3].location.z < min_dist[ship_no])
		universe[3].location.z = min_dist[ship_no];

	if (universe[3].location.z > 4500)
	{
		do
		{
			ship_no++;
			if (ship_no > NO_OF_SHIPS-1)
				ship_no = 1;
		} while (min_dist[ship_no] == 0);

		show_time = 0;
		direction = -100;

		ship_count[universe[3].type] = 0;
		universe[3].type = 0;		

		if(ship_no==2) ship_no=3;
		add_new_ship (ship_no, 0, 0, 4500, intro_ship_matrix, -1, -1);
	}


	update_starfield();
	update_universe();

	gfx_display_centre_text (360, "Press Fire or Space, Commander.", 140, GFX_COL_GOLD);
	gfx_display_centre_text (330, ship_list[ship_no]->name, 120, GFX_COL_WHITE);
}


