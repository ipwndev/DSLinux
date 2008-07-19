

// - - - - - - - - - - -  F I L E   E N D - - - - - - - - - - - 






void create_viper_patrol(void);
void create_traders_convoy(void);
void create_melee (void);



void do_the_scoring(int type, int flags)
{

	if(flags &FLG_GOOD)
	{
		if(type== SHIP_VIPER)
			cmdr.legal_status+= 50;
		else
			cmdr.legal_status+= 30;

		if(cmdr.legal_status > 255) 
			cmdr.legal_status= 255;

	}
	else
	if(flags &FLG_BAD)
	{
		global_bounty+= ship_list[type]->bounty;
		cmdr.score+= ship_list[type]->bounty;

		if(cmdr.legal_status >carrying_contraband()+5)
			cmdr.legal_status-= 5;

		if(in_battle >0) 
			in_battle--;
	}

	cmdr.score+= ship_list[type]->energy;


//*** ----
	static int scr= 10000;

	if(cmdr.score >=scr)
	{
		scr+= 10000;
		message_count= 0;
		
		if(rand255()>100) 
			info_message ("RIGHT ON, COMMANDER", 100);
		else
			info_message ("GOOD SHOOTING, COMMANDER", 100);
		
	}

}


void clear_universe (void)
{
	int i;

	for(i = 0; i < MAX_UNIV_OBJECTS; i++)
		universe[i].type = 0;

	for (i = 0; i <= NO_OF_SHIPS; i++)
		ship_count[i] = 0;


	in_battle = 0;
	is_melee= FALSE;

	traders_are_angry= FALSE;
	police_is_angry= FALSE;
	convoy_present= FALSE;

	message_count=0;
	missile_target = MISSILE_UNARMED;

	station_is_exploding= FALSE;
}


int add_new_ship (int ship_type, int x, int y, int z, struct vector *rotmat, int rotx, int rotz)
{

	int i, fr;

	if(ship_type >0 && ship_type!=SHIP_CORIOLIS && ship_type!=SHIP_DODEC) fr= 3; else fr= 0;

	for(i = fr; i < MAX_UNIV_OBJECTS; i++)
	{
		if( universe[i].type ==0 )
		{
			universe[i].type = ship_type;
			universe[i].location.x = x;
			universe[i].location.y = y;
			universe[i].location.z = z;
			
			universe[i].distance = sqrt(x*x + y*y + z*z);

			universe[i].rotmat[0] = rotmat[0];
			universe[i].rotmat[1] = rotmat[1];
			universe[i].rotmat[2] = rotmat[2];

			universe[i].rotx = rotx;
			universe[i].rotz = rotz;
			
			universe[i].velocity = 0;
			universe[i].acceleration = 0;
			universe[i].bravery = 0;
			universe[i].target = 0;
			
			universe[i].flags = initial_flags[ship_type];

			if ((ship_type != SHIP_PLANET) && (ship_type != SHIP_SUN))
			{
				universe[i].energy = ship_list[ship_type]->energy;
				universe[i].missiles = ship_list[ship_type]->missiles;
				ship_count[ship_type]++;
			}
			

			if(ship_type ==SHIP_MISSILE)
			{
				mm_t[i]= TRUE;
				mm_current[i]= 0;
				int imt;
				for(imt=0; imt<5; imt++) 
				{
					mm_trailX[i][imt]= x;
					mm_trailY[i][imt]= y;
					mm_trailZ[i][imt]= z;

				}

			}

			return i;
		}
	}

	return -1;
}




void check_missiles (int un)
{

	int i;
	
	if(missile_target == un)
	{
		missile_target = MISSILE_UNARMED;
		info_message ("TARGET LOST", 40);
	}

	for(i = 0; i < MAX_UNIV_OBJECTS; i++)
	{
		if((universe[i].type == SHIP_MISSILE) && (universe[i].target == un))
			universe[i].flags |= FLG_DEAD;
	}



}


void remove_ship (int un)
{

	int type= universe[un].type;

	if(station_is_exploding && (type ==SHIP_CORIOLIS || type ==SHIP_DODEC))
	{
	//messy - move to explode_ship
		if(universe[2].location.z <15000)
			universe[2].location.z+= 250;


		if(universe[2].location.z >3000)
		{
			st_exp_timer--; 
			if(st_exp_timer <0)
			{
				game_over= TRUE;
				current_screen= SCR_GAME_OVER;
				st_exp_timer= 0;
			}

		}

		auto_pilot = 1;


		int i, ttp;
		for (i = 0; i < 10; i++)
		{

			universe[50 +i].type= 0;

			if(!(i%3))
				ttp= SHIP_DODEC;						
			else
				ttp= SHIP_CARGO;

			launch_enemy(un, ttp, FLG_DEAD | FLG_INACTIVE,0);
		}


	}


	ship_count[type]--;
	universe[un].type = 0;	

	mm_t[un]= FALSE;
	check_missiles (un);

}


void add_new_station (float sx, float sy, float sz, Matrix rotmat)
{
	int station;

	station = (current_planet_data.techlevel >= 10) ? SHIP_DODEC : SHIP_CORIOLIS;
//	universe[2].type = 0;
	add_new_ship (station, sx, sy, sz, rotmat, 0, -1);			

	station_is_exploding= FALSE;
}
	


	
void reset_weapons (void)
{
	laser_temp = 0;
	laser_counter = 0;
	laser = 0;
	ecm_active = 0;
	missile_target = MISSILE_UNARMED;
}

 
void launch_enemy(int un, int type, int flags, int bravery)
{

	struct univ_object *ns;

	int launch_type= universe[un].type;
	int newship = add_new_ship (type, universe[un].location.x, universe[un].location.y,
							universe[un].location.z, universe[un].rotmat,
							universe[un].rotx, universe[un].rotz);

	if(newship == -1) return;
	
	ns = &universe[newship];
	if(flags >0) 
		ns->flags = flags;


	ns->velocity= ship_list[type]->velocity;
	
	if(launch_type ==SHIP_CORIOLIS || launch_type ==SHIP_DODEC)
	{
		//ns->velocity = 29;
		ns->location.x += ns->rotmat[2].x * 2; 		
		ns->location.y += ns->rotmat[2].y * 2; 		
		ns->location.z += ns->rotmat[2].z * 2;

		ns->rotz = ((rand255() * 2) & 255) - 128;
	}

	
 //****
	if(type == SHIP_MISSILE)
		ns->velocity= 25 +universe[un].velocity/2;
	else
	if((type == SHIP_CARGO) || (type == SHIP_ALLOY) || (type == SHIP_ROCK))
	{

		ns->rotx= (rand255() &3)-2;
		ns->rotz= (rand255() &3)-2;
		ns->velocity = 1+(rand255() &3);

		ns->bravery= rand255() & 7; //choose cargo type here
		
		ns->location.x+= rand255()-128;
		ns->location.y+= rand255()-128;
		ns->location.z+= rand255()-128;
	}

}


