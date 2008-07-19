

void restore_saved_commander (void)
{
	cmdr = saved_cmdr;

	docked_planet = find_planet (cmdr.ship_x, cmdr.ship_y);
	hyperspace_planet = docked_planet;

	generate_planet_data (&current_planet_data, docked_planet);
	generate_stock_market ();
	set_stock_quantities (cmdr.station_stock);
}


void finish_game (void)
{
	finish = TRUE;
	game_over = TRUE;
}






/*
 * Move the planet chart cross hairs to specified position.
 */


void move_cross (int dx, int dy)
{
	cross_timer = 3;

	if (current_screen == SCR_SHORT_RANGE)
	{
		cross_x += (dx * 4);
		cross_y += (dy * 4);

		if (cross_x < 1)
			cross_x = 1;
			
		if (cross_x > 510)
			cross_x = 510;

		if (cross_y < 37)
			cross_y = 37;
		
		if (cross_y > 383)
			cross_y = 383;
	}
	else
	if (current_screen == SCR_GALACTIC_CHART)
	{
		cross_x += (dx * 2);
		cross_y += (dy * 2);

		if (cross_x < 1)
			cross_x = 1;
			
		if (cross_x > 510)
			cross_x = 510;

		if (cross_y < 37)
			cross_y = 37;
		
		if (cross_y > 293)
			cross_y = 293;
	}
}




void draw_laser_sights(void)
{
	int laser = 0;
	int x1,y1,x2,y2;
	
	x1= 210;
	y1= 1;

	switch (current_screen)
	{
		case SCR_FRONT_VIEW:
			gfx_display_text (x1, y1, "Front View");
			laser = cmdr.front_laser;
			break;
		
		case SCR_REAR_VIEW:
			gfx_display_text (x1, y1, "Rear View");
			laser = cmdr.rear_laser;
			break;

		case SCR_LEFT_VIEW:
			gfx_display_text (x1, y1, "Left View");
			laser = cmdr.left_laser;
			break;

		case SCR_RIGHT_VIEW:
			gfx_display_text (x1, y1, "Right View");
			laser = cmdr.right_laser;
			break;
	}
	

	if(laser)
	{
		x1 = 128 * GFX_SCALE;
		y1 = (96-8) * GFX_SCALE;
		y2 = (96-16) * GFX_SCALE;
   
		gfx_draw_colour_line (x1-1, y1, x1-1, y2, GFX_COL_GREY_6); 
		gfx_draw_colour_line (x1, y1, x1, y2, cross_color);
		gfx_draw_colour_line (x1+1, y1, x1+1, y2, GFX_COL_GREY_6); 

		y1 = (96+8) * GFX_SCALE;
		y2 = (96+16) * GFX_SCALE;
		
		gfx_draw_colour_line (x1-1, y1, x1-1, y2, GFX_COL_GREY_6); 
		gfx_draw_colour_line (x1, y1, x1, y2, cross_color);
		gfx_draw_colour_line (x1+1, y1, x1+1, y2, GFX_COL_GREY_6); 

		x1 = (128-8) * GFX_SCALE;
		y1 = 96 * GFX_SCALE;
		x2 = (128-16) * GFX_SCALE;
		   
		gfx_draw_colour_line (x1, y1-1, x2, y1-1, GFX_COL_GREY_6); 
		gfx_draw_colour_line (x1, y1, x2, y1, cross_color);
		gfx_draw_colour_line (x1, y1+1, x2, y1+1, GFX_COL_GREY_6); 

		x1 = (128+8) * GFX_SCALE;
		x2 = (128+16) * GFX_SCALE;

		gfx_draw_colour_line (x1, y1-1, x2, y1-1, GFX_COL_GREY_6); 
		gfx_draw_colour_line (x1, y1, x2, y1, cross_color);
		gfx_draw_colour_line (x1, y1+1, x2, y1+1, GFX_COL_GREY_6); 
	}
}



