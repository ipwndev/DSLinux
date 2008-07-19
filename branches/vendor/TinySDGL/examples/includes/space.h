
// - - - - - - - - - - -  F I L E   E N D - - - - - - - - - - - 




int calc_distance_to_planet (struct galaxy_seed from_planet, struct galaxy_seed to_planet)
{
	int dx,dy;
	int light_years;

	dx = abs(to_planet.d - from_planet.d);
	dy = abs(to_planet.b - from_planet.b);

	dx = dx * dx;
	dy = dy / 2;
	dy = dy * dy;

	light_years = sqrt(dx + dy);
	light_years *= 4;

	return light_years;
}



void rotate_x_first (float *a, float *b, int direction)
{
	float fx,ux;

	fx = *a;
	ux = *b;

	if(direction <=0)   
	{	
		*a = fx - (fx / 500) + (ux / 20);
		*b = ux - (ux / 500) - (fx / 20);
	}
	else
	{
		*a = fx - (fx / 500) - (ux / 20);
		*b = ux - (ux / 500) + (fx / 20);
	}
}


void rotate_vec (struct vector *vec, float alpha, float beta)
{
	float x,y,z;
	
	x = vec->x;
	y = vec->y;
	z = vec->z;

	y = y - alpha * x;
	x = x + alpha * y;
	y = y - beta * z;
	z = z + beta * y;
	
	vec->x = x;
	vec->y = y;
	vec->z = z;
}


/*
 * Update an objects location in the universe.
 */

void move_univ_object (struct univ_object *obj)
{
	float x,y,z;
	float k2;
	float alpha;
	float beta;
	float speed;

	int rotx, rotz;

	alpha = flight_roll / 250;
	beta = flight_climb / 250;
	
	x = obj->location.x;
	y = obj->location.y;
	z = obj->location.z;

	if(!(obj->flags & FLG_DEAD))
	{ 
		if(obj->velocity != 0)
		{
			speed = obj->velocity;
			x += obj->rotmat[2].x * speed; 
			y += obj->rotmat[2].y * speed; 
			z += obj->rotmat[2].z * speed; 
		}

		if(obj->acceleration != 0)
		{
			obj->velocity += obj->acceleration;
			obj->acceleration = 0;
			if(obj->velocity > ship_list[obj->type]->velocity)
				obj->velocity = ship_list[obj->type]->velocity;
			
			if(obj->velocity <1)
				obj->velocity = 1;
		}
	}

	k2 = y - alpha * x;
	z = z + beta * k2;
	y = k2 - z * beta;
	x = x + alpha * y;

	z = z - flight_speed; 

	obj->location.x = x;
	obj->location.y = y;
	obj->location.z = z;	

	obj->distance = sqrt(x*x + y*y + z*z);
	

	rotx= obj->rotx;
	rotz= obj->rotz;

	if(rotz != 0)
	{	
		rotate_x_first (&obj->rotmat[0].x, &obj->rotmat[1].x, rotz);
		rotate_x_first (&obj->rotmat[0].y, &obj->rotmat[1].y, rotz);	
		rotate_x_first (&obj->rotmat[0].z, &obj->rotmat[1].z, rotz);	
	}

	rotate_vec (&obj->rotmat[2], alpha, beta);
	rotate_vec (&obj->rotmat[1], alpha, beta);
	rotate_vec (&obj->rotmat[0], alpha, beta);

	//*** Orthonormalize the rotation matrix... 
	tidy_matrix (obj->rotmat);

	if(rotx != 0)
	{
		rotate_x_first (&obj->rotmat[2].x, &obj->rotmat[1].x, rotx);
		rotate_x_first (&obj->rotmat[2].y, &obj->rotmat[1].y, rotx);	
		rotate_x_first (&obj->rotmat[2].z, &obj->rotmat[1].z, rotx);

	}	

}





/*
 * Check if we are correctly aligned to dock.
 */

int is_docking (int sn)
{
	struct vector vec;
	float fz, ux;

	if(auto_pilot)		// Don't want it to kill anyone!
		return 1;
	
	fz = universe[sn].rotmat[2].z;

	if (fz > -0.90)
		return 0;
	
	vec = unit_vector (&universe[sn].location);

	if (vec.z < 0.927)
		return 0;
	
	ux = universe[sn].rotmat[1].x;
	if (ux < 0)
		ux = -ux;
	
	if (ux < 0.84)
		return 0;
	 
	return 1;
}





void update_altitude (int univ_n)
{
		
	myship.altitude = 255;

	if (witchspace) 
	    return;

	int x,y,z,dist;
	x = abs((int)universe[univ_n].location.x);
	y = abs((int)universe[univ_n].location.y);
	z = abs((int)universe[univ_n].location.z);
	
	if ((x > 65535) || (y > 65535) || (z > 65535))
		return;

	x /= 256;
	y /= 256;
	z /= 256;
	
	dist = (x * x) + (y * y) + (z * z);

	if (dist > 65535)
	    return;
	
	dist -= 9472;
	if (dist < 1)
	{
		myship.altitude = 0;
		do_game_over();
		return;
	}

	dist = sqrt (dist);
	if (dist < 1)
	{
		myship.altitude = 0;
		do_game_over ();
		return;
	}

	myship.altitude = dist;	
	if(myship.altitude <0) 
		myship.altitude= 255;
}