void launch_loot (int un, int loot)
{
	int i,cnt;

	if (loot == SHIP_ROCK)
	{
		cnt = rand255() & 3;
	}
	else
	{
		cnt = rand255();
		if(cnt >= 128)
			return;

		cnt &= ship_list[universe[un].type]->max_loot;
		cnt &= 15;
	}

	for (i = 0; i < cnt; i++)
		launch_enemy (un, loot, FLG_INACTIVE,0);

}




int in_target (int type, float x, float y, float z)
{
	float size;
	
	if (z < 0)
		return 0;
		
	size = ship_list[type]->size;

	return ((x*x + y*y) <= size+z/2);	
}



void make_angry (int un)
{

	int type;
	int flags;
	
	type = universe[un].type;
	flags = universe[un].flags;


	
	if((flags &FLG_INACTIVE))
		return;
	else
	if(ship_list[type]->energy <300) 
		universe[un].rotz= (rand255() &3) - 2;


//*** ---
	if(!is_melee)
	{

		if(type >SHIP_ROCK && type <SHIP_COBRA3)	//shuttle & transporter
			universe[un].flags = FLG_FLY_TO_STATION;
		else
			universe[un].flags |= FLG_ANGRY;


		if(convoy_present && (flags &FLG_GOOD))
			traders_are_angry= TRUE;

		if((flags &FLG_GOOD) && (traders_are_angry || ship_count[SHIP_VIPER] ||  
			ship_count[SHIP_DODEC] || ship_count[SHIP_CORIOLIS]))
		{
			police_is_angry= TRUE;
		}

	}
}



void explode_object (int un)
{

	struct univ_object *univ;
	univ = &universe[un];

	if(universe[un].flags & FLG_DEAD) 
		return;

	//	snd_play_sample (SND_EXPLODE);

	universe[un].flags |= FLG_DEAD;


	if (cmdr.mission ==-3 && universe[un].type ==SHIP_COUGAR)
		cmdr.mission= -104;


	if (cmdr.mission ==1 && universe[un].type == SHIP_CONSTRICTOR)
		cmdr.mission= 2;


	if (cmdr.mission ==3 && gal_fam> -10 && universe[un].type == SHIP_CARGO &&  universe[un].bravery == -101 )
	{

		if(!detonate_bomb ||  myship.altitude > 50 || universe[un].distance >5000 || 
			gal_fam >0 || docked_planet.d !=150 || docked_planet.b !=130)
		{

			gal_fam= -999;
				message_count= 0;
				info_message ("World of Zaisedan is left with no hope. Mission failed.", 100);

		}
		else
		{
			
			gal_fam--; 
			
			if(gal_fam ==-10){

				message_count= 0;
				info_message ("Hopefully that will do Commander.", 100);
				gal_fam= -500;

			}

		}
	}


//**** ---
	if(!station_is_exploding && univ->type ==SHIP_CORIOLIS || univ->type ==SHIP_DODEC)
	{
		station_is_exploding= TRUE;
		st_exp_timer= 45;
	}

}



void check_target (int un, struct univ_object *flip)
{
	struct univ_object *univ;
	
	univ = &universe[un];

	int type= univ->type;
	int i, max_e= ship_list[type]->energy;

	if(in_target (type, flip->location.x, flip->location.y, flip->location.z))
	{
	
		if( current_screen ==SCR_FRONT_VIEW || 
			(current_screen ==SCR_REAR_VIEW && cmdr.rear_laser >0) ||
			(current_screen ==SCR_LEFT_VIEW && cmdr.left_laser >0) ||
			(current_screen ==SCR_RIGHT_VIEW && cmdr.right_laser >0) )
		{
			if(missile_target ==MISSILE_ARMED && type >= SHIP_MISSILE)
			{

				if(type ==SHIP_COUGAR)
				{

					sprintf(str, "UNABLE TO LOCK-ON - %s(%d)", ship_list[type]->name, univ->energy);
					info_message (str, 70);

				}
				else
				{
					missile_target = un;
					sprintf(str, "TARGET LOCKED - %s(%d)", ship_list[type]->name, univ->energy);
					info_message (str, 70);
				}
			}
			else
			{

				if(type ==SHIP_CARGO){

					if(univ->bravery == -101)
						sprintf(str, "Genesis Capsule(Ecology Seed)");
					else
						sprintf(str, "%s(%s)", ship_list[type]->name, stock_market[univ->bravery].name);
				

				}
				else
					sprintf(str, "%s(%d)", ship_list[type]->name, univ->energy);

				info_message (str, 40);

			}

			if((flip->flags &FLG_BAD)) //!(mcount%2) && 
				cross_color= GFX_COL_RED_1;
		
		}


		if(laser)
		{

			draw_laser= TRUE;
			cmdr.score++;

			if(type ==SHIP_CONSTRICTOR || 
				type ==SHIP_DODEC || type ==SHIP_CORIOLIS)
			{
				if(laser2 == MILITARY_LASER)
					univ->energy -= laser/4;
			}
			else 
			if(type == SHIP_COUGAR)
				univ->energy -= laser/2;
			else
				univ->energy -= laser; 

			if(is_melee && im_good && (univ->flags &FLG_GOOD))
			{
				if(rand255()> 50)
					info_message ("HEY, CHECK YOUR FIRE", 40);
				else
					info_message ("WHAT THE... WHOSE SIDE ARE YOU ON", 40);
			}


			if(type >SHIP_ROCK && univ->energy <max_e/5 && !(univ->distance%7))
			{
				int i, n_alloy= (rand255() &3);
				if(n_alloy >0)
				{
					for (i = 0; i <n_alloy; i++)
						launch_enemy (un, SHIP_ALLOY, FLG_INACTIVE,0);
				}
			}



			if(univ->energy <= 0)
			{

				explode_object (un);
				do_the_scoring(type, univ->flags);
				
				if(type ==SHIP_CONSTRICTOR)
				{
					for (i = 0; i < 15; i++)
						launch_enemy (un, SHIP_CARGO, FLG_INACTIVE,0);

				}
				else
				if(type ==SHIP_COUGAR || (type !=SHIP_THARGOID && convoy_present && max_e >200))
				{
					for (i = 0; i < 10; i++)
						launch_enemy (un, SHIP_CARGO, FLG_INACTIVE,0);

				}
				else
				if(type ==SHIP_ASTEROID)
				{
					if (laser2 == MINING_LASER )
					    launch_loot (un, SHIP_ROCK);
	
					if(cmdr.mission ==-1 || cmdr.mission ==-6) 
						global_bounty+= 50;

				}
				else
					launch_loot (un, SHIP_CARGO); 

				return;
			}
					
			make_angry (un);

		}
	}
}