void arrow_right (void)
{
	switch (current_screen)
	{		

		case SCR_INVENTORY:
			kbd_right_pressed = FALSE;

			if(!docked && cmdr.current_cargo[current_jettison] >0 ) // jettison_cargo();
			{

			
				Matrix rotmat;
				set_init_matrix (rotmat);
				rotmat[2].z = 1.0;
				rotmat[0].x = -1.0;
			
				int newship= add_new_ship(SHIP_CARGO, -64+rand255()/2, -100-rand255()/5, +180+rand255()/2, 
											rotmat, 0, (rand255() &3)-1);

				if(newship <0 || newship >=MAX_UNIV_OBJECTS)
					info_message ("CARGO DOOR JAMMED", 40);
				else
				{
				
								
					if(gal_fam >0 && current_jettison ==0) 
					{
						gal_fam--;
						universe[newship].bravery= -101;
					}
					else
						universe[newship].bravery= current_jettison;

					cmdr.current_cargo[current_jettison]--;
					universe[newship].velocity = flight_speed+ 3+ rand255()/100;


				}
			}

		break;



		case SCR_MARKET_PRICES:
			buy_stock();
			kbd_right_pressed = FALSE;
		break;
		
		case SCR_EQUIP_SHIP:
			buy_equip();
			kbd_right_pressed = FALSE;
		break;

		case SCR_SHORT_RANGE:
		case SCR_GALACTIC_CHART:
			move_cross(2, 0);
		break;

		case SCR_FRONT_VIEW:
		case SCR_REAR_VIEW:
		case SCR_RIGHT_VIEW:
		case SCR_LEFT_VIEW:
			//if (flight_roll > 0) flight_roll = 0;
			//else
			{
				decrease_flight_roll(2);
				rolling = 1;
			}
		break;
	}
}


void arrow_left (void)
{
	switch (current_screen)
	{
		case SCR_MARKET_PRICES:
			sell_stock();
			kbd_left_pressed = FALSE;
		break;

	
		case SCR_SHORT_RANGE:
		case SCR_GALACTIC_CHART:
			move_cross (-2, 0);
		break;

		case SCR_FRONT_VIEW:
		case SCR_REAR_VIEW:
		case SCR_RIGHT_VIEW:
		case SCR_LEFT_VIEW:
			//if (flight_roll < 0) flight_roll = 0;
			//else
			{
				increase_flight_roll(2);
				rolling = 1;
			}
		break;
	}
}


void arrow_up (void)
{
	switch (current_screen)
	{
		case SCR_INVENTORY:
			kbd_up_pressed = FALSE;
			if(!docked && hilite_item > 1) hilite_item--;
		break;

		case SCR_MARKET_PRICES:
			select_previous_stock();
			kbd_up_pressed = FALSE;
			break;

		case SCR_EQUIP_SHIP:
			select_previous_equip();
			kbd_up_pressed = FALSE;
			break;

		
		case SCR_SHORT_RANGE:
		case SCR_GALACTIC_CHART:
			move_cross (0, -2);
			break;

		case SCR_FRONT_VIEW:
		case SCR_REAR_VIEW:
		case SCR_RIGHT_VIEW:
		case SCR_LEFT_VIEW:
			//if (flight_climb > 0) flight_climb = 0;
			//else
			{
				decrease_flight_climb(1);
			}
			climbing = 1;
			break;
	}
}



void arrow_down (void)
{
	switch (current_screen)
	{
		case SCR_INVENTORY:
			kbd_down_pressed = FALSE;
			if(!docked && hilite_item<cargo_item_count) hilite_item++;
		break;

		case SCR_MARKET_PRICES:
			select_next_stock();
			kbd_down_pressed = FALSE;
		break;

		case SCR_EQUIP_SHIP:
			select_next_equip();
			kbd_down_pressed = FALSE;
		break;
		
		case SCR_SHORT_RANGE:
		case SCR_GALACTIC_CHART:
			move_cross (0, 2);
		break;

		case SCR_FRONT_VIEW:
		case SCR_REAR_VIEW:
		case SCR_RIGHT_VIEW:
		case SCR_LEFT_VIEW:
			//if (flight_climb < 0) flight_climb = 0;
			//else
			{
				increase_flight_climb(1);
			}
			climbing = 1;
		break;

	}
}









