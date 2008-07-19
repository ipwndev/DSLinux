






/*
 * Generate a random number between 0 and 255.
 * This is the version used in the 6502 Elites.
 */

int gen_rnd_number (void)
{
	int a,x;

	x = (rnd_seed.a * 2) & 0xFF;
	a = x + rnd_seed.c;
	if (rnd_seed.a > 127)
		a++;
	rnd_seed.a = a & 0xFF;
	rnd_seed.c = x;

	a = a / 256;	/* a = any carry left from above */
	x = rnd_seed.b;
	a = (a + x + rnd_seed.d) & 0xFF;
	rnd_seed.b = a;
	rnd_seed.d = x;
	return a;
}


/*
 * Generate a random number between 0 and 255.
 * This is the version used in the MSX and 16bit Elites.
 */


int gen_msx_rnd_number (void)
{
    int a,b;

	a = rnd_seed.a;
	b = rnd_seed.b;
	
	rnd_seed.a = rnd_seed.c;
	rnd_seed.b = rnd_seed.d;
	
	a += rnd_seed.c;
	b = (b + rnd_seed.d) & 255;
	if (a > 255)
	{
		a &= 255;
		b++;
	}
	
	rnd_seed.c = a;
	rnd_seed.d = b;
	
	return rnd_seed.c / 0x34;
}


void waggle_galaxy (struct galaxy_seed *glx_ptr)
{
    unsigned int x;
	unsigned int y;
	extern int carry_flag;

	x = glx_ptr->a + glx_ptr->c;
    y = glx_ptr->b + glx_ptr->d;


	if (x > 0xFF)
	    y++;

	x &= 0xFF;
	y &= 0xFF;

	glx_ptr->a = glx_ptr->c;
	glx_ptr->b = glx_ptr->d;
	glx_ptr->c = glx_ptr->e;
	glx_ptr->d = glx_ptr->f;

    x += glx_ptr->c;
	y += glx_ptr->d;


	if (x > 0xFF)
		y++;

	if (y > 0xFF)
		carry_flag = 1;
	else
		carry_flag = 0;

    x &= 0xFF;
	y &= 0xFF;

	glx_ptr->e = x;
	glx_ptr->f = y;
}




struct galaxy_seed find_planet (int cx, int cy)
{
    int min_dist = 10000;
	struct galaxy_seed glx;
	struct galaxy_seed planet;
	int distance;
	int dx, dy;
	int i;

	glx = cmdr.galaxy;

	for (i = 0; i < 256; i++)
	{

		dx = abs(cx - glx.d);
		dy = abs(cy - glx.b);

		if (dx > dy)
			distance = (dx + dx + dy) / 2;
		else
			distance = (dx + dy + dy) / 2;

		if (distance < min_dist)
		{
			min_dist = distance;
			planet = glx;
		}

		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
	}

	return planet;
}


int find_planet_number (struct galaxy_seed planet)
{
	struct galaxy_seed glx;
	int i;

	glx = cmdr.galaxy;

	for (i = 0; i < 256; i++)
	{

		if ((planet.a == glx.a) &&
			(planet.b == glx.b) &&
			(planet.c == glx.c) &&
			(planet.d == glx.d) &&
			(planet.e == glx.e) &&
			(planet.f == glx.f))
			return i;
	
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
	}

	return -1;
}



void name_planet (char *gname, struct galaxy_seed glx)
{
    int size;
	int i;
	char *gp;
	unsigned int x;


	gp = gname;

	if ((glx.a & 0x40) == 0)
		size = 3;
	else
		size = 4;

	for (i = 0; i < size; i++)
	{
		x = glx.f & 0x1F;
		if (x != 0)
		{
			x += 12;
			x *= 2;
			*gp++ = digrams[x];
			if (digrams[x+1] != '?')
				*gp++ = digrams[x+1];
		}

		waggle_galaxy (&glx);
	}

	*gp = '\0';
}


void capitalise_name (char *name)
{
    char *ptr = name;

	if (*ptr == '\0')
		return;

	*ptr = toupper(*ptr);
	ptr++;

	while (*ptr != '\0')
	{
	    *ptr = tolower(*ptr);
		ptr++;
	}
}