void activate_ecm (int ours)
{

	if (ecm_active == 0) 
	{
		ecm_active = 32;
		ecm_ours = ours;
	//	snd_play_sample (SND_ECM);
	}

}


void time_ecm (void)
{
	if (ecm_active != 0)
	{
		ecm_active--;
		if (ecm_ours)
			decrease_energy (-1);
	}
}


void arm_missile (void)
{
	razaar_g_x= 128 * GFX_SCALE;
	razaar_g_y= 96 * GFX_SCALE;

	if ((cmdr.missiles != 0) && (missile_target == MISSILE_UNARMED))
		missile_target = MISSILE_ARMED;
}


void unarm_missile (void)
{
	missile_target = MISSILE_UNARMED;
//	snd_play_sample (SND_BOOP);
}

void fire_missile (void)
{
	int newship;
	struct univ_object *ns;
	Matrix rotmat;

	if (missile_target < 0)
		return;
	
	set_init_matrix (rotmat);
	rotmat[2].z = 1.0;
	rotmat[0].x = -1.0;
	
	newship = add_new_ship (SHIP_MISSILE, 0, -30, 25, rotmat, 0, 0);

	if (newship == -1)
	{
		info_message ("MISSILE BAY JAMMED", 40);
		return;
	}

	ns = &universe[newship];
	
	ns->velocity = 15+ flight_speed/2;
	ns->flags = FLG_ANGRY;
	ns->target = missile_target;

	make_angry(missile_target);
	
	cmdr.missiles--;
	missile_target = MISSILE_UNARMED;
	
//	snd_play_sample (SND_MISSILE);
}




void melee_tactics (int un)
{


	Vector vec, nvec;
	struct univ_object *ship;
	struct univ_object *target;
	
	ship = &universe[un];
	target= &universe[ship->target];


	if( ship->target ==un || target->type <2 || 
		((ship->flags &FLG_BAD) && n_attacking_station >5) ||
		ship->target ==last_bad || ship->target ==last_good ) 
	{

		ship->flags &= ~FLG_ANGRY;
		ship->flags &= ~FLG_HOSTILE;

		int rnd_un= 3+rand255()/2;
		target = &universe[rnd_un];

		if( target->type >0 && un !=rnd_un && (((ship->flags &FLG_GOOD) && (target->flags &FLG_BAD)) ||
			((ship->flags &FLG_BAD) && (target->flags &FLG_GOOD))) )			
			ship->target= rnd_un;
		else
		{
			if(ship->flags &FLG_BAD) 
			{

				if(im_good && ship->distance <9000)
				{
					ship->flags |= FLG_ANGRY;
					ship->target= un;
				}
				else
				if(n_attacking_station <5 && 
					(ship->type ==SHIP_THARGOID || ship->type ==SHIP_THARGLET))
				{
					ship->target= 2;
					st_attacker= un;
				}
				else
					ship->target= last_good;
			}
			else
			if(ship->flags &FLG_GOOD)
			{
				if(!im_good && ship->distance <9000) 
				{
					ship->flags |= FLG_ANGRY;
					ship->target= un;
				}
				else
				if(st_attacker >0)
					ship->target= st_attacker;
				else
					ship->target= last_bad;

			}





		}		


		target= &universe[ship->target];
	}



//*** ----------------------
	vec.x = target->location.x - ship->location.x;
	vec.y = target->location.y - ship->location.y;
	vec.z = target->location.z - ship->location.z;


	nvec = unit_vector(&vec);
	float dir= -vector_dot_product (&nvec, &ship->rotmat[2]);
	int distance= (int)sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);



	if(dir > -0.7 && dir < 0.2 && ship->velocity >25)
		ship->acceleration = -1;
	else
		ship->acceleration = 2;

	ship->rotx= 0;
	if(dir >-0.4 && distance <3000 && nvec.z >0 && rand255() <150)
		ship->acceleration= 3;
	else
	if(dir <0 && distance >1500 && ship->distance >2000) 
	{
		float fbs= fabs(vector_dot_product (&nvec, &ship->rotmat[1]));
		if(fbs >0.25)
			ship->rotx= (vector_dot_product(&nvec, &ship->rotmat[1]) >0) ? -1 : 1;
	}
	else
		ship->rotx= (ship->rotx >0) ? -1 : 1;

	
	if(distance <2000 || ship->distance <1000)
		ship->rotz= (!((distance/100)%2)) ? -1 : 1;
	else
		ship->rotz= (vector_dot_product(&nvec, &ship->rotmat[0]) >0) ? -1 : 1;



	//laser
	if(dir < -0.85 && distance <10000)
	{

		ship->acceleration = 1;
		draw_laser_3d(un, ship->target);

		if(dir < -0.95)
		{
			if(distance <5000)
				target->energy-= ship_list[ship->type]->laser_strength;
			else
				target->energy-= ship_list[ship->type]->laser_strength/3;
			
		}

	}

}