void auto_dock (void)
{
//	struct univ_object ship;

	univ_oM.location.x = 0;
	univ_oM.location.y = 0;
	univ_oM.location.z = 0;
	
	set_init_matrix (univ_oM.rotmat);
	univ_oM.rotmat[2].z = 1;
	univ_oM.rotmat[0].x = -1;
	univ_oM.velocity = flight_speed;
	univ_oM.acceleration = 0;
	univ_oM.bravery = 0;
	univ_oM.rotz = 0;
	univ_oM.rotx = 0;

	auto_pilot_ship (&univ_oM);
	if (univ_oM.velocity > 22)
		flight_speed = 22;
	else
		flight_speed = univ_oM.velocity;
	
	if(univ_oM.acceleration > 0)
	{
		flight_speed++;
		if (flight_speed > 22)
			flight_speed = 22;
	}

	if(univ_oM.acceleration < 0)
	{
		flight_speed--;
		if (flight_speed < 1)
			flight_speed = 1;
	}	



//*** --------
	if(station_is_exploding)
		flight_speed= 3;



	if (univ_oM.rotx == 0)
		flight_climb = 0;
	
	if (univ_oM.rotx < 0)
	{
		increase_flight_climb(1);

		if (univ_oM.rotx < -1)
			increase_flight_climb(1);
	}
	
	if (univ_oM.rotx > 0)
	{
		decrease_flight_climb(1);

		if (univ_oM.rotx > 1)
			decrease_flight_climb(1);
	}
	
	if (univ_oM.rotz == 127)
		flight_roll = -14;
	else
	{
		if (univ_oM.rotz == 0)
			flight_roll = 0;

		if (univ_oM.rotz > 0)
		{
			increase_flight_roll(1);

			if (univ_oM.rotz > 1)
				increase_flight_roll(1);
		}
		
		if (univ_oM.rotz < 0)
		{
			decrease_flight_roll(1);

			if (univ_oM.rotz < -1)
				decrease_flight_roll(1);
		}
	}

}





