

// - - - - - - - - - - -  F I L E   E N D - - - - - - - - - - - 


/*
 * Fly to a given point in space.
 */

void fly_to_vector (struct univ_object *ship, Vector vec)
{

	Vector nvec = unit_vector(&vec);
	float fbs, dir= -vector_dot_product (&nvec, &ship->rotmat[2]);
	int distance= (int)sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);

	ship->rotx= 0;
	ship->rotz= 0;
	if(distance< 2000 && dir >0.4 && vec.z >0)
	{ 
		ship->acceleration= 1;
		return;
	}
	else
	if(dir < 0)
	{ 
		fbs= fabs(vector_dot_product (&nvec, &ship->rotmat[1]));
		if(fbs >0.25)
			ship->rotx= (vector_dot_product(&nvec, &ship->rotmat[1]) >0) ? -1 : 1;
	}
	else
		ship->rotx= 1;


	fbs= fabs(vector_dot_product (&nvec, &ship->rotmat[0]));
	if(fbs >0.15 )
		ship->rotz= (vector_dot_product(&nvec, &ship->rotmat[0]) >0) ? -1 : 1;
}



/*
 * Fly towards the planet.
 */

void fly_to_planet (struct univ_object *ship)
{
	Vector vec;

	vec.x = universe[1].location.x - ship->location.x;
	vec.y = universe[1].location.y - ship->location.y;
	vec.z = universe[1].location.z - ship->location.z;

	fly_to_vector (ship, vec);	
}


/*
 * Fly to a point in front of the station docking bay.
 * Done prior to the final stage of docking.
 */


void fly_to_station_front (struct univ_object *ship)
{
	Vector vec;

	vec.x = universe[2].location.x - ship->location.x;
	vec.y = universe[2].location.y - ship->location.y;
	vec.z = universe[2].location.z - ship->location.z;

	vec.x += universe[2].rotmat[2].x * 768;
	vec.y += universe[2].rotmat[2].y * 768;
	vec.z += universe[2].rotmat[2].z * 768;

	fly_to_vector (ship, vec);	
}


/*
 * Fly towards the space station.
 */

void fly_to_station (struct univ_object *ship)
{
	Vector vec;

	vec.x = universe[2].location.x - ship->location.x;
	vec.y = universe[2].location.y - ship->location.y;
	vec.z = universe[2].location.z - ship->location.z;

	fly_to_vector (ship, vec);	
}


/*
 * Final stage of docking.
 * Fly into the docking bay.
 */
 
void fly_to_docking_bay (struct univ_object *ship)
{
	Vector diff;
	Vector vec;
	float dir;

	diff.x = ship->location.x - universe[2].location.x;
	diff.y = ship->location.y - universe[2].location.y;
	diff.z = ship->location.z - universe[2].location.z;

	vec = unit_vector (&diff);	

	ship->rotx = 0;

	if (ship->type < 0)
	{
		ship->rotz = 1;
		if (((vec.x >= 0) && (vec.y >= 0)) ||
			 ((vec.x < 0) && (vec.y < 0)))
		{
			ship->rotz = -ship->rotz;
		}

		if (fabs(vec.x) >= 0.0625)
		{
			ship->acceleration = 0;
			ship->velocity = 1;
			return;
		}

		if(fabs(vec.y) > 0.002436)
			ship->rotx = (vec.y < 0) ? -1 : 1;

		if (fabs(vec.y) >= 0.0625)
		{
			 ship->acceleration = 0;
			 ship->velocity = 1;
			 return;
		}
	}

	ship->rotz = 0;

	dir = vector_dot_product (&ship->rotmat[0], &universe[2].rotmat[1]);

	if (fabs(dir) >= 0.9166)
	{
		ship->acceleration++;
		ship->rotz = 1;
		return;
	}

	ship->acceleration = 0;
	ship->rotz = 0;
}


/*
 * Fly a ship to the planet or to the space station and dock it.
 */

void auto_pilot_ship (struct univ_object *ship)
{

	Vector diff;
	Vector vec;
	float dist;
	float dir;
	


	diff.x = ship->location.x - universe[2].location.x;	
	diff.y = ship->location.y - universe[2].location.y;	
	diff.z = ship->location.z - universe[2].location.z;	

	dist = sqrt (diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);


	if(ship->type >=SHIP_MISSILE &&	ship->velocity >ship_list[ship->type]->velocity) 
		ship->velocity= ship_list[ship->type]->velocity;

	if ((ship->flags & FLG_FLY_TO_PLANET) ||
		((ship_count[SHIP_CORIOLIS] == 0) && (ship_count[SHIP_DODEC] == 0)))
	{
		fly_to_planet (ship);
		return;
	}


	if(dist < 200)
	{
		ship->flags |= FLG_REMOVE | FLG_DOCKED;		// Ship has docked.
		return;
	}	

	if(dist <5000 && ship->velocity >5 && ship->velocity >(dist/100)) 
		ship->velocity--;
	
	vec = unit_vector (&diff);	
	dir = vector_dot_product (&universe[2].rotmat[2], &vec);

	if (dir < 0.95)
	{
		fly_to_station_front (ship);
		return;
	}

	dir = vector_dot_product (&ship->rotmat[2], &vec);

	if (dir < -0.98)
	{
		fly_to_docking_bay (ship);
		return;
	}

	fly_to_station (ship);
}


void engage_auto_pilot (void)
{
	if(auto_pilot || witchspace || hyper_ready)
		return; 

	auto_pilot = 1;
//	snd_play_midi (SND_BLUE_DANUBE, 1);
}


void disengage_auto_pilot (void)
{
	if (auto_pilot)
	{
		auto_pilot = 0;
//		snd_stop_midi();
	}
}
 