void missile_tactics (int un)
{
	struct univ_object *missile;
	struct univ_object *target;
	Vector vec;
	Vector nvec;
	float dir;
	
	missile = &universe[un];


	if(ecm_active)
	{
//		snd_play_sample (SND_EXPLODE);
		missile->flags |= FLG_DEAD;		
		return;
	}

	if(missile->target == 0)
	{
		if (missile->distance < 450)
		{
			missile->flags |= FLG_DEAD;
//			snd_play_sample (SND_EXPLODE);
			damage_ship (250, missile->location.z >= 0.0);
			return;
		}

		vec.x = missile->location.x;
		vec.y = missile->location.y;
		vec.z = missile->location.z;


	}
	else
	{
		target = &universe[missile->target];

		vec.x = missile->location.x - target->location.x;
		vec.y = missile->location.y - target->location.y; 
		vec.z = missile->location.z - target->location.z;
	

		if(target->distance> 500 && (fabs(vec.x) <300 +target->distance/20) && 
			(fabs(vec.y) <300 +target->distance/20) && (fabs(vec.z) <700 +target->distance/10)) //easy missile hit
		{

			if(target->type ==SHIP_ASTEROID && 
				(cmdr.mission ==-1 || cmdr.mission ==-6)) 
				global_bounty+= 50;

			do_the_scoring(target->type, target->flags);
			universe[missile->target].energy-= 300;

			missile->flags |= FLG_DEAD;		
//			snd_play_sample (SND_EXPLODE);

			return;
		}

		if ((rand255() < 16) && (target->flags & FLG_HAS_ECM))
		{
			activate_ecm (0);
			return;
		}
	}	

	nvec = unit_vector(&vec);
	dir = vector_dot_product (&nvec, &missile->rotmat[2]); 
	if(dir > -0.4 && missile->velocity>15)
		missile->acceleration = -5;
	else
		missile->acceleration = 4;

	missile->rotx= 0;
	if(dir <0)
	{
		float fbs= fabs(vector_dot_product (&nvec, &missile->rotmat[1]));
		if(fbs >0.25)
			missile->rotx= (vector_dot_product(&nvec, &missile->rotmat[1]) <0) ? -1 : 1;
	}
	else
		missile->rotx= 1;

	missile->rotz= (vector_dot_product(&nvec, &missile->rotmat[0]) <0) ? -1 : 1;
}





void launch_shuttle (void)
{

	if(police_is_angry || universe[2].distance <5000 || ship_count[SHIP_TRANSPORTER] || 
			rand255() >(5+current_planet_data.government) || (auto_pilot) )
		return;

	int type;

	if(rand255() >50)
	{
		if(rand255() <100) 
			type= SHIP_TRANSPORTER;
		else
			type= SHIP_SHUTTLE;

		launch_enemy (2, type, FLG_GOOD | FLG_HAS_ECM | FLG_FLY_TO_PLANET, 115);
	}
	else
	{
		type = SHIP_SIDEWINDER + (rand255() &7);
		launch_enemy (2, type, FLG_GOOD | FLG_HAS_ECM | FLG_FLY_TO_PLANET, 115);
	}

}