void handle_flight_keys (void)
{
	rolling = 0;
	climbing = 0;

	if(kbd_F1_pressed)
	{
		
		if (docked)
			launch_player();
		else
		{
			if (current_screen != SCR_FRONT_VIEW)
			{
				current_screen = SCR_FRONT_VIEW;
				flip_stars();
			}
		}
	}

	if(kbd_F2_pressed)
	{
		
		if (!docked)
		{
			if (current_screen != SCR_REAR_VIEW)
			{
				current_screen = SCR_REAR_VIEW;
				flip_stars();
			}
		}
	}

	if(kbd_F3_pressed)
	{
		
		if (!docked)
		{
			if (current_screen != SCR_LEFT_VIEW)
			{
				current_screen = SCR_LEFT_VIEW;
				flip_stars();
			}
		}
		else
		{
			if(cmdr.mission == -6)
			{
				current_screen = SCR_MISSION_D2;
			}
			else
			if(cmdr.mission == -106)
			{
				current_screen = SCR_MISSION_D;
			}
			else
			if(cmdr.mission == -5)
			{
				current_screen = SCR_MISSION_C2;
			}
			else
			if(cmdr.mission == -4)
			{
				current_screen = SCR_MISSION_C;
			}
			else
			if(cmdr.mission == -3)
			{
				current_screen = SCR_MISSION_B2;
			}
			else
			if(cmdr.mission == -2)
			{
				current_screen = SCR_MISSION_B;
			}
			else
			if(cmdr.mission == -1)
			{
				current_screen = SCR_MISSION_A;
			}
			else
			if(cmdr.mission == 1)
			{
				current_screen = SCR_MISSION_1;

				clear_universe();
				set_init_matrix (intro_ship_matrix);
				add_new_ship (SHIP_CONSTRICTOR, 160, 65, 550, intro_ship_matrix, -1, -1);

			}
			else
			if(cmdr.mission == 3)
				current_screen = SCR_MISSION_2; //104 on completition
			else
			if(cmdr.mission == 4)
				current_screen = SCR_MISSION_GENESIS;
			else
			if(cmdr.mission == 5)
			{
				current_screen = SCR_MISSION_3;
				clear_universe();
				set_init_matrix (intro_ship_matrix);
				add_new_ship (SHIP_THARGOID, 410, 200, 1300, intro_ship_matrix, 1, -1);

			}
			else
			if(cmdr.mission == 6)
				current_screen = SCR_LAST_MISSION;



		}

	}


	if(kbd_F4_pressed)
	{
		if (docked)
		{
			hilite_item = 0;
			collapse_equip_list();

			equip_ship();
		}
		else
		{
			if (current_screen != SCR_RIGHT_VIEW)
			{
				current_screen = SCR_RIGHT_VIEW;
				flip_stars();
			}
		}
	}

	
	if(kbd_F5_pressed)
	{
		old_cross_x = -1;
		cross_x = hyperspace_planet.d * GFX_SCALE;
		cross_y = (hyperspace_planet.b / (2 / GFX_SCALE)) + (18 * GFX_SCALE) + 1;
		display_galactic_chart();

	}

	if(kbd_F6_pressed)
	{
		old_cross_x = -1;
		cross_x = ((hyperspace_planet.d - docked_planet.d) * 4 * GFX_SCALE) + GFX_X_CENTRE;
		cross_y = ((hyperspace_planet.b - docked_planet.b) * 2 * GFX_SCALE) + GFX_Y_CENTRE;
		display_short_range_chart();
	}

	if(kbd_F7_pressed)
	{

		display_data_on_planet();
	}


	if(kbd_F8_pressed )
	{
		hilite_item= 0;
		display_market_prices();
	}	

	if(kbd_F9_pressed)
		display_commander_status();

	
	if(kbd_F10_pressed)
	{
		hilite_item = 1;
		display_inventory();
	}




	if(kbd_enter_pressed)
	{
	
		kbd_enter_pressed = FALSE; 



		switch(current_screen)
		{


//			case SCR_INVENTORY:
			case SCR_PLANET_DATA:
				old_cross_x = -1;
				cross_x = hyperspace_planet.d * GFX_SCALE;
				cross_y = (hyperspace_planet.b / (2 / GFX_SCALE)) + (18 * GFX_SCALE) + 1;
				display_galactic_chart();

			break;

			case SCR_GALACTIC_CHART:
				old_cross_x = -1;
				cross_x = ((hyperspace_planet.d - docked_planet.d) * 4 * GFX_SCALE) + GFX_X_CENTRE;
				cross_y = ((hyperspace_planet.b - docked_planet.b) * 2 * GFX_SCALE) + GFX_Y_CENTRE;
				display_short_range_chart();
			break;

			case SCR_SHORT_RANGE:
				display_data_on_planet();
			break;

/*

			case SCR_PLANET_DATA:
				hilite_item= 0;
				display_market_prices();
			break;


			case SCR_MARKET_PRICES:
				display_commander_status();

			break;

			case SCR_CMDR_STATUS:
				display_inventory();
			break;

*/


			default:
				display_data_on_planet();
			//	display_inventory();

		}


	}


	if(kbd_dock_pressed)
	{
		kbd_dock_pressed = FALSE; 

		if(auto_pilot)
			disengage_auto_pilot();
		else
			if(!docked && cmdr.docking_computer)
				engage_auto_pilot();
	}

	if(auto_pilot)
		return;



//*** -----------------
	if(kbd_ecm_pressed)
	{
		kbd_ecm_pressed = FALSE; 
		if (!docked && cmdr.ecm)
			activate_ecm(1);
	}


	if(kbd_scanner_zoom_pressed)
	{
		kbd_scanner_zoom_pressed= FALSE;
		scanner_zoom = (scanner_zoom==2) ? 1:2;
	}

	if(kbd_find_pressed)
	{
		kbd_find_pressed = FALSE; 

		if ((current_screen == SCR_GALACTIC_CHART) || (current_screen == SCR_SHORT_RANGE))
			find_planet_by_name(find_name);
	}


	if(kbd_origin_pressed)
	{
		kbd_origin_pressed= FALSE;
		move_cursor_to_origin();
	}



	if(kbd_hyperspace_pressed)
	{
		kbd_hyperspace_pressed = FALSE;
		if(!docked) 
			start_hyperspace();
	}

	if(kbd_gal_hyperspace_pressed)
	{
		kbd_gal_hyperspace_pressed = FALSE;
		if(!docked) 
			start_galactic_hyperspace();
	}

	
	if(kbd_fire_missile_pressed)
	{
		kbd_fire_missile_pressed = FALSE; 

		if (!docked)
			fire_missile();
	}

	
	if(kbd_target_missile_pressed)
	{
		kbd_target_missile_pressed = FALSE; 
		if (!docked)
		{
		
			if(missile_target == MISSILE_UNARMED)
				arm_missile();		
			else
				unarm_missile();
		}
	}

	if(kbd_unarm_missile_pressed)
	{
		kbd_unarm_missile_pressed = FALSE; 
		if (!docked) unarm_missile();
	}
	

	if(kbd_energy_bomb_pressed)
	{
 		kbd_energy_bomb_pressed = FALSE;
		if ((!docked) && (cmdr.energy_bomb))
		{
			detonate_bomb = 1;
			cmdr.energy_bomb= 0; 
		}
	}		

	if(kbd_escape_pressed)
	{
		kbd_escape_pressed = FALSE;
		if ((!docked) && (cmdr.escape_pod) && (!witchspace) )
		{
			cmdr.escape_pod= 1;
		    game_over= TRUE;
			current_screen= SCR_GAME_OVER;
		}
	}








	if(kbd_jump_pressed)
	{
		if(!docked && !witchspace)
			jump_warp();
	}
		if (kbd_inc_speed_pressed)
	{
		if (!docked)
		{
			if(flight_speed < myship.max_speed)
				flight_speed++;

/*
			else
			{
				flight_speed= 100;
			}

*/

		}
	}

	if(kbd_dec_speed_pressed)
	{

		if(flight_speed >myship.max_speed)
			flight_speed= myship.max_speed;

		if (!docked)
		{
			if (flight_speed > 1)
				flight_speed--;
		}
	}

	if(kbd_fire_pressed)
	{
		if ((!docked) && (draw_lasers == 0))
			draw_lasers = fire_laser();
		
	}

	if(kbd_up_pressed)
		arrow_up();
	
	if(kbd_down_pressed)
		arrow_down();

	if(kbd_left_pressed)
		arrow_left();
		
	if(kbd_right_pressed)
		arrow_right();
	
}