void update_cabin_temp (int univ_n)
{

	int x,y,z,dist;
	
	myship.cabtemp = 30;

	if (witchspace)
		return;
	
	if (ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
		return;
	
	x = abs((int)universe[univ_n].location.x);
	y = abs((int)universe[univ_n].location.y);
	z = abs((int)universe[univ_n].location.z);
	
	if ((x > 65535) || (y > 65535) || (z > 65535))
		return;

	x /= 256;
	y /= 256;
	z /= 256;
	
	dist = ((x * x) + (y * y) + (z * z)) / 256;

	if (dist > 255)
		return;

  	dist ^=  255;

	myship.cabtemp = dist + 30;
 
	if(myship.cabtemp > 255)
	{
		myship.cabtemp = 255;
		do_game_over ();
		return;
	}
	

	if(cmdr.fuel_scoop && myship.cabtemp >220)
	{
			
		if(cmdr.fuel <myship.max_fuel)
			cmdr.fuel+= flight_speed/10;

		info_message ("FUEL SCOOP ON", 40);	
	}

}






void make_station_appear (void)
{


	float px,py,pz;
	float sx,sy,sz;
	Vector vec;
	Matrix rotmat;
	
	px = universe[1].location.x;
	py = universe[1].location.y;
	pz = universe[1].location.z;
	
	vec.x = (rand() & 32767) - 16384;	
	vec.y = (rand() & 32767) - 16384;	
	vec.z = rand() & 32767;	

	vec = unit_vector (&vec);

	sx = px - vec.x * 65792;
	sy = py - vec.y * 65792;
	sz = pz - vec.z * 65792;

	set_init_matrix (rotmat);
	
	rotmat[0].x = 1.0;
	rotmat[0].y = 0.0;
	rotmat[0].z = 0.0;

	rotmat[1].x = vec.x;
	rotmat[1].y = vec.z;
	rotmat[1].z = -vec.y;
	
	rotmat[2].x = vec.x;
	rotmat[2].y = vec.y;
	rotmat[2].z = vec.z;

	tidy_matrix (rotmat);
	
	add_new_station (sx, sy, sz, rotmat);



//*** -------------------------------------

	int newship;
	int i, type;
	int rnd= ( (rand255() &3) * current_planet_data.government ) /2;

	for(i=0; i <rnd; i++)
	{
		type = SHIP_SIDEWINDER + (rand255() &7);
		newship = add_new_ship (type, sx+rand255()*100, sy+rand255()*100, sz+5000 +rand255()*100, rotmat, 0, 0);
		if (newship != -1)
		{
			universe[newship].flags = FLG_GOOD;   
			universe[newship].velocity = ship_list[type]->velocity;
			if(rand255() <50) 
				universe[newship].flags |= FLG_HAS_ECM;
			else
				universe[newship].flags |= FLG_FLY_TO_STATION;
			if(rand255()>200) universe[newship].energy+=50;
		}
	}

}



void check_docking (int i)
{
	if (is_docking(i))
	{
		dock_player();
		return;
	}
					
	if(flight_speed >5)
	{
		do_game_over();
		return;
	}

	flight_speed = 1;
	damage_ship (25, universe[i].location.z > 0);
}


void switch_to_view (struct univ_object *flip)
{
	float tmp;
	
	if ((current_screen == SCR_REAR_VIEW) ||
		(current_screen == SCR_GAME_OVER))
	{
		flip->location.x = -flip->location.x;
		flip->location.z = -flip->location.z;

		flip->rotmat[0].x = -flip->rotmat[0].x;
		flip->rotmat[0].z = -flip->rotmat[0].z;

		flip->rotmat[1].x = -flip->rotmat[1].x;
		flip->rotmat[1].z = -flip->rotmat[1].z;

		flip->rotmat[2].x = -flip->rotmat[2].x;
		flip->rotmat[2].z = -flip->rotmat[2].z;
		return;
	}


	if (current_screen == SCR_LEFT_VIEW)
	{
		tmp = flip->location.x;
		flip->location.x = flip->location.z;
		flip->location.z = -tmp;

		if (flip->type < 0)
			return;
		
		tmp = flip->rotmat[0].x;
		flip->rotmat[0].x = flip->rotmat[0].z;
		flip->rotmat[0].z = -tmp;		

		tmp = flip->rotmat[1].x;
		flip->rotmat[1].x = flip->rotmat[1].z;
		flip->rotmat[1].z = -tmp;		

		tmp = flip->rotmat[2].x;
		flip->rotmat[2].x = flip->rotmat[2].z;
		flip->rotmat[2].z = -tmp;		
		return;
	}

	if (current_screen == SCR_RIGHT_VIEW)
	{
		tmp = flip->location.x;
		flip->location.x = -flip->location.z;
		flip->location.z = tmp;

		if (flip->type < 0)
			return;
		
		tmp = flip->rotmat[0].x;
		flip->rotmat[0].x = -flip->rotmat[0].z;
		flip->rotmat[0].z = tmp;		

		tmp = flip->rotmat[1].x;
		flip->rotmat[1].x = -flip->rotmat[1].z;
		flip->rotmat[1].z = tmp;		

		tmp = flip->rotmat[2].x;
		flip->rotmat[2].x = -flip->rotmat[2].z;
		flip->rotmat[2].z = tmp;		

	}
}


/*
 * Update all the objects in the universe and render them.
 */

void update_universe (void)
{

	int i, type, flags;
	struct univ_object flip;


	mcount--;
	if(mcount < 0)
		mcount= 255;


	st_attacker= -999;

	if(cmdr.mission >-1000)
	{
		if(n_bad >0 && n_good>0)
			is_melee= TRUE; 
		else
			is_melee= FALSE;
	}

	if(cmdr.legal_status >100 || 
		police_is_angry || traders_are_angry)
		im_good= FALSE;
	else
		im_good= TRUE;


	int tmp_n_good= 0;
	int tmp_n_bad= 0;
	
	last_good= -1;
	last_bad= -1;
	n_angry_at_me= 0;
	n_attacking_station= 0;
	
	
	gfx_init_render();
	cross_color= GFX_COL_WHITE;

	for(i= 0; i <MAX_UNIV_OBJECTS; i++)
	{

		univ_obj_currently_being_updated= i;

		type = universe[i].type;
		flags= universe[i].flags;

		if(type != 0)
		{

			if((detonate_bomb) && !(flags &FLG_DEAD) &&
				(type !=SHIP_PLANET) && (type !=SHIP_SUN) &&
				(type !=SHIP_CONSTRICTOR) && (type !=SHIP_CORIOLIS) && (type !=SHIP_DODEC))
			{

				if(cmdr.mission !=6 || (flags &FLG_BAD))
					explode_object(i);

			}

			if(universe[i].energy <0)
				explode_object (i);

			if(flags &FLG_REMOVE)
			{
				remove_ship (i);
				continue;
			}



			static int tt[MAX_UNIV_OBJECTS];
			
			if(type >SHIP_ESCAPE_CAPSULE && 
				type <SHIP_SHUTTLE && universe[i].rotx !=0)
			{

				tt[i]++;
				
				if(tt[i]>15 || tt[i]<0)
				{
					universe[i].rotx= 0;
					tt[i]= 0;
				}			
		
			}

//*******************

			if( ((i^mcount)&7) == 0 &&  //AI UPDATE FREQUENCY (3, 7, 15, 31...
				(current_screen != SCR_INTRO_ONE) &&
				(current_screen != SCR_INTRO_TWO) &&
				(current_screen != SCR_GAME_OVER) &&
				(current_screen != SCR_ESCAPE_NOVA) &&
				(current_screen != SCR_ESCAPE_POD)) 
			{

				tactics (i);
			} 

			move_univ_object (&universe[i]);

			flip = universe[i];
			switch_to_view (&flip);

			if(type == SHIP_SUN )
			{
				draw_ship (&flip);
				continue;
			}
			else
			if(type == SHIP_PLANET)
			{
				if ((ship_count[SHIP_CORIOLIS] == 0) &&
					(ship_count[SHIP_DODEC] == 0) &&
					(myship.altitude > 200 ) && universe[i].distance <70000 ) // was 49152
				{

					make_station_appear();
				}				

				draw_ship (&flip);

				if(universe[0].distance < universe[1].distance)
				{
					struct univ_object ft;
					
					ft = universe[0];
					switch_to_view (&ft);
					draw_ship (&ft);
				}

				continue;
			}


			if(current_screen!=SCR_ESCAPE_NOVA && universe[i].distance > 75000) //need explosions in distance, dont remove when nova
			{

				if(type ==SHIP_ASTEROID && (cmdr.mission ==-1 || cmdr.mission ==-6))
				{
					message_count= 0;
					info_message ("Asteroid impact detected. Let's try again.", 100);
					asteroid_wave= -1;
	
				}
				else
				if(cmdr.mission ==-3 && type ==SHIP_COUGAR && zpino ==1)
				{
					message_count= 0;
					info_message ("COUGAR must be destroyed. Let's try again.", 100);
				}
				else
				if(cmdr.mission ==1 && type ==SHIP_CONSTRICTOR && cougar_pod ==1)
				{
					message_count= 0;
					info_message ("CONSTRICTOR must be destroyed. Let's try again.", 100);
				}
				else
				if(cmdr.mission ==6 && (type ==SHIP_CORIOLIS ||  type ==SHIP_DODEC))
				{
					message_count= 0;
					info_message ("You coward, i'll restart your computer now.", 100);
					dock_player();
				}


				remove_ship (i);
				continue;
			}


			if(universe[i].distance < 170)
			{
				if ((type == SHIP_CORIOLIS) || (type == SHIP_DODEC))
					check_docking (i);
				else
					scoop_item(i);
				
				continue;
			}




			draw_ship (&flip);

			universe[i].flags = flip.flags;
			universe[i].exp_seed = flip.exp_seed;
			universe[i].exp_delta = flip.exp_delta;
			universe[i].flags &= ~FLG_FIRING;
			
			if (universe[i].flags & FLG_DEAD)
				continue;

			check_target (i, &flip);

			if(i >2 && (flip.flags &FLG_GOOD))
			{
				tmp_n_good++;
				last_good= i;
			}

			if(i >2 && (flip.flags &FLG_BAD))
			{
				tmp_n_bad++;
				last_bad= i;
			}

			if(flip.flags & FLG_ANGRY)
				n_angry_at_me++;
			
			if(flip.target ==2)
				n_attacking_station++;

		}
	}


	detonate_bomb = 0;

	n_good= tmp_n_good;
	n_bad= tmp_n_bad;


//*** ----
	if ((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
		(current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW) ||
		(current_screen == SCR_INTRO_ONE) || (current_screen == SCR_INTRO_TWO) ||
		(current_screen == SCR_GAME_OVER) || (current_screen == SCR_ESCAPE_POD) || 
		(current_screen == SCR_ESCAPE_NOVA))
		update_starfield();

	gfx_finish_render();

}




/*
 * Update the scanner and draw all the lollipops.
 */

void update_scanner (void)
{

    if(docked) return;

	int i, f_dist, m_id= -1;

	int dx, dy, limit_x, limit_y;
	int x,y,z;
	int x1,y1,y2;
	int colour, ccol;
	
	float angle;

	
	condition = 1;

	closest_distance= 9999999;
	closest_missile_distance= 9999999;

	for (i = 0; i < MAX_UNIV_OBJECTS; i++)
	{

		if(universe[i].type <1) 
			continue;

		f_dist= universe[i].distance;

		if(universe[i].type ==SHIP_MISSILE && f_dist <closest_missile_distance) 
		{
			m_id= i;
			closest_missile_distance= f_dist;
		}
		else
		if(f_dist <closest_distance)
			closest_distance= f_dist;
		


		if ((universe[i].type == SHIP_MISSILE) ||
			((universe[i].type > SHIP_ROCK) && (universe[i].type < SHIP_DODEC)))
			condition = 2;


		if ((universe[i].type <= 0) ||
			(universe[i].flags & FLG_DEAD))
			continue;


		if( (universe[i].flags & FLG_CLOAKED) && rand255()>15 )
			continue;
	
		x = universe[i].location.x / 256;
		y = universe[i].location.y / 256;
		z = universe[i].location.z / 256;

		if(scanner_zoom==2)
		{
			x1 = x*4;
			y1 = -z ;
			y2 = y1 - y *2;
		}
		else if(scanner_zoom==1)
		{
			x1 = x;
			y1 = -z /4 ;
			y2 = y1 - y /2;

		}


		angle= 270*PI/180+ atan2( x1, -y2) ;
		if(angle < 0) angle+= 360;

		x1= scanner_cx + x1;
		y1= scanner_cy + y1;
		y2= scanner_cy + y2;

		
		dx= (int)scanner_w*cos(angle);
		dy= (int)scanner_h*sin(angle);


		limit_x= scanner_cx+dx;
		limit_y= scanner_cy+dy;


		if(dx>0 && x1>limit_x)  x1= limit_x; 
		if(dx<=0 && x1<limit_x)  x1= limit_x;

		if(dy>0 && y2>limit_y) y2= limit_y; 
		if(dy<=0 && y2<limit_y) y2= limit_y;
		
		if(y1< scanner_cy-scanner_h) y1= scanner_cy-scanner_h;
		if(y1> scanner_cy+scanner_h) y1= scanner_cy+scanner_h;


		colour = (universe[i].flags & FLG_HOSTILE) ? GFX_COL_RED_1 : GFX_COL_WHITE;
			
		switch (universe[i].type)
		{
			case SHIP_MISSILE:
				colour = GFX_COL_PINK_1;
				break;

			case SHIP_DODEC:
			case SHIP_CORIOLIS:
				colour = GFX_COL_GREEN_2;
				break;
				
			case SHIP_VIPER:
				if(mcount%2 ==0 && (universe[i].flags &FLG_HOSTILE)) colour = GFX_COL_BLUE_4;
				else
					colour = GFX_COL_BLUE_2;
				
				break;

			case SHIP_THARGOID:
				if(universe[i].flags &FLG_HOSTILE) 
				{
					if(mcount%2 ==0) colour = GFX_COL_RED_1;
					else
						colour = GFX_COL_PINK_3;
				}
				else
					colour = GFX_COL_RED_2;

				break;


			case SHIP_ASTEROID:
			case SHIP_BOULDER:
			case SHIP_CARGO:
			case SHIP_ALLOY:
			case SHIP_ROCK:
				colour = GFX_COL_GREY_2;
				break;

			case SHIP_ESCAPE_CAPSULE:
				colour = GFX_COL_YELLOW_3;
				break;

		}
			



		//scanner blob --> to Quads
		gfx_draw_colour_line (x1+2, y2,   x1-3, y2, colour);
		gfx_draw_colour_line (x1+2, y2+1, x1-3, y2+1, colour);
		gfx_draw_colour_line (x1+2, y2+2, x1-3, y2+2, colour);
		gfx_draw_colour_line (x1+2, y2+3, x1-3, y2+3, colour);

		//vertical diff
		gfx_draw_colour_line (x1+1, y1, x1+1, y2, colour);

	}



//*** condition

	ccol= GFX_COL_WHITE;

	if (docked) condition = 0;
	else
	if ((condition == 2) && (energy < 128))
		condition = 3;

	switch(condition)
	{
		case 1: ccol= GFX_COL_GREEN_1; break;
		case 2: ccol= GFX_COL_ORANGE_1; break;
		case 3: ccol= GFX_COL_RED_1; break;
	}


//	gfx_display_colour_text (105, 390, "Cond.", GFX_COL_WHITE);
	
	gfx_display_colour_text (105, 390, condition_txt[condition], ccol);

	sprintf(str, "(%d/255)", cmdr.legal_status);
	if(cmdr.legal_status >100 && !(mcount%10))
		gfx_display_colour_text (105, 405, str, GFX_COL_RED_3);
	else
	if(cmdr.legal_status >50 && !(mcount%25))
		gfx_display_colour_text (105, 405, str, GFX_COL_RED_3);
	else
		gfx_display_colour_text (105, 405, str, GFX_COL_BLUE_5);


//*** ------

	if(closest_missile_distance >99999)
		sprintf(str, "----.- cm");
	else
		sprintf(str, "%06.1f cm", (float)(closest_missile_distance/100));

	int tgt= -1;
	
	if(m_id >0)
		tgt= universe[m_id].target;

	if(closest_missile_distance <5000 && (!(mcount%2) && tgt ==0))
		ccol= GFX_COL_RED_2;
	else
		ccol= GFX_COL_BLUE_5;

	gfx_display_colour_text (158, 491, str, ccol);





	if(universe[0].distance <50010 || universe[1].distance <50010)
	{
		ccol= GFX_COL_RED_2;
		closest_distance= (universe[0].distance <universe[1].distance) ?
							universe[0].distance : universe[1].distance;
	}
	else
	if(closest_distance <10000)
		ccol= GFX_COL_BLUE_5;
	else
		ccol= GFX_COL_GREEN_3;

	if(closest_distance >99999)
		sprintf(str, "cc ----.-");
	else
		sprintf(str, "cc %06.1f", (float)closest_distance/100);

	gfx_display_colour_text (270, 491, str, ccol);


//*** --- score
	glColor3f(0,1,0);
	sprintf(str, "Score"); 
	txtOut(340, 425, str, 2);

	glColor3f(1,1,1);
	sprintf(str, "%06d", cmdr.score); 
	txtOut(395, 425, str, 2);



}


/*
 * Update the compass which tracks the space station / planet.
 */

void update_compass (void)
{
	struct vector dest;
	int compass_x;
	int compass_y;
	int un = 1; //planet

	if (witchspace)
		return;
	
	if (ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
		un = 2; //station

	dest = unit_vector (&universe[un].location);
	
	compass_x = compass_centre_x + (dest.x * 18);
	compass_y = compass_centre_y + (dest.y * -18);
	
	gfx_draw_circle (compass_centre_x, compass_centre_y, 18, GFX_COL_WHITE);

	if (dest.z < 0)
		gfx_draw_circle (compass_x, compass_y, 4, GFX_COL_GREEN_3);
	else
		gfx_draw_filled_circle (compass_x, compass_y, 5, GFX_COL_GREEN_2);


	//*** sun
	dest = unit_vector (&universe[0].location);
	
	compass_x = compass_centre_x + (dest.x * 18);
	compass_y = compass_centre_y + (dest.y * -18);
	

	if (dest.z < 0)
		gfx_draw_circle (compass_x, compass_y, 3, GFX_COL_ORANGE_3);
	else
		gfx_draw_filled_circle (compass_x, compass_y, 3, GFX_COL_YELLOW_3);
				
}




/*
 * Draw an indicator bar.
 * Used for shields and energy banks.
 */

void display_dial_bar (int len, int x, int y)
{
	int i = 0;

	for (i = 0; i < 10; i++)
		gfx_draw_colour_line (x, y + i + 384, x + len, y + i + 384, GFX_COL_YELLOW_5);

}




/*
 * Display the current shield strengths.
 */

void display_shields (void)
{

gfx_display_text(5,374+14, "FS");
gfx_display_text(5,374+14 +17, "RS");

	if (front_shield > 3)
		display_dial_bar (front_shield / 4, 31, 14 );

	if (aft_shield > 3)
		display_dial_bar (aft_shield / 4, 31, 14+17);
}



void display_fuel (void)
{
gfx_display_text(5,374+14 +17*2, "FU");

int temp_f= cmdr.fuel; 


if (temp_f>myship.max_fuel) temp_f=myship.max_fuel;
//this intead of above if (cmdr.fuel > myship.max_fuel) cmdr.fuel=myship.max_fuel;
if(cmdr.fuel<0) cmdr.fuel=0;;

	if (cmdr.fuel > 0)
		display_dial_bar ((temp_f * 64) / myship.max_fuel-1, 31, 14+17*2);


}

void display_cabin_temp (void)
{
gfx_display_text(5,374+14 +17*3, "CT");

	if (myship.cabtemp > 3)
		display_dial_bar (myship.cabtemp / 4, 31, 14+17*3);
}


void display_laser_temp (void)
{
gfx_display_text(5,374+14 +17*4, "LT");

	if (laser_temp > 0)
		display_dial_bar (laser_temp / 4, 31, 14+17*4);
}

void display_altitude (void)
{

gfx_display_text(5,374+14 +17*5, "AL");

	if (myship.altitude > 3)
		display_dial_bar (myship.altitude / 4, 31, 14+17*5);
}



void display_missiles (void)
{
	int nomiss;
	int x,y;

	gfx_display_text(5,374+14 +17*6, "MS");

	if (cmdr.missiles == 0)
		return;
	
	nomiss = cmdr.missiles > 4 ? 4 : cmdr.missiles;

	x = GFX_X_OFFSET + (4 - nomiss) * 16 + 33;
	y = GFX_Y_OFFSET + 114 + 386;
	
	if (missile_target != MISSILE_UNARMED)
	{

		glColor3f(1,1,1);
		glBegin(GL_LINE_LOOP);

			glVertex2f(x, y);
			glVertex2f(x+10, y);
			glVertex2f(x+10, y+10);
			glVertex2f(x, y+10);

		glEnd();

		if(missile_target>=0) glColor3f(1,0,0);
		else
			glColor3f(1,1,0);
		
		glBegin(GL_QUADS);

			glVertex2f(x+2, y+1);
			glVertex2f(x+9, y+1);
			glVertex2f(x+9, y+8);
			glVertex2f(x+2, y+8);

		glEnd();

		x += 16;
		nomiss--;
	}

	glColor3f(1,1,1);
	for (; nomiss > 0; nomiss--)
	{

		glBegin(GL_LINE_LOOP);

			glVertex2f(x, y);
			glVertex2f(x+10, y);
			glVertex2f(x+10, y+10);
			glVertex2f(x, y+10);

		glEnd();

		x += 16;
	}
}



/*
 * Display the speed bar.
 */

void display_speed (void)
{
/*
	if(flight_speed ==100)
	{

		int i = 0;
		for (i = 0; i < 10; i++)
			gfx_draw_colour_line (416, 14 + i + 384, 416 + 64, 14 + i + 384, GFX_COL_RED_2);
	}
	else
	{

*/

	gfx_display_text(486,374+14 , "SP");

	int len = ((flight_speed * 64) / myship.max_speed) ;
	display_dial_bar (len, 416, 14);

}



void display_flight_roll (void)
{
	int sx,sy;
	int i;
	int pos;

	gfx_display_text(486,374+14 +17, "RL");

	sx = 414;
	sy = 384 + 14+17;

	pos = sx - ((flight_roll * 30) / myship.max_roll);
	pos += 32;

	for (i = 0; i < 4; i++)
	{
		gfx_draw_colour_line (pos + i, sy, pos + i, sy + 9, GFX_COL_YELLOW_1);
	}
}

void display_flight_climb (void)
{
	int sx,sy;
	int i;
	int pos;

	gfx_display_text(486,374+14 +17*2, "DC");

	sx = 414;
	sy = 384 + 14+17*2;

	pos = sx + ((flight_climb * 30) / myship.max_climb);
	pos += 32;

	for (i = 0; i < 4; i++)
	{
		gfx_draw_colour_line (pos + i, sy, pos + i, sy + 9, GFX_COL_YELLOW_1);
	}
}


/*
 * Display the energy banks.
 */

void display_energy (void)
{
	int e1,e2,e3,e4;

	gfx_display_text(486,374+14 +17*3, "e1");
	gfx_display_text(486,374+14 +17*4, "e2");
	gfx_display_text(486,374+14 +17*5, "e3");
	gfx_display_text(486,374+14 +17*6, "e4");

	e1 = energy > 64 ? 64 : energy;
	e2 = energy > 128 ? 64 : energy - 64;
	e3 = energy > 192 ? 64 : energy - 128;
	e4 = energy - 192;  	
	
	if (e4 > 0)
		display_dial_bar (e4+1, 416, 14+17*3);

	if (e3 > 0)
		display_dial_bar (e3, 416, 14+17*4);

	if (e2 > 0)
		display_dial_bar (e2, 416, 14+17*5);

	if (e1 > 0)
		display_dial_bar (e1, 416, 14+17*6);
}






void update_console (void)
{

//	gfx_set_clip_region (0, 0, 512, 512);
//*************AAAAAAAAAAAAAAAAAAAAA
    gfx_ScissorHack();


	gfx_draw_scanner();
	
	display_speed();
	display_flight_climb();
	display_flight_roll();
	display_shields();
	display_altitude();
	display_energy();
	display_cabin_temp();
	display_laser_temp();
	display_fuel();
	display_missiles();
	
	update_scanner();

	if (docked)
		return;
	update_compass();

	glColor3f(1,1,1);
	if (ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
		txtOut(387+GFX_X_OFFSET, 490+GFX_Y_OFFSET, "S", 5);

	if(ecm_active && (mcount%3 == 0))
		txtOut(101+GFX_X_OFFSET, 490+GFX_Y_OFFSET, "E", 5);

	if(is_melee)
	{

		glColor3f(0,0,1);
		if(im_good)
			sprintf(str, ">%03d", n_good);
		else
			sprintf(str, "%03d", n_good);

		gfx_display_colour_text (105, 425, str, GFX_COL_BLUE_3);


		glColor3f(1,0,0);
		if(!im_good)
			sprintf(str, ">%03d", n_bad);
		else
			sprintf(str, " %03d", n_bad);

		gfx_display_colour_text (105, 440, str, GFX_COL_RED_3);

	}

}

void increase_flight_roll (float dr)
{
	if (flight_roll < myship.max_roll)
		flight_roll+= dr;
	flight_roll+= dr;
	if (flight_roll > myship.max_roll)
		flight_roll= myship.max_roll;
	else
	if(flight_roll> 0 && flight_roll< 1) 
		flight_roll= 0;
}


void decrease_flight_roll (float dr)
{

	flight_roll-= dr;
	if (flight_roll < -myship.max_roll)
		flight_roll= -myship.max_roll;
	else
	if(flight_roll< 0 && flight_roll> -1) 
		flight_roll= 0;
}


void increase_flight_climb (float dc)
{

	flight_climb+= dc;
	if (flight_climb > myship.max_climb)
		flight_climb= myship.max_climb;
	else
	if(flight_climb> 0 && flight_climb<1) 
		flight_climb= 0;

}

void decrease_flight_climb (float dc)
{
	flight_climb-= dc;
	if (flight_climb < -myship.max_climb)
		flight_climb= -myship.max_climb;
	else
	if(flight_climb< 0 && flight_climb> -1) 
		flight_climb= 0;
}


void start_hyperspace (void)
{
	if (hyper_ready)
		return;
		
	hyper_distance = calc_distance_to_planet (docked_planet, hyperspace_planet);

	if ((hyper_distance == 0) || (hyper_distance > cmdr.fuel))
		return;

	destination_planet = hyperspace_planet;
	name_planet (hyper_name, destination_planet);
	capitalise_name (hyper_name);
	
	hyper_ready = 1;
	hyper_countdown = 10;
	hyper_galactic = 0;

	disengage_auto_pilot();
}

void start_galactic_hyperspace (void)
{
	if (hyper_ready)
		return;

	if (cmdr.galactic_hyperdrive == 0)
		return;

	hyper_ready = 1;
	hyper_countdown = 15; 
	hyper_galactic = 1;
	disengage_auto_pilot();
}



void display_hyper_status (void)
{
	if((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
		(current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW))
	{

		if (hyper_galactic)
		{
			gfx_display_centre_text (358, "Galactic Hyperspace", 120, GFX_COL_WHITE);
		}
		else
		{
			sprintf (str, "Hyperspace - %s", hyper_name);
			gfx_display_centre_text (358, str, 120, GFX_COL_WHITE);
		} 	
	}

	sprintf (str, "%d", hyper_countdown);	
	gfx_display_text (482, 5, str);
}


int rotate_byte_left (int x)
{
	return ((x << 1) | (x >> 7)) & 255;
}

void enter_next_galaxy (void)
{
	cmdr.galaxy_number++;
	cmdr.galaxy_number &= 7;
	
	cmdr.galaxy.a = rotate_byte_left (cmdr.galaxy.a);
	cmdr.galaxy.b = rotate_byte_left (cmdr.galaxy.b);
	cmdr.galaxy.c = rotate_byte_left (cmdr.galaxy.c);
	cmdr.galaxy.d = rotate_byte_left (cmdr.galaxy.d);
	cmdr.galaxy.e = rotate_byte_left (cmdr.galaxy.e);
	cmdr.galaxy.f = rotate_byte_left (cmdr.galaxy.f);

	docked_planet = find_planet (0x60, 0x60);
	hyperspace_planet = docked_planet;
}





void enter_witchspace (void)
{
	int i;
	int nthg;

	witchspace = 1;
	remb_db= docked_planet.b;
	docked_planet.b ^= 31;
	in_battle = 50;  

	flight_speed = 12;
	flight_roll = 0;
	flight_climb = 0;
	create_new_stars();
	clear_universe();

	nthg = 1 +(rand255() & 3); 
	for (i = 0; i < nthg; i++)
		create_thargoid();	
	
	current_screen = SCR_BREAK_PATTERN;
//	snd_play_sample (SND_HYPERSPACE);

	if(rand255() <10) 			
		info_message ("WHY DID THE GROIGAN DANCE... HRGH, HRGH, HRRGH", 100);
	else
	if(rand255() <10) 			
		info_message ("WHY WAS THE KANGAROO HAPPY... HRG, HRGH, HGREE", 100);

}


void complete_hyperspace (void)
{
	Matrix rotmat;
	int px,py,pz;
	int px2,py2,pz2;
	
	hyper_ready = 0;
	witchspace = 0;
	
	if(is_melee || (cmdr.mission == -1) || (cmdr.mission == -6) ||  (cmdr.mission < -1000) )
	{
		message_count= 0;
		info_message ("Abnormal mass present, Jump aborted.", 40);
		return;
	} 


	if((cmdr.mission == 6))
	{
		message_count= 0;
		info_message ("Hyperdrive Jammer detected, Jump aborted.", 40);
		return;
	} 



	if(hyper_galactic)
	{
		if(cmdr.mission<1 || (cmdr.galaxy_number ==1 && cmdr.mission <3) ||
			(cmdr.galaxy_number ==2 && cmdr.mission <10) )
		{

			message_count= 0;
			info_message ("Still have a job to do in this Galaxy.", 100);
			return;
		}

		cmdr.galactic_hyperdrive--; //aaaaaaaa more than 1, more than 1 energy bomb??
		enter_next_galaxy();
		cmdr.legal_status = 0;
		cmdr.fuel = 1;


	}
	else
	{
		
		cmdr.fuel -= hyper_distance;

		cmdr.legal_status-= hyper_distance/5;

		if(cmdr.legal_status <carrying_contraband()) 
			cmdr.legal_status= carrying_contraband();

		if( rand255() <10 || (cmdr.mission >=5 && rand255() <30) ||
			(flight_climb ==myship.max_climb && rand255() <100) )
		{
			enter_witchspace();
			return;
		}

		docked_planet = destination_planet; 
	}

	cmdr.market_rnd = rand255();
	generate_planet_data (&current_planet_data, docked_planet);
	generate_stock_market ();
	
	flight_speed = 12;
	flight_roll = 0;
	flight_climb = 0;
	create_new_stars();

	clear_universe();

	set_init_matrix (rotmat);

	pz = (((docked_planet.b) & 7) + 7) / 2;
	px = pz / 2;
	py = px;

	px <<= 16;
	py <<= 16;
	pz <<= 16;
	

	if ((docked_planet.b & 1) == 0)
	{
		px = -px;
		py = -py;
	}

	px2= px;
	py2= py;
	pz2= pz;


	if(docked_planet.f>100)
	{
		pz = -(((docked_planet.f & 7) | 1) << 16);
		px = ((docked_planet.e & 3) << 16) | ((docked_planet.e & 3) << 8);
		py = ((docked_planet.d & 3) << 16) | ((docked_planet.d & 3) << 8);
	}
	else
	{
		pz = 200000+(((docked_planet.f & 7) | 1) << 16);
		px = 70000+((docked_planet.e & 3) << 16) | ((docked_planet.e & 3) << 8)*200;
		py = 70000+((docked_planet.d & 3) << 16) | ((docked_planet.d & 3) << 8)*200;
	}
	
	add_new_ship (SHIP_SUN, px, py, pz, rotmat, 0, 0);
	add_new_ship (SHIP_PLANET, px2, py2, pz2, rotmat, 0, 0);


	if(razaar_refugees>100 && (docked_planet.d == 112) && 
		(docked_planet.b == 95) && (cmdr.galaxy_number == 0))
		universe[1].type= SHIP_SUN;
	

	if(cmdr.mission == -4 && (docked_planet.d == 112) && 
		(docked_planet.b == 95) && (cmdr.galaxy_number == 0))
	{

		cmdr.fuel= 35;
				
	}


	current_screen = SCR_BREAK_PATTERN;
}


void countdown_hyperspace (void)
{
	hyper_countdown--;

	if(hyper_countdown <=0)
	{
		hyper_countdown= 0;
		complete_hyperspace();
	}

}



void jump_warp (void)
{

	int i, jump;
	
	if(flight_speed <myship.max_speed)
	{
		info_message ("VELOCITY LOCKED", 40);
		return;
	}

	if(closest_distance <10000)
	{
		info_message ("MASS LOCKED", 40);
		return;
	}


	if(universe[0].distance <50010 || universe[1].distance <50010)
	{
		info_message ("MASS LOCKED", 40);
		return;
	}


	if(universe[0].distance < universe[1].distance)
		jump = universe[0].distance - 50000;
	else
		jump = universe[1].distance - 50000;	

	if(jump > 1024)
		jump = 1024;
	
	for(i = 0; i < MAX_UNIV_OBJECTS; i++)
	{
		if (universe[i].type != 0)
			universe[i].location.z -=jump;
	}

	warp_stars = 1;
	mcount &= 63;

	if(closest_distance >99999)
	{
		in_battle = 0;
		traders_are_angry= FALSE;
		police_is_angry= FALSE;
		convoy_present= FALSE;
	}

}


void launch_player (void)
{
	Matrix rotmat;
	int px, py, pz;

	int i;

	docked = 0;
	flight_speed = 12;
	flight_roll = -15;
	flight_climb = 0;
	cmdr.legal_status+= carrying_contraband();
	create_new_stars();
	clear_universe();

	px= 0;
	py= 0;
	pz= 65536;

	if(docked_planet.f>100)
	{
		pz = -(((docked_planet.f & 7) | 1) << 16);
		px = ((docked_planet.e & 3) << 16) | ((docked_planet.e & 3) << 8);
		py = ((docked_planet.d & 3) << 16) | ((docked_planet.d & 3) << 8);
	}
	else
	{
		pz = 200000+(((docked_planet.f & 7) | 1) << 16);
		px = 70000+((docked_planet.e & 3) << 16) | ((docked_planet.e & 3) << 8)*200;
		py = 70000+((docked_planet.d & 3) << 16) | ((docked_planet.d & 3) << 8)*200;
	}


	set_init_matrix (rotmat);

	add_new_ship (SHIP_SUN, px, py, pz, rotmat, 0, 0);
	add_new_ship (SHIP_PLANET, 0, 0, 65536, rotmat, 0, 0);

	rotmat[2].x = -rotmat[2].x;
	rotmat[2].y = -rotmat[2].y;
	rotmat[2].z = -rotmat[2].z;
	add_new_station (0, 0, -256, rotmat);



//asteroid
	if((cmdr.mission == -1) || (cmdr.mission == -6))
	{		
	
		asteroid_wave=0;
		float ax, ay, az;
		
		int cnt= 10;

		if(cmdr.mission ==-6)
		{
	
			for(i=0; i <9;i++)
				create_trader();

			cnt= 40;
		}
	
		for(i=0; i <cnt;i++)
		{

			ax= universe[2].location.x +100+rand255()*15;
			ay= universe[2].location.y -700+rand255()*10; 
			az= universe[2].location.z +1000+rand255()*15;

			int new_ship = add_new_ship (SHIP_ASTEROID, ax, ay, az, universe[2].rotmat, 
											0, (rand255() &3)-2);
			
			if(new_ship> 2)
			{
				universe[new_ship].velocity = 21;
				universe[new_ship].flags |= FLG_BAD;
			}
		}




	}
	else 
	if(gal_fam !=-500 && docked_planet.d == 150 && docked_planet.b == 130 && cmdr.galaxy_number == 2)
	{
	

		//starving zaisedan
	}
	else
	if (cmdr.mission == 6)
	{

		//*** last mission, add thargoids
		tharg_wave= 0;

		for (i = 0; i < 35; i++)
			create_thargoid();	

			create_traders_convoy();
			create_viper_patrol();
			create_viper_patrol();

	}
	else
	if(cmdr.mission >-1000)
	{
	
		int i;
		int rnd= ( (rand255() &3) * current_planet_data.government ) /2;
		
		if(cmdr.mission== -5 && razaar_refugees ==100)
		{

			if(cmdr.credits > 950)	
				nova_timer= 170; 
			else
				nova_timer= 210;

			create_viper_patrol();
			create_viper_patrol();

			rnd=300; //everyone is running avay from NOVA
		}
		else
		if(rand255() <(20 - current_planet_data.government*3)) //gvmnt 0-7
			create_melee();

		for(i=0; i< rnd;i++) 
			create_trader();

	}
		
	current_screen = SCR_BREAK_PATTERN;
	
	LAST_GALAXY= cmdr.galaxy;
	last_glx= cmdr.galaxy_number;

	LAST_DOCKED= docked_planet;

}



/*
 * Engage the docking computer.
 * For the moment we just do an instant dock if we are in the safe zone.
 */

void engage_docking_computer (void)
{
	if (ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
	{
		dock_player();
		current_screen = SCR_BREAK_PATTERN;
	}
}