void tactics (int un)
{

	int type;
	int energy;
	int maxeng;
	int flags;
	struct univ_object *ship;
	
	ship = &universe[un];
	type = ship->type;
	flags = ship->flags;

	if((type == SHIP_PLANET) || (type == SHIP_SUN))
		return;
	
	if(flags & FLG_DEAD)
		return;

	if((flags &FLG_INACTIVE) || (cmdr.mission <-1000 && !is_melee))
		return;


	if(type == SHIP_MISSILE)
	{
		if (flags & FLG_ANGRY)
			missile_tactics (un);
		return;
	}



	if((type == SHIP_CORIOLIS) || (type == SHIP_DODEC))
	{
		if(cmdr.mission== -1 || cmdr.mission== -6 ) 
			return; //asteroid missions, last mission 
		else
		if( gal_fam !=-500 && docked_planet.d == 150 && 
			docked_planet.b == 130 && cmdr.galaxy_number == 2) 
			return; //starving zaisedan
		else
 		if( rand255() <20 && ship_count[SHIP_VIPER] <5 && 
			((flags &FLG_ANGRY) || police_is_angry || 
			(is_melee && (rand255() <((n_bad+current_planet_data.government)*10)))) ) 
			launch_enemy (un, SHIP_VIPER, FLG_GOOD, 115);
		else
		if(!is_melee)
			launch_shuttle ();
		
		return;
	}



//who's who enemy? 
	if(!(flags & FLG_ANGRY))
	{
		if(is_melee) //melee
			melee_tactics(un);
		else
		if((flags &FLG_BAD) || (traders_are_angry && (flags &FLG_GOOD)) || 
			(type ==SHIP_VIPER && police_is_angry) ) 
			make_angry(un);
		else
		if((flags & FLG_FLY_TO_PLANET) || (flags & FLG_FLY_TO_STATION))
		{
			if(ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
			{
				if((flags & FLG_FLY_TO_STATION) && 
					ship->distance <1000 && universe[2].distance <3000) 
				{
					ship->flags &= ~FLG_FLY_TO_STATION;
					ship->flags |= FLG_FLY_TO_PLANET;

				}
				else
				if((flags & FLG_FLY_TO_PLANET) && ship_list[type]->energy <170 && 
					(myship.altitude< 100 || ship->distance >35000))
				{
					ship->flags &= ~FLG_FLY_TO_PLANET;
					ship->flags |= FLG_FLY_TO_STATION;
				}

			}

			auto_pilot_ship (&universe[un]);		
		}
		else
		if(flags &FLG_GOOD)
		{
			if(ship->energy <ship_list[type]->energy)
			{
				if(ship_list[type]->energy <170 && 
					(ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])) 
					ship->flags |= FLG_FLY_TO_STATION; 
				else
					ship->flags |= FLG_FLY_TO_PLANET; 
			}
			else
			{
				ship->rotx= ship->rotz= 0;
				if(!convoy_present)
					ship->velocity= ship_list[type]->velocity;
			}
		}


		return;

	}
	else //if angry
	if(is_melee && n_angry_at_me >3)
	{
		ship->flags &= ~FLG_ANGRY;
		ship->flags &= ~FLG_HOSTILE;
		return;
	}


	if(station_is_exploding)
		return;

//*** ------------------------------------------------------------------
//*** still angry? - then go crazy
	if(type ==SHIP_HERMIT && ship->missiles >0 && rand255() <10)
	{
		ship->missiles--;
		launch_enemy (un, SHIP_SIDEWINDER + (rand255() &3), flags , 115);
		return;
	}
	else
	if(type ==SHIP_ANACONDA && ship->missiles >0 && rand255() <5)
	{

		ship->missiles--;
		launch_enemy(un, rand255() <100 ? 
						SHIP_WORM : SHIP_SIDEWINDER, flags, 115);
		return;
	}
	else
	if(type ==SHIP_THARGOID && ship->missiles >0 && rand255() <5)
	{
		ship->missiles--;
		launch_enemy (un, SHIP_THARGLET, FLG_BAD | FLG_ANGRY, 0);  
	}
	else
	if(type ==SHIP_THARGLET && ship_count[SHIP_THARGOID] ==0)
	{

		ship->rotx= 0;
		ship->rotz= 0;
		ship->flags &= ~FLG_ANGRY & ~FLG_CLOAKED & ~FLG_HOSTILE;

		if(ship->velocity >0)
			ship->acceleration= -1;
		else
			ship->flags |= FLG_INACTIVE;

		return;
	}



//**** low energy? -fire missile/esc pod
	
	maxeng = ship_list[type]->energy;
	energy = ship->energy;

	if(!is_melee && energy < (maxeng/2)) 
	{

	//*** run & dock with station if not too big and if good trader
		if((flags &FLG_GOOD) && ship_list[type]->energy <170 &&
			(ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC]) )
		{
			ship->flags &= ~FLG_ANGRY;
			ship->flags &= ~FLG_HOSTILE;
			ship->flags |= FLG_FLY_TO_STATION; 
		}


	//*** missile
		if(ship->missiles >0 && !ecm_active && 
			type !=SHIP_THARGOID && rand255() <10 && ship_count[SHIP_MISSILE] <5)
		{

			ship->missiles--;
			launch_enemy (un, SHIP_MISSILE, FLG_ANGRY, 126);

			if(ship->type == SHIP_CONSTRICTOR || ship->type == SHIP_COUGAR)
			{
				if(rand255() <50) 			
					info_message ("RIDE MY ROCKET, HERMIT-FACE", 100);
				else
				if(rand255() <50 || ship->missiles ==0) 			
					info_message ("DODGE THIS, TRUMBLE-BRAIN", 100);
				else
				if(rand255() <50) 			
					info_message ("THIS ONE HAS YOUR NAME ON IT", 100);
				else
	 				info_message ("INCOMING MISSILE", 40);

			}
			else
				info_message ("INCOMING MISSILE", 40);


			return;
		}


	//*** esc pod
		if(energy <(maxeng/10) && rand255() <25 && type !=SHIP_THARGOID)
		{

			ship->rotx= 0;
			ship->rotz= 0;
			ship->velocity= ship_list[type]->velocity;
			ship->flags &= ~FLG_ANGRY & ~FLG_CLOAKED & ~FLG_HOSTILE;
			ship->flags |= FLG_INACTIVE;

			
			if(ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
				launch_enemy (un, SHIP_ESCAPE_CAPSULE, FLG_FLY_TO_STATION, 0);
			else
				launch_enemy (un, SHIP_ESCAPE_CAPSULE, FLG_FLY_TO_PLANET, 0);
	

			if(cmdr.mission== -3 && ship_count[SHIP_COUGAR] == 1) {

				message_count= 0;
				info_message("Zurid Pino launched Escape Pod, capture him Commander.", 100);
				zpino= 1;
			}
			else
			if(cmdr.mission== 1 && ship_count[SHIP_CONSTRICTOR] == 1) {

				message_count= 0;
				info_message("Pirates escaping. Capture the crew for Bonus reward.", 100);
				cougar_pod= 1;
			}


			return;				
		}


	}//end (low energy?)






//*** combat manuvering tactics
	if(energy > maxeng/10 && (flags &FLG_ANGRY) && ship_list[type]->laser_strength >0)
	{

		Vector nvec = unit_vector(&universe[un].location);
		float dir= vector_dot_product (&nvec, &ship->rotmat[2]);


		if(dir > -0.7 && dir < 0.2 && ship->velocity >25)
			ship->acceleration = -1;
		else
			ship->acceleration = 2;


		ship->rotx= 0;
		if(dir >-0.4 && ship->distance <3000 && nvec.z >0 && rand255() <150)
			ship->acceleration= 3;
		else
		if(dir <0 && ship->distance >1500) 
		{
			float fbs= fabs(vector_dot_product (&nvec, &ship->rotmat[1]));
			if(fbs >0.25)
				ship->rotx= (vector_dot_product(&nvec, &ship->rotmat[1]) <0) ? -1 : 1;
		}
		else
			ship->rotx= (ship->rotx <0) ? -1 : 1;
		
		if(ship->distance <2000)
			ship->rotz= (!((ship->distance/100)%2)) ? -1 : 1;
		else
			ship->rotz= (vector_dot_product(&nvec, &ship->rotmat[0]) <0) ? -1 : 1;


		if(dir < -0.85 && ship->distance <10000)
		{
			ship->flags |= FLG_FIRING | FLG_HOSTILE;		
			ship->acceleration = 1;

			if(dir < -0.95)
			{
				if(ship->distance <5000)
					damage_ship (ship_list[type]->laser_strength, ship->location.z >= 0.0);
				else
					damage_ship (ship_list[type]->laser_strength/3, ship->location.z >= 0.0);

			}

		}

	}


}




static int new_laser;

void draw_laser_lines (void)
{
	int col = 0;

	if(laser2 == PULSE_LASER) col= GFX_COL_GREEN_1;
	if(laser2 == BEAM_LASER) col= GFX_COL_RED_1;
	if(laser2 == MILITARY_LASER) col= GFX_COL_PINK_1;
	if(laser2 == MINING_LASER ) col= GFX_COL_BLUE_4;


	gfx_draw_colour_line (32 * GFX_SCALE, GFX_VIEW_BY, laser_x, laser_y, col);
	gfx_draw_colour_line (48 * GFX_SCALE, GFX_VIEW_BY, laser_x, laser_y, col);
	gfx_draw_colour_line (208 * GFX_SCALE, GFX_VIEW_BY, laser_x, laser_y, col);
	gfx_draw_colour_line (224 * GFX_SCALE, GFX_VIEW_BY, laser_x, laser_y, col);

	if(laser2 == MILITARY_LASER) 
	{
		col= GFX_COL_RED_1;

		gfx_draw_colour_line (32 * GFX_SCALE-5, GFX_VIEW_BY, laser_x-1, laser_y, col);
		gfx_draw_colour_line (48 * GFX_SCALE+5, GFX_VIEW_BY, laser_x+1, laser_y, col);
		gfx_draw_colour_line (208 * GFX_SCALE-5, GFX_VIEW_BY, laser_x-1, laser_y, col);
		gfx_draw_colour_line (224 * GFX_SCALE+5, GFX_VIEW_BY, laser_x+1, laser_y, col);	
	}

}