/*
 * Draw the game over sequence. 
 */

void run_game_over_screen()
{


	int i;
	int newship;
	Matrix rotmat;
	int type;

	current_screen = SCR_GAME_OVER;
	gfx_set_clip_region (1, 1, 510, 383);
	
	flight_speed = 6;
	flight_roll = 0;
	flight_climb = 0;

	set_init_matrix (rotmat);

	universe[99].type =0;
	newship = add_new_ship (SHIP_COBRA3, 0, 0, -150, rotmat, 127, -12);
	
	universe[newship].flags |= FLG_DEAD;
	universe[newship].velocity= 15;

	for (i = 0; i < 30; i++)
	{
		universe[100-i].type =0; //clear if universe full

		type= (rand255() >50) ? SHIP_CARGO : SHIP_ALLOY;
		if(!(i%3))
			launch_enemy(newship, type, FLG_INACTIVE,0);
		else
			launch_enemy(newship, type, FLG_DEAD | FLG_INACTIVE,0);

	}
}








/*
 * Initialise the game parameters.
 */

void initialise_game(void)
{
	set_rand_seed (15);

	current_screen = SCR_INTRO_ONE;

	restore_saved_commander();

	sprintf(find_name,"");

	flight_speed = 1;
	flight_roll = 0;
	flight_climb = 0;
	front_shield = 255;
	aft_shield = 255;
	energy = 255;
	draw_lasers = 0;


	hyper_ready = 0;
	detonate_bomb = 0;
	find_input = 0;
	witchspace = 0;
	game_paused = 0;
	auto_pilot = 0;
	
	create_new_stars();
	clear_universe();
	
	cross_x = -1;
	cross_y = -1;
	cross_timer = 0;

	myship.max_speed = 40;		/* 40 = 0.27 Light Mach */
	myship.max_roll = 31;
	myship.max_climb = 8;		/* CF 8 */
	myship.max_fuel = 70;		/* 7.0 Light Years */


	razaar_refugees= 0;
	gal_fam= 0;

	global_bounty= 0;

	finish = FALSE;
	auto_pilot = FALSE;
	game_over = FALSE;	
	cheat= FALSE;
	dock_player ();

	asteroid_wave= -1;
	tharg_wave= -1;

LAST_DOCKED= docked_planet;
RAZAAR= docked_planet;
LAST_GALAXY= cmdr.galaxy;
last_glx= cmdr.galaxy_number;

RAZAAR= docked_planet;



if(!finished_once)
	cmdr.mission= -1001;
else
{
	razaar_refugees= 500;
	cmdr.front_laser= MILITARY_LASER;

	cmdr.mission = 6;
	cmdr.energy_unit = 2; 
	cmdr.energy_bomb = 1;
	current_screen = SCR_LAST_MISSION;

	cmdr.fuel_scoop= 1;
	cmdr.ecm= 1;
	cmdr.docking_computer= 1;
	cmdr.mission= 6;
}

//cmdr.mission= -102;

//cmdr.galactic_hyperdrive= 5;
//cmdr.fuel_scoop= 1;
//cmdr.ecm= 1;

//sprintf(find_name,"ORORQU");
//cmdr.mission= -3;

//sprintf(find_name,"RAZAAR");
//cmdr.mission= -4;

/*
cmdr.credits= 100000;
cmdr.score= 59000;
cmdr.fuel_scoop= 1;
cmdr.cargo_capacity= 35;
cmdr.galactic_hyperdrive= 5;
cmdr.front_laser= MILITARY_LASER;
cmdr.rear_laser= MINING_LASER;
cmdr.ecm= 1;
razaar_refugees= 500;
*/

//sprintf(find_name,"QUBE");
//cmdr.mission= -106;


//cmdr.mission= -107;

//cmdr.mission= 5;

 
}