void describe_inhabitants (char *str, struct galaxy_seed planet)
{
	int inhab;

	strcpy (str, "(");

	if (planet.e < 128)
	{
		strcat (str, "Human Colonial");
	}
	else
	{
		inhab = (planet.f / 4) & 7;
		if (inhab < 3)
			strcat (str, inhabitant_desc1[inhab]);

		inhab = planet.f / 32;
		if (inhab < 6)
			strcat (str, inhabitant_desc2[inhab]);

		inhab = (planet.d ^ planet.b) & 7;
		if (inhab < 6)
			strcat (str, inhabitant_desc3[inhab]);

		inhab = (inhab + (planet.f & 3)) & 7;
		strcat (str, inhabitant_desc4[inhab]);
	}

	strcat (str, "s)");
}



void expand_description (char *source)
{
	char *ptr;
	int num;
	int rnd;
	int option;
	int i, len, x;

	while (*source != '\0')
	{
		if (*source == '<')
		{
			source++;
			ptr = str;
			while (*source != '>')
				*ptr++ = *source++;
			*ptr = '\0';
			source++;
			num = atoi(str);
			
			if (hoopy_casinos)
			{
				option = gen_msx_rnd_number();
			}
			else
			{
				rnd = gen_rnd_number();
				option = 0;
				if (rnd >= 0x33) option++;
				if (rnd >= 0x66) option++;
				if (rnd >= 0x99) option++;
				if (rnd >= 0xCC) option++;
			}
			
			expand_description (desc_list[num][option]);
			continue;
		}

		if (*source == '%')
		{
			source++;
			switch (*source)
			{
				case 'H':
					name_planet (str, hyperspace_planet);
					capitalise_name (str);
					for (ptr = str; *ptr != '\0';)
						*desc_ptr++ = *ptr++;
					break;

				case 'I':
					name_planet (str, hyperspace_planet);
					capitalise_name (str);
					for (ptr = str; *ptr != '\0';)
						*desc_ptr++ = *ptr++;
						desc_ptr--;
					strcpy (desc_ptr, "ian");
					desc_ptr += 3;
					break;

				case 'R':
					len = gen_rnd_number() & 3;
					for (i = 0; i <= len; i++)
					{
						x = gen_rnd_number() & 0x3e;
						if (i == 0)
						    *desc_ptr++ = digrams[x];
						else
							*desc_ptr++ = tolower(digrams[x]);
						*desc_ptr++ = tolower(digrams[x+1]);
					}

			}

			source++;
			continue;
		}

		*desc_ptr++ = *source++;
	}



	*desc_ptr = '\0';
}



char *describe_planet (struct galaxy_seed planet)
{
	char *mission_text;
	
	if (cmdr.mission == 1)
	{
		mission_text = mission_planet_desc (planet);
		if (mission_text != NULL)
			return mission_text;
	}
	
	if(razaar_refugees>100 && (planet.d == 112) && 
		(planet.b == 95) && cmdr.galaxy_number == 0 )
	{
		mission_text= "The world Razaar was a dull place. Now it is a Binary Star "
					   "System after Supernova explosion ignited the planet which became a Sun itself.";
	
		return mission_text;
	}	


	if(planet.d == 150 && planet.b == 130 && cmdr.galaxy_number == 2) 
	{

		if(gal_fam ==-500)
			mission_text= "A starving planet with a glimmer of hope.";
		else
			mission_text= "A hopless starving planet.";

		return mission_text;

	}





	rnd_seed.a = planet.c;
	rnd_seed.b = planet.d;
	rnd_seed.c = planet.e;
	rnd_seed.d = planet.f;

	if (hoopy_casinos)
	{
		rnd_seed.a ^= planet.a;
		rnd_seed.b ^= planet.b;
		rnd_seed.c ^= rnd_seed.a;
		rnd_seed.d ^= rnd_seed.b;
	}
	
	desc_ptr = planet_description;

	expand_description ("<14> is <22>.");

	return planet_description;
}



void generate_planet_data (struct planet_data *pl, struct galaxy_seed planet_seed)
{

	pl->government = (planet_seed.c / 8) & 7;

	pl->economy = planet_seed.b & 7;

	if (pl->government < 2)
		pl->economy = pl->economy | 2;

	pl->techlevel = pl->economy ^ 7;
	pl->techlevel += planet_seed.d & 3;
	pl->techlevel += (pl->government / 2) + (pl->government & 1);


	pl->population = pl->techlevel * 4;
	pl->population += pl->government;
	pl->population += pl->economy;
	pl->population++;

	pl->productivity = (pl->economy ^ 7) + 3;
	pl->productivity *= pl->government + 4;
	pl->productivity *= pl->population;
	pl->productivity *= 8;

	pl->radius = (((planet_seed.f & 15) + 11) * 256) + planet_seed.d;
}