int fire_laser (void)
{
	if ((laser_counter == 0) && (laser_temp < 242))
	{
		switch (current_screen)
		{
			case SCR_FRONT_VIEW:
				laser = cmdr.front_laser;
				break;
			
			case SCR_REAR_VIEW:
				laser = cmdr.rear_laser;
				break;
					
			case SCR_RIGHT_VIEW:
				laser = cmdr.right_laser;
				break;
					
			case SCR_LEFT_VIEW:
				laser = cmdr.left_laser;
				break;
				
			default:
				laser = 0;
		}

		if (laser != 0)
		{
			laser_counter = (laser > 127) ? 0 : (laser & 0xFA);
			laser2= laser;
			laser &= 127;
			
			laser_temp += 8;
			if (energy > 1)
				energy--;
			
			laser_x = ((rand() & 3) + 128 - 2) * GFX_SCALE;
			laser_y = ((rand() & 3) + 96 - 2) * GFX_SCALE;
			
			return 2;
		}
	}

	return 0;
}


void cool_laser (void)
{
	laser = 0;

	if (laser_temp > 0)
		laser_temp--;
					
	if (laser_counter > 0)
		laser_counter--;
				
	if (laser_counter > 0)
		laser_counter--;
}


int create_other_ship (int type)
{
	Matrix rotmat;
	int x,y,z;
	int newship;
	
	set_init_matrix (rotmat);

	z = 12000;
	x = 1000 + (randint() & 8191);
	y = 1000 + (randint() & 8191);

	if (rand255() > 127)
		x = -x;
	if (rand255() > 127)
		y = -y;

	if( tharg_wave >1 && rand255() >100 && 
		(type ==SHIP_THARGOID || type ==SHIP_THARGLET) &&
		(ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC]) )
	{
		z*= -1;
	}

	newship = add_new_ship (type, x, y, z, rotmat, 0, 0);

	return newship;
}


void create_thargoid (void)
{
	int newship;
	
	newship = create_other_ship (SHIP_THARGOID);
	if (newship != -1)
		universe[newship].flags = FLG_ANGRY | FLG_HAS_ECM | FLG_BAD;  	 

	if(rand255() <50)
		launch_enemy (newship, SHIP_THARGLET, FLG_BAD | FLG_ANGRY, 0);  
}


void create_lone_hunter (void)
{
	int rnd;
	int type;
	int newship;


	rnd = rand255();
	type = SHIP_COBRA3_LONE + (rnd & 3) + (rnd > 127);
		
	newship = create_other_ship (type);
	if (newship != -1)
	{
		in_battle++;  
		universe[newship].flags = FLG_ANGRY | FLG_BAD;  	 
		if(rand255() <100)
		{
			universe[newship].flags |= FLG_HAS_ECM;
			universe[newship].energy+= 50;
		}
		else
			universe[newship].energy+= 100;
	}	

}



void create_pirates (void)
{
	int x,y,z;
	int newship;
	int rnd;
	int type;
	int i;

	
	if(rand255() < 100 && n_good <3)
	{
		if(rand255() <50)
			create_lone_hunter();

		create_lone_hunter();
		return;
	}	


//*** Pack hunters... 
	
	Matrix rotmat;
	set_init_matrix (rotmat);

	z = 12000;

	if(rand255() <25) 			
	{

		info_message ("REPENT SINNER, REPENT... OR DIE", 200);
		rnd = 3 +(rand255() & 3) + n_good;
		for (i = 0; i < rnd; i++)
		{
			x = 1000 + (randint() & 8191);
			y = 1000 + (randint() & 8191);

			if (rand255() > 127)
				x = -x;
			if (rand255() > 127)
				y = -y;

			newship = add_new_ship (SHIP_WORM, x, y, z, rotmat, 0, 0);
			if (newship != -1)
			{
				in_battle++;  
				universe[newship].flags = FLG_ANGRY | FLG_BAD; 	 
				if(rand255() <100) 
					universe[newship].energy+= 50;
			}
		}


	}
	else
	{
		rnd = 1 +(rand255() & 3) + n_good;

		for (i = 0; i < rnd; i++)
		{
			x = 1000 + (randint() & 8191);
			y = 1000 + (randint() & 8191);

			if (rand255() > 127)
				x = -x;
			if (rand255() > 127)
				y = -y;

			type = SHIP_SIDEWINDER + (rand255() &7);
			if(type ==SHIP_WORM)
				type= SHIP_MORAY;

			newship = add_new_ship (type, x, y, z, rotmat, 0, 0);
			if (newship != -1)
			{
				in_battle++;  
				universe[newship].flags = FLG_ANGRY | FLG_BAD; 	 
				if(rand255() <100)
					universe[newship].flags |= FLG_HAS_ECM;
				else
					universe[newship].energy+= 50;
			}
		}

	}
	 	
}

	

void create_trader (void)
{

	if(rand255() <25 && cmdr.mission != -6)
	{
		create_traders_convoy();
		return;
	}	


	int x, y, z, newship, type; 

	Matrix rotmat;
	set_init_matrix (rotmat);

	z = 12000;
	x = 1000 + (randint() & 8191);
	y = 1000 + (randint() & 8191);

	if(rand255() > 127) x = -x;
	if(rand255() > 127) y = -y;


	if( cmdr.mission == -6 || 
		(rand255() <100 && (ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])) ) 
		type = SHIP_SIDEWINDER + (rand255() &7);
	else
		type = SHIP_COBRA3 + (rand255() &3);

	newship = add_new_ship (type, x, y, z, rotmat, 0, 0);
	if (newship != -1)
	{
		universe[newship].rotmat[2].z = -1.0;
		universe[newship].rotz = rand255()/30;
		universe[newship].velocity = ship_list[type]->velocity;
		universe[newship].flags = FLG_GOOD;  
		if(rand255()>100) 
			universe[newship].flags |= FLG_HAS_ECM;
		else
		if(type >= SHIP_SIDEWINDER)
			universe[newship].flags |= FLG_FLY_TO_STATION;

		if(rand255()>200) universe[newship].energy+=50;
	}

}







/* Check for a random asteroid encounter... */

void check_for_asteroids (void)
{
	int i, type, newship;

	int rnd= (rand255() &7);

	if(rand255() <100) 
		rnd= 15 + (rand255() &31);

	for(i=0; i<rnd; i++)
	{
		if (rand255() > 250)
			type = SHIP_HERMIT;
		else
			type = SHIP_ASTEROID;
			
		newship = create_other_ship (type);
		if (newship != -1)
		{
			universe[newship].velocity = 4 + (rand255() &7) ;
			universe[newship].rotz = rand255() > 127 ? -1 : 1; 
		}
	}

	if(rand255() <(rnd*3))
		create_pirates();
}