void main_loop(void)
{
	
	handle_flight_keys ();


	if (!docked)
	{
		if (!rolling)
		{
			if (flight_roll>0)
				decrease_flight_roll((float)myship.max_roll/80);

			if (flight_roll<0)
				increase_flight_roll((float)myship.max_roll/80);
		}

		if (!climbing)
		{
			if (flight_climb>0)
				decrease_flight_climb((float)myship.max_climb/40);

			if (flight_climb<0)
				increase_flight_climb((float)myship.max_climb/40);
		}




		update_universe ();


		if(cmdr.mission == -1002 && (mcount%5))
		{
			glColor3f(0.7,0.5,0);
			txtOut(270, 75, "Wake up... wake up Commander.", 2);
			
			if(mcount==1 && !is_melee) 
				is_melee= TRUE;

			if(!n_good || !n_bad)
			{
				cmdr.mission= -101;
				dock_player();
			}
		}



		if(cmdr.mission ==6 && !station_is_exploding)
		{
			sprintf(str, "Station (%d)", universe[2].energy);
			gfx_display_colour_text (380, 360, str, GFX_COL_ORANGE_1);

		}

		if(!(mcount%45) && cmdr.mission == 6 && 
			tharg_wave <3 && n_good <(ship_count[SHIP_THARGOID]+5)) 
		{
			create_trader();		
			launch_enemy (2, SHIP_VIPER, FLG_GOOD, 0);
		}

		if(cmdr.mission ==6 && tharg_wave >=0 && ship_count[SHIP_THARGOID] <=0 && !(mcount%150)) 
		{

			int i;
			switch(tharg_wave) 
			{

				case 0:
					tharg_wave= 1;
					for (i = 0; i < 45; i++)
						create_thargoid();	

					message_count= 0;
					info_message ("There are more coming this way Commander.", 100);
				break;


				case 1:
					tharg_wave= 2;
					for (i = 0; i < 55; i++)
						create_thargoid();	

					message_count= 0;
					info_message ("Watch out, its not over yet", 100);
				break;


				case 2:
					tharg_wave= 3;
					for (i = 0; i < 95; i++)
						create_thargoid();	

					message_count= 0;
					info_message ("Commander... the sky is full of them." , 100);
				break;


				case 3:

					cmdr.mission= 107;
					message_count= 0;
					info_message ("Yeepeee-yeah, Pangalactic Gargle for everyone.", 300);
				break;					
								
	
			}


		}


		if(cmdr.mission == -1 && !(mcount%30) && asteroid_wave ==2 && n_good <7 ) 
		{
			int	type = SHIP_SIDEWINDER + (rand255() &7);
			if(type== SHIP_WORM) 
				type= SHIP_VIPER;

			launch_enemy (2, type, FLG_GOOD | FLG_FLY_TO_STATION, 115);
			info_message ("REINFORCEMENT ON THE WAY", 70);
		}
		else
		if(cmdr.mission == -6 && !(mcount%150) && n_good <12 ) 
		{
			int	type = SHIP_SIDEWINDER + (rand255() &7);
			if(type== SHIP_WORM) 
				type= SHIP_VIPER;

			launch_enemy (2, type, FLG_GOOD | FLG_FLY_TO_STATION, 115);
			info_message ("REINFORCEMENT ON THE WAY", 70);
		}


		if((cmdr.mission== -1 || cmdr.mission== -6 ) && !(mcount%150) &&
			asteroid_wave>=0 && ship_count[SHIP_ASTEROID] <=0) 
		{

			int i, cnt= 0;
			float ax, ay, az;
			if(asteroid_wave <2 && myship.altitude <230)
			{
				info_message ("Asteroid wave cleared, return to safe altitude.", 40);
			}	
			else
			switch(asteroid_wave) 
			{

				case 0:

					asteroid_wave= 1;

					if(cmdr.mission ==-1)
						cnt= 15;
					else
						cnt= 50;

					for(i=0; i <cnt;i++)
					{
						ax= 5000+rand255()*20;
						ay= 5000+rand255()*15; 
						az= 5000+rand255()*25;

						if(universe[1].location.z >0)
							az= -az;

						int new_ship = add_new_ship (SHIP_ASTEROID, ax, ay, az, universe[2].rotmat, 
														0, (rand255() &3)-2);

						if(new_ship> 2)
						{
							universe[new_ship].velocity = 23;
							universe[new_ship].flags |= FLG_BAD;
						}
							
					}
					message_count= 0;
					info_message ("There are more coming this way Commander.", 100);

				break;

				case 1:
	
					asteroid_wave= 2;
			
					if(cmdr.mission ==-1)
						cnt= 50;
					else
						cnt= 75;

					for(i=0; i <cnt;i++)
					{
						ax= 5000+rand255()*20;
						ay= 5000+rand255()*15; 
						az= 5000+rand255()*25;

						if(universe[1].location.z >0)
							az= -az;

						int new_ship = add_new_ship (SHIP_ASTEROID, ax, ay, az, universe[2].rotmat, 
														0, (rand255() &3)-2);

						if(new_ship> 2)
						{
							universe[new_ship].velocity = 25;
							universe[new_ship].flags |= FLG_BAD;
						}
					}


					message_count= 0;
					info_message ("Watch out, its not over yet", 100);
	
				break;

				
				case 2:

					if(cmdr.mission == -6)
					{
						cmdr.mission= -107;
						cmdr.credits+= 5000;
						message_count= 0;
						info_message ("Well done boys, you kicked those Asteroid butts good.", 300);
						
					}
					else
					{
						cmdr.mission= -102;
						message_count= 0;
						info_message ("Great job guys, the Milky Bars are on me.", 300);
					}
					
				break;

			}

		}


		



		if((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
			(current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW))
		{
			if (draw_lasers)
			{
				draw_laser_lines();
				draw_lasers--;
			}
			
			draw_laser_sights();
		}

		if(energy <50 && !(mcount%50))
			info_message ("ENERGY LOW", 40);

		if(auto_pilot)
		{
			auto_dock();
			
			if(station_is_exploding){

				message_count= 0;
				gfx_display_colour_text (8,358, "Space Station destroyed. Let's try again.", GFX_COL_BLUE_4);
			}
			else
			if(!(mcount %40))
				info_message ("DOCKING COMPUTER ON", 30);
		}

		if(hyper_ready)
		{
			message_count= 0;

			if(!(mcount%3))
				countdown_hyperspace();

			display_hyper_status();
		}

		if(!(mcount%5)) 
			regenerate_shields();

		if(razaar_refugees>100 && (docked_planet.d == 112) && (docked_planet.b == 95) && (cmdr.galaxy_number == 0))
		{
			if(!(mcount%10))
			{

				if(universe[0].distance < universe[1].distance)
					update_cabin_temp(0); //sun
				else
					update_cabin_temp(1); //second sun, ex-planet			
			}
				
		}
		else
		{

			if(!(mcount%5))
				update_altitude(1);
			
			if(!(mcount%10))
				update_cabin_temp(0);

		}



		if(mcount == 0 && !witchspace)
			random_encounter();


		if ( cmdr.fuel> 1 && cmdr.mission == -4 && ((mcount %7) == 0) && 
			(docked_planet.d == 112) && (docked_planet.b == 95))
		{
			cmdr.fuel--;
			info_message ("FUEL LEAK DETECTED", 4);
		}


		if(cmdr.mission== -5 && razaar_refugees== 100 && ((mcount %7) == 0))
		{
	
			nova_timer--;
			sprintf(str, "ETN T- %.2f", (float)nova_timer/100);
			info_message (str, 4);
			
			if(nova_timer<0)
			{
				game_over= TRUE;
				current_screen= SCR_GAME_OVER;
			}

		
			if(universe[1].distance> 500000 && universe[0].distance> 700000 && kbd_jump_pressed)
			{
			
				jump_warp();				
				if(warp_stars==1)
				{

					docked_planet= RAZAAR;
					hyperspace_planet= docked_planet;
					destination_planet= docked_planet;
					complete_hyperspace();
					current_screen= SCR_ESCAPE_NOVA;
				}

			}
		}



		if(global_bounty >0 && !witchspace)
		{
			cmdr.credits += global_bounty;
			sprintf (str, "+ %d.%d Cr (Total %.1f Cr)", global_bounty/10, global_bounty%10, (float)cmdr.credits/10);
			info_message (str, 19);

			global_bounty= 0;
		}

		cool_laser();				
		time_ecm();
	}
	else //docked
	{


		//empty -hrhrhryt

	}


	if(cmdr.score>999999) 
		cmdr.score =999999;

	if( message_count > 0 && (current_screen == SCR_FRONT_VIEW || current_screen ==SCR_REAR_VIEW ||
		current_screen ==SCR_LEFT_VIEW || current_screen ==SCR_RIGHT_VIEW) )
	{
		message_count--;
		gfx_display_colour_text (8,358, message_string, GFX_COL_BLUE_4);
	}

	if (cross_timer > 0)
	{
		cross_timer--;
		if (cross_timer == 0)
		{
    		show_distance_to_planet();
		}
	}


	update_console();



//*** -----------------------------------------------
	if (current_screen == SCR_EQUIP_SHIP)
		equip_ship();

	if (current_screen == SCR_GALACTIC_CHART)
		display_galactic_chart();

	if (current_screen == SCR_SHORT_RANGE)
		display_short_range_chart();

	if (current_screen == SCR_PLANET_DATA)
		display_data_on_planet();

	if (current_screen == SCR_MARKET_PRICES)
		display_market_prices();

	if (current_screen == SCR_CMDR_STATUS)
		display_commander_status();

	if (current_screen == SCR_INVENTORY)
		display_inventory();


	if (current_screen == SCR_MISSION_A)
		asteroid_mission_brief();

	if (current_screen == SCR_MISSION_B)
		cloak_mission_brief();

	if (current_screen == SCR_MISSION_B2)
		cloak_mission_brief2();


	if (current_screen == SCR_MISSION_C)
		nova_mission_brief();
	if (current_screen == SCR_MISSION_C2)
		nova_mission_brief2();

	if (current_screen == SCR_MISSION_D)
		astr2_mission_brief();

	if (current_screen == SCR_MISSION_D2)
		astr2_mission_brief2();




	if (current_screen == SCR_MISSION_GENESIS)
		gen_mission_debrief();

	if (current_screen == SCR_LAST_MISSION)
		last_mission();


	if (current_screen == SCR_MISSION_1)
		constrictor_mission_brief();

	if (current_screen == SCR_MISSION_2)
		constrictor_mission_debrief();	

	if (current_screen == SCR_MISSION_3)
		thargoid_mission_second_brief();

	if (current_screen == SCR_MISSION_4)
		thargoid_mission_debrief();




	if(docked || cmdr.mission <-1000)
	{
		glColor3f(0.5, 0.5, 0.5);


		txtOut(10,5+15*2, "SMICOLON acc", 2);
		txtOut(10,5+15*3, "/ de-acceler", 2);
		txtOut(10,5+15*4, "J warp jump", 2);
		txtOut(10,5+15*5, "<, > roll", 2);
		txtOut(10,5+15*6, "S, X - climb", 2);
		txtOut(10,5+15*7, "A fire", 2);
		txtOut(10,5+15*9, "K arm/un-arm", 2);
		txtOut(10,5+15*10, "M fire missl", 2);
		txtOut(10,5+15*12, "H hyper jump", 2);
		txtOut(10,5+15*13, "G gal. jump", 2);
		txtOut(10,5+15*14, "E e.c.m.", 2);
		txtOut(10,5+15*15, "TAB en. bomb", 2);
		txtOut(10,5+15*16, "D dock. comp", 2);
		txtOut(10,5+15*17, "Z scan sr/lr", 2);
		txtOut(10,5+15*18, "Q e.pod 30Cr", 2);

		txtOut(10,5+15*20, "1 lnch/frnt", 2);
		txtOut(10,5+15*21, "2 rear", 2);
		txtOut(10,5+15*22, "3 MSG/left", 2);
		txtOut(10,5+15*23, "4 equip/rght", 2);
		txtOut(10,5+15*24, "5 gal. chart", 2);
		txtOut(10,5+15*25, "6 hyp. chart", 2);
		txtOut(10,5+15*26, "7 planet inf", 2);
		txtOut(10,5+15*27, "8 market", 2);
		txtOut(10,5+15*28, "9 cmdr. stat", 2);
		txtOut(10,5+15*29, "0 inventory", 2);
		txtOut(10,5+15*31, "ENTER toggle", 2);
		txtOut(10,5+15*32, "F find /chrt", 2);
		txtOut(10,5+15*33, "O origin /ch", 2);

		glColor3f(0.5, 0, 0);
		txtOut(10,5+15*35, "CHEATS", 2);
		txtOut(10,5+15*36, "min, equal", 2);

	}


/*
	glColor3f(0,0,1); 	
	sprintf (str, "pAg%d, tAg%d, cBd%d, inBtl%d, atkMe%d, atkS%d, gvt%d, mct%d, stEx%d, stEt%d, lstGlx%d, cTmp%d", 
		police_is_angry, traders_are_angry,  carrying_contraband(), in_battle, n_angry_at_me, n_attacking_station, current_planet_data.government, mcount, station_is_exploding, st_exp_timer, last_glx, myship.cabtemp); 	
	txtOut (1, 1, str,2);
*/

	if(!finish && game_over)
		run_game_over_screen();


}