void random_encounter (void) // every 255 mcounts
{


	if(ship_count[SHIP_COUGAR] || ship_count[SHIP_CONSTRICTOR] 
		|| cmdr.mission== -1 || cmdr.mission== -6 || cmdr.mission ==6) 
		return;
	else
	if(razaar_refugees >0 && 
		docked_planet.d ==112 && docked_planet.b ==95 && cmdr.galaxy_number ==0) 
		return;
	else
	if(gal_fam !=-500 && 
		docked_planet.d ==150 && docked_planet.b ==130 && cmdr.galaxy_number ==2) 
		return; //starving zaisedan


	if(!ship_count[SHIP_COUGAR] && condition ==1 &&
		cmdr.mission== -3 && cmdr.galaxy_number ==0 &&
		(docked_planet.d==102) && (docked_planet.b==57))
	{

		int new_ship= create_other_ship(SHIP_COUGAR);
		if(new_ship >0)
		{
			universe[new_ship].flags = FLG_ANGRY | FLG_HAS_ECM | FLG_CLOAKED | FLG_BAD;  
			zpino= 0;
		}
					
		return;
	}	
	else
	if(!ship_count[SHIP_CONSTRICTOR] && cmdr.mission == 1 && cmdr.galaxy_number == 1 &&
		(docked_planet.d == 144) && (docked_planet.b == 33) && myship.cabtemp > 200)
	{		

		int new_ship= create_other_ship(SHIP_CONSTRICTOR);
		if(new_ship >0)
		{
			universe[new_ship].flags = FLG_ANGRY | FLG_HAS_ECM | FLG_BAD;   
			cougar_pod= 0;
		}
		return;
	}


	if(myship.cabtemp >50) return;
//******** ---

	if(condition ==1 && rand255() <(25 + current_planet_data.government*3))
		create_trader();
	else
	if( !ship_count[SHIP_VIPER] && condition !=1 &&
			(rand255() <(cmdr.legal_status/10 + current_planet_data.government*3)) )
		create_viper_patrol();
	else
	if(ship_count[SHIP_VIPER] || 
			ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
	{
		if(cmdr.legal_status >50 && rand255() <(cmdr.legal_status/5))
			police_is_angry= TRUE;	
	}
	else
	if(n_bad <2 && rand255() < (35 - current_planet_data.government*4)) //gvmnt 0-7 (15 -rare encounter
		create_pirates();	
	else
	if(condition ==1 && rand255() < (25 -current_planet_data.government*3)) //gvmnt 0-7
		create_melee();
	else
	if(condition ==1 && rand255() <10) //10 -rare ?
		check_for_asteroids();
	else
	if(n_bad <1 && cmdr.mission >=5 && rand255() <25) //30 average , 50 frequent
	{
		int i, cnt= (rand255() &3); 
		for(i = 0; i <cnt; i++)
			create_thargoid();	
	}

}



/*
 * Dock the player into the space station.
 */

void dock_player (void)
{

	if( razaar_refugees >100 && (docked_planet.d == 112) && 
			(docked_planet.b == 95) && (cmdr.galaxy_number == 0) ) return;


	disengage_auto_pilot();
	docked = TRUE;
	flight_speed = 0;
	flight_roll = 0;
	flight_climb = 0;
	front_shield = 255;
	aft_shield = 255;
	energy = 255;
	myship.altitude = 255;
	myship.cabtemp = 30;
	reset_weapons();
	current_screen = SCR_BREAK_PATTERN;


	if(cmdr.credits < 0)
	{
		cmdr.credits= 0;
		current_screen=	SCR_MISSION_4;
	}

	if(cmdr.mission ==-1) cmdr.mission= -101;
	if(cmdr.mission ==-6) cmdr.mission= -106;
	if(cmdr.mission ==6) current_screen = SCR_LAST_MISSION;
	if(cmdr.mission == -1002) cmdr.mission= -1001;

	if(cmdr.legal_status >carrying_contraband())
		cmdr.legal_status--;
}



void abandon_ship (void)
{
	int i;

	cmdr.escape_pod = 0;
	cmdr.escape_pod= 1; 
	cmdr.legal_status = 0;
	cmdr.fuel = myship.max_fuel;
	
	for (i = 0; i < NO_OF_STOCK_ITEMS; i++)
		cmdr.current_cargo[i] = 0;
	
//	snd_play_sample (SND_DOCK);					
	dock_player();



}


//********** formations
void create_viper_patrol(void)
{

	int vel= 15+ rand255()/30;
	
	int x, y, z, newship; 

	Matrix rotmat;
	set_init_matrix (rotmat);

	z = 12000;
	x = 1000 + (randint() & 8191);
	y = 1000 + (randint() & 8191);

	if (rand255() > 127)
		x = -x;
	if (rand255() > 127)
		y = -y;

	newship = add_new_ship (SHIP_VIPER, x, y, z-500, rotmat, 0, 0);
	if (newship != -1)
	{
		universe[newship].flags = FLG_GOOD;  
		universe[newship].velocity= vel;
		if(rand255()>100) universe[newship].energy+=50;
		if(rand255()>50) universe[newship].flags |= FLG_HAS_ECM;
	}

//****
	newship = add_new_ship (SHIP_VIPER, x+200, y-100, z-200, rotmat, 0, 0);
	if (newship != -1)
	{
		universe[newship].flags = FLG_GOOD;  
		universe[newship].velocity= vel;
		if(rand255()>100) universe[newship].energy+=50;
		if(rand255()>50) universe[newship].flags |= FLG_HAS_ECM;
	}


	newship = add_new_ship (SHIP_VIPER, x-200, y-100, z-200, rotmat, 0, 0);
	if (newship != -1)
	{
			universe[newship].flags = FLG_GOOD; 
			universe[newship].velocity= vel;
			if(rand255()>100) universe[newship].energy+=50;
			if(rand255()>50) universe[newship].flags |= FLG_HAS_ECM;
	}


	if(rand255()>100)
	{		

		newship = add_new_ship (SHIP_VIPER, x+400, y, z, rotmat, 0, 0);
		if (newship != -1)
		{
			universe[newship].flags = FLG_GOOD;  
			universe[newship].velocity= vel;
			if(rand255()>100) universe[newship].energy+=50;
			if(rand255()>50) universe[newship].flags |= FLG_HAS_ECM;
		}


		newship = add_new_ship (SHIP_VIPER, x-400, y, z, rotmat, 0, 0);
		if (newship != -1)
		{
			universe[newship].flags = FLG_GOOD;   
			universe[newship].velocity= vel;
			if(rand255()>100) universe[newship].energy+=50;
			if(rand255()>50) universe[newship].flags |= FLG_HAS_ECM;
		}

	}
//*****

	if(rand255()>150)
	{		

		newship = add_new_ship (SHIP_VIPER, x+100, y+100, z+200, rotmat, 0, 0);
		if (newship != -1)
		{
			universe[newship].flags = FLG_GOOD;   
			universe[newship].velocity= vel;
			if(rand255()>100) universe[newship].energy+=50;
			if(rand255()>50) universe[newship].flags |= FLG_HAS_ECM;
		}


		newship = add_new_ship (SHIP_VIPER, x-100, y+100, z+200, rotmat, 0, 0);
		if (newship != -1)
		{
			universe[newship].flags = FLG_GOOD;   
			universe[newship].velocity= vel;
			if(rand255()>100) universe[newship].energy+=50;
			if(rand255()>50) universe[newship].flags |= FLG_HAS_ECM;
		}

	}
}


void create_traders_convoy(void)
{

	convoy_present= TRUE;
	
	int i, cnt, x, y, z, newship, type; 

	int vel= 10+ rand255()/30;

	Matrix rotmat;
	set_init_matrix (rotmat);

	z = 12000;
	x = 1000 + (randint() & 8191);
	y = 1000 + (randint() & 8191);

	if (rand255() > 127)
		x = -x;
	if (rand255() > 127)
		y = -y;


//*** big ships in the middle
	cnt= 2+(rand255() & 3);
	for(i=0; i<cnt;i++)
	{
		type = SHIP_PYTHON + (rand255() &3);
		if(type == SHIP_HERMIT) type= SHIP_PYTHON_LONE;

		newship = add_new_ship (type, x-rand255()+128, y-rand255()+128, z+i*2500+rand255()*2, rotmat, 0, 0);
		if (newship != -1)
		{
			universe[newship].velocity = vel;
			universe[newship].flags = FLG_GOOD;  
			if(rand255()>150) universe[newship].flags |= FLG_HAS_ECM;
			if(rand255()>200) universe[newship].energy+=50;
		}
	
	}


//*** fighters escort on sides
	cnt= 1+(rand255() & 3);
	for(i=0; i<cnt; i++)
	{
		type = SHIP_SIDEWINDER + (rand255() &7);
		newship = add_new_ship (type, x+rand255()*2 +700, y+rand255()*2-255, z+i*2500+rand255()*5, rotmat, 0, 0);
		if (newship != -1)
		{
			universe[newship].velocity = vel;
			universe[newship].flags = FLG_GOOD;   
			if(rand255()>50) 
				universe[newship].flags |= FLG_HAS_ECM;
			else
			if(ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
				universe[newship].flags |= FLG_FLY_TO_STATION;

			if(rand255()>200) universe[newship].energy+=50;
		}
	}

	cnt= 1+(rand255() & 3);
	for(i=0; i<cnt; i++)
	{
		type = SHIP_SIDEWINDER + (rand255() &7);
		newship = add_new_ship (type, x-rand255()*2-700, y+rand255()*2-255, z+i*2500+rand255()*5, rotmat, 0, 0);
		if (newship != -1)
		{
			universe[newship].velocity = vel;
			universe[newship].flags = FLG_GOOD;   
			if(rand255()>50) 
				universe[newship].flags |= FLG_HAS_ECM;
			else
			if(ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
				universe[newship].flags |= FLG_FLY_TO_STATION;

			if(rand255()>200) universe[newship].energy+=50;

		}
	}


}


void create_melee (void)
{
	int XX, YY, x,y,z;
	int newship;
	int i, type, rnd;

	
	Matrix rotmat;
	set_init_matrix (rotmat);

	z= 30000;
	XX= rand255()*50;
	YY= rand255()*50;


	//bad pirates
	rnd= 15 - current_planet_data.government + rand255()/3;
	for(i = 0; i <rnd; i++)
	{
		x = XX +(rand255()-127)*20;
		y = YY +(rand255()-127)*20;

		type = SHIP_SIDEWINDER + (rand255() &7);
		newship = add_new_ship (type, x, y, z, rotmat, 0, 0);
		if (newship != -1)
		{
			universe[newship].flags = FLG_ANGRY | FLG_BAD; 	 
			if(rand255() >200)universe[newship].flags |= FLG_HAS_ECM;
			if(rand255()>200) universe[newship].energy+= 50;
		}
	}


//trade convoy(good pirates) & police
	rnd= 10 + current_planet_data.government + rand255()/10;
	if(rand255() <100)
	{

		for(i=0; i <rnd; i++)
		{

			x = (rand255()-127)*30;
			y = (rand255()-127)*30;
			type = SHIP_SIDEWINDER + (rand255() &7);
			newship = add_new_ship (type, x, y, z, rotmat, 0, 0);
			if (newship != -1)
			{
				universe[newship].flags = FLG_GOOD;   
				if(rand255()>100) 
					universe[newship].flags |= FLG_HAS_ECM;
				else
				if(ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
					universe[newship].flags |= FLG_FLY_TO_STATION;

				if(rand255()>200) universe[newship].energy+=50;
			}


			x = (rand255()-127)*30;
			y = (rand255()-127)*30;
			newship = add_new_ship (SHIP_VIPER, x, y, z, rotmat, 0, 0);
			if (newship != -1)
			{
				universe[newship].flags = FLG_GOOD;   
				if(rand255()>100) universe[newship].energy+=50;
				if(rand255()>50) universe[newship].flags |= FLG_HAS_ECM;
			}

		}

	}
	else
	{

	//just police against pirates
		for(i=0; i <rnd*2; i++)
		{
			x = (rand255()-127)*30;
			y = (rand255()-127)*30;
			newship = add_new_ship (SHIP_VIPER, x, y, z, rotmat, 0, 0);
			if (newship != -1)
			{
				universe[newship].flags = FLG_GOOD;   
				if(rand255()>100) universe[newship].energy+=50;
				if(rand255()>50) universe[newship].flags |= FLG_HAS_ECM;
			}

		}

	}


}
