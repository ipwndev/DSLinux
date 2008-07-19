

void draw_fuel_limit_circle (int cx, int cy)
{
	int radius;
	int cross_size;

	if (current_screen == SCR_GALACTIC_CHART)
	{
		radius = cmdr.fuel / 4 * GFX_SCALE;
		cross_size = 7 * GFX_SCALE;
	}
	else
	{
		radius = cmdr.fuel * GFX_SCALE;
		cross_size = 16 * GFX_SCALE;
	}
	
	gfx_draw_circle (cx, cy, radius, GFX_COL_GREEN_1);

	gfx_draw_line (cx, cy - cross_size, cx, cy + cross_size);
	gfx_draw_line (cx - cross_size, cy, cx + cross_size, cy);
}







void show_distance (int ypos, struct galaxy_seed from_planet, struct galaxy_seed to_planet)
{
	int light_years;

	light_years = calc_distance_to_planet (from_planet, to_planet);
	
	sprintf (str, "%d.%d Light Years ", light_years / 10, light_years % 10);

	gfx_display_colour_text (16, ypos, "Distance", GFX_COL_GREEN_1);
	gfx_display_text (100, ypos, str);


}



void show_distance_to_planet (void)
{
	int px,py;

	if (current_screen == SCR_GALACTIC_CHART)
	{
		px = cross_x / GFX_SCALE;
		py = (cross_y - ((18 * GFX_SCALE) + 1)) * (2 / GFX_SCALE);
	}
	else
	{
		px = ((cross_x - GFX_X_CENTRE) / (4 * GFX_SCALE)) + docked_planet.d;
		py = ((cross_y - GFX_Y_CENTRE) / (2 * GFX_SCALE)) + docked_planet.b;
	}

	hyperspace_planet = find_planet (px, py);


	if (current_screen == SCR_GALACTIC_CHART)
	{
		cross_x = hyperspace_planet.d * GFX_SCALE;
		cross_y = hyperspace_planet.b / (2 / GFX_SCALE) + (18 * GFX_SCALE) + 1;
	}
	else
	{
		cross_x = ((hyperspace_planet.d - docked_planet.d) * (4 * GFX_SCALE)) + GFX_X_CENTRE;
		cross_y = ((hyperspace_planet.b - docked_planet.b) * (2 * GFX_SCALE)) + GFX_Y_CENTRE;
	}


	if (cross_x < 1)
		cross_x = 1;
		
	if (cross_x > 510)
		cross_x = 510;

	if (cross_y < 37)
		cross_y = 37;
	
	if (cross_y > 383)
		cross_y = 383;

}


void move_cursor_to_origin (void)
{
	if (current_screen == SCR_GALACTIC_CHART)
	{
		cross_x = docked_planet.d * GFX_SCALE;
		cross_y = docked_planet.b / (2 / GFX_SCALE) + (18 * GFX_SCALE) + 1;
	}
	else
	{
		cross_x = GFX_X_CENTRE;
		cross_y = GFX_Y_CENTRE;
	}

	show_distance_to_planet();
}


void find_planet_by_name (char *f_n)
{
    int i;
	struct galaxy_seed glx;
	char planet_name[16];
	int found;
	
	glx = cmdr.galaxy;
	found = 0;
	
	for (i = 0; i < 256; i++)
	{
		name_planet (planet_name, glx);
		
		if (strcmp (planet_name, f_n) == 0)
		{
			found = 1;
			break;
		}

		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
	}

	if (!found)
	{
	//	sprintf(str, "Unknown Planet %s", f_n);
	//	gfx_display_text (200, 340, str);
		return;
	}

	hyperspace_planet = glx;
	show_distance (356, docked_planet, hyperspace_planet);

	if (current_screen == SCR_GALACTIC_CHART)
	{
		cross_x = hyperspace_planet.d * GFX_SCALE;
		cross_y = hyperspace_planet.b / (2 / GFX_SCALE) + (18 * GFX_SCALE) + 1;
	}
	else
	{
		cross_x = ((hyperspace_planet.d - docked_planet.d) * (4 * GFX_SCALE)) + GFX_X_CENTRE;
		cross_y = ((hyperspace_planet.b - docked_planet.b) * (2 * GFX_SCALE)) + GFX_Y_CENTRE;
	}
}



void display_short_range_chart (void)
{
    int i;
	struct galaxy_seed glx;
	int dx,dy;
	int px,py;
	char planet_name[16];
	int row_used[64];
	int row;
	int blob_size;

	current_screen = SCR_SHORT_RANGE;

	gfx_clear_display();

	gfx_display_centre_text (10, "SHORT RANGE CHART", 140, GFX_COL_GOLD);

	gfx_draw_line (0, 36, 511, 36);

	draw_fuel_limit_circle (GFX_X_CENTRE, GFX_Y_CENTRE);

	for (i = 0; i < 64; i++)
		row_used[i] = 0;

	glx = cmdr.galaxy;

	for (i = 0; i < 256; i++)
	{

		dx = abs (glx.d - docked_planet.d);
		dy = abs (glx.b - docked_planet.b);

		if ((dx >= 20) || (dy >= 38))
		{
			waggle_galaxy (&glx);
			waggle_galaxy (&glx);
			waggle_galaxy (&glx);
			waggle_galaxy (&glx);

			continue;
		}

		px = (glx.d - docked_planet.d);
		px = px * 4 * GFX_SCALE + GFX_X_CENTRE;  /* Convert to screen co-ords */

		py = (glx.b - docked_planet.b);
		py = py * 2 * GFX_SCALE + GFX_Y_CENTRE;	/* Convert to screen co-ords */

		row = py / (8 * GFX_SCALE);

		if (row_used[row] == 1)
		    row++;

		if (row_used[row] == 1)
			row -= 2;

		if (row <= 3)
		{
			waggle_galaxy (&glx);
			waggle_galaxy (&glx);
			waggle_galaxy (&glx);
			waggle_galaxy (&glx);

			continue;
		}

		if (row_used[row] == 0)
		{
			row_used[row] = 1;

			name_planet (planet_name, glx);
			capitalise_name (planet_name);

			gfx_display_text (px + (4 * GFX_SCALE), (row * 8 - 5) * GFX_SCALE, planet_name);
		}


		/* The next bit calculates the size of the circle used to represent */
		/* a planet.  The carry_flag is left over from the name generation. */
		/* Yes this was how it was done... don't ask :-( */

		blob_size = (glx.f & 1) + 2 + carry_flag;
		blob_size *= GFX_SCALE;
		gfx_draw_filled_circle (px, py, blob_size, GFX_COL_GOLD);

		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
	}

	name_planet (planet_name, hyperspace_planet);

	sprintf (str, "%-18s", planet_name);
	gfx_display_text (16, 340, str);

	show_distance (356, docked_planet, hyperspace_planet);

	draw_cross (cross_x, cross_y);

}




void display_galactic_chart (void)
{

    int i;
	struct galaxy_seed glx;
	int px,py;
	

	current_screen = SCR_GALACTIC_CHART;

	gfx_clear_display();

	sprintf (str, "GALACTIC CHART %d", cmdr.galaxy_number + 1);

	gfx_display_centre_text (10, str, 140, GFX_COL_GOLD);

	gfx_draw_line (0, 36, 511, 36);
	gfx_draw_line (0, 36+258, 511, 36+258);

	draw_fuel_limit_circle (docked_planet.d * GFX_SCALE,
					(docked_planet.b / (2 / GFX_SCALE)) + (18 * GFX_SCALE) + 1);

	glx = cmdr.galaxy;

	for (i = 0; i < 256; i++)
	{
		px = glx.d * GFX_SCALE;
		py = (glx.b / (2 / GFX_SCALE)) + (18 * GFX_SCALE) + 1;

		gfx_plot_pixel (px, py, GFX_COL_WHITE);

		if ((glx.e | 0x50) < 0x90)
			gfx_plot_pixel (px + 1, py, GFX_COL_WHITE);

		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);

	}
	

	char planet_name[16];
	name_planet (planet_name, hyperspace_planet);

//	sprintf (str, "%-18s gov ...(pl.num=%d) --> pl.d=%d, pl.b=%d ", hyperspace_planet. planet_name, find_planet_number(hyperspace_planet), hyperspace_planet.d, hyperspace_planet.b);

	sprintf (str, "%-18s", planet_name);
	gfx_display_text (16, 340, str);

	show_distance (356, docked_planet, hyperspace_planet);

	draw_cross (cross_x, cross_y);

}





/*
 * Displays data on the currently selected Hyperspace Planet.
 */

void display_data_on_planet (void)
{
    char planet_name[16];
	char *description;
	struct planet_data hyper_planet_data;

	current_screen = SCR_PLANET_DATA;


	name_planet (planet_name, hyperspace_planet);
	sprintf (str, "DATA ON %s", planet_name);

	gfx_display_centre_text (10, str, 140, GFX_COL_GOLD);

	gfx_draw_line (0, 36, 511, 36);


	generate_planet_data (&hyper_planet_data, hyperspace_planet);

	show_distance (42, docked_planet, hyperspace_planet);



	gfx_display_colour_text (16, 74, "Economy", GFX_COL_GREEN_1);
	sprintf (str, "%s", economy_type[hyper_planet_data.economy]);
	gfx_display_text (100, 74, str);

	gfx_display_colour_text (16, 106, "Government", GFX_COL_GREEN_1);
	sprintf (str, "%s", government_type[hyper_planet_data.government]);
	gfx_display_text (120, 106, str);

	gfx_display_colour_text (16, 138, "Tech.Level", GFX_COL_GREEN_1);
	sprintf (str, "%d", hyper_planet_data.techlevel + 1);
	gfx_display_text (120, 138, str);

	gfx_display_colour_text (16, 170, "Population", GFX_COL_GREEN_1);
	sprintf (str, "%d.%d Billion", hyper_planet_data.population / 10, hyper_planet_data.population % 10);
	gfx_display_text (120, 170, str);

	describe_inhabitants (str, hyperspace_planet);
	gfx_display_text (16, 202, str);

	gfx_display_colour_text (16, 234, "Gross Productivity", GFX_COL_GREEN_1);
	sprintf (str, "%d M.Cr", hyper_planet_data.productivity);
	gfx_display_text (192, 234, str);

	gfx_display_colour_text (16, 266, "Average Radius", GFX_COL_GREEN_1);
	sprintf (str, "%d km", hyper_planet_data.radius);
	gfx_display_text (155, 266, str);


	description = describe_planet (hyperspace_planet);
	gfx_display_pretty_text (16, 298, 400, 384, description);


	int x1= 430,
		y1= 90;

	int col=0;
	unsigned char pl_f= hyperspace_planet.f;




	render_sun(x1-70, y1+20, 25 + pl_f/8 );
	

	if(pl_f<50) col= GFX_COL_RED_4;
	else
	if(pl_f<100) col= GFX_COL_BLUE_5;
	else
	if(pl_f<150) col= GFX_COL_GREEN_4;
	else
	if(pl_f<200) col= GFX_COL_GREY_4;
	else
		col= GFX_COL_ORANGE_4;

	gfx_draw_filled_circle((x1-GFX_X_OFFSET+120) - 0, (y1-GFX_Y_OFFSET + 120) - 0, 
										hyper_planet_data.radius/100, col);



}







char *laser_type (int strength)
{
	switch (strength)
	{
		case PULSE_LASER:
			return laser_name[0];

		case BEAM_LASER:
			return laser_name[1];
		
		case MILITARY_LASER:
			return laser_name[2];
		
		case MINING_LASER:
			return laser_name[3];
	}	

	return laser_name[4];
}





void display_commander_status (void)
{
    char planet_name[16];
	int i;
	int x,y;
	
	current_screen = SCR_CMDR_STATUS;

	sprintf (str, "COMMANDER %s", cmdr.name);

	gfx_display_centre_text (10, str, 140, GFX_COL_GOLD);

	gfx_draw_line (0, 36, 511, 36);


	gfx_display_colour_text (16, 58, "Present System", GFX_COL_GREEN_1);
	
	if (!witchspace)
	{
		name_planet (planet_name, docked_planet);
		capitalise_name (planet_name);
		sprintf (str, "%s", planet_name);
		gfx_display_text (182, 58, str);
	}

	gfx_display_colour_text (16, 74, "Hyperspace System", GFX_COL_GREEN_1);
	name_planet (planet_name, hyperspace_planet);
	capitalise_name (planet_name);
	sprintf (str, "%s", planet_name);
	gfx_display_text (182, 74, str);



	
	gfx_display_colour_text (16, 90, "Condition", GFX_COL_GREEN_1);
	gfx_display_text (182, 90, condition_txt[condition]);


	sprintf (str, "%d.%d Light Years", cmdr.fuel / 10, cmdr.fuel % 10);
	gfx_display_colour_text (16, 106, "Fuel", GFX_COL_GREEN_1);
	gfx_display_text (62, 106, str);

	sprintf (str, "%d.%d Cr", cmdr.credits / 10, cmdr.credits % 10);
	gfx_display_colour_text (16, 122, "Cash", GFX_COL_GREEN_1);
	gfx_display_text (62, 122, str);

	if(cmdr.legal_status == 0)
		strcpy (str, "Clean");
	else
		strcpy (str, cmdr.legal_status > 100 ? "Fugitive" : "Offender");

	gfx_display_colour_text (16, 138, "Legal Status", GFX_COL_GREEN_1);
	sprintf(str, "%s (%d/255)", str, cmdr.legal_status);
	gfx_display_text (136, 138, str);

	for (i = 0; i < NO_OF_RANKS; i++)
		if (cmdr.score >= rating[i].score)
			strcpy (str, rating[i].title);
				
	gfx_display_colour_text (16, 154, "Rating", GFX_COL_GREEN_1);

		
	if(cheat) 
		strcat(str, " (chEaT)");		
	
	gfx_display_text (82, 154, str );

	gfx_display_colour_text (16, 186, "EQUIPMENT", GFX_COL_GREEN_1);

	x = EQUIP_START_X;
	y = EQUIP_START_Y;

	if (cmdr.cargo_capacity > 20)
	{
		gfx_display_text (x, y, "Large Cargo Bay");
		y += Y_INC;
	}
	
	if (cmdr.escape_pod)
	{
		gfx_display_text (x, y, "Escape Pod");
		y += Y_INC;
	}
	
	if (cmdr.fuel_scoop)
	{
		gfx_display_text (x, y, "Fuel Scoops");
		y += Y_INC;
	}

	if (cmdr.ecm)
	{
		gfx_display_text (x, y, "E.C.M. System");
		y += Y_INC;
	}

	if (cmdr.energy_bomb)
	{
		gfx_display_text (x, y, "Energy Bomb");
		y += Y_INC;
	}

	if (cmdr.energy_unit)
	{
	    if(cmdr.energy_unit <2)
    		gfx_display_text (x, y, "Extra Energy Unit");
	    else
		gfx_display_text (x, y, "Naval Energy Unit");

		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}

	if (cmdr.docking_computer)
	{
		gfx_display_text (x, y, "Docking Computer");
		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}

	
	if (cmdr.galactic_hyperdrive)
	{
		gfx_display_text (x, y, "Galactic Hyperspace");
		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}

	if (cmdr.front_laser)
	{
		sprintf (str, "Front %s Laser", laser_type(cmdr.front_laser));
		gfx_display_text (x, y, str);
		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}
	
	if (cmdr.rear_laser)
	{
		sprintf (str, "Rear %s Laser", laser_type(cmdr.rear_laser));
		gfx_display_text (x, y, str);
		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}

	if (cmdr.left_laser)
	{
		sprintf (str, "Left %s Laser", laser_type(cmdr.left_laser));
		gfx_display_text (x, y, str);
		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}

	if (cmdr.right_laser)
	{
		sprintf (str, "Right %s Laser", laser_type(cmdr.right_laser));
		gfx_display_text (x, y, str);
	}



	glColor3f(0,1,0);
	sprintf(str, "Score"); 
	txtOut(510,53, str, 2.5);

	glColor3f(1,1,1);
	sprintf(str, "%06d", cmdr.score); 
	txtOut(578,53, str, 2.5);


}



/***********************************************************************************/



void display_stock_price (int i)
{
	int y;

	y = i * 15 + 55;

	gfx_display_text (16, y, stock_market[i].name);

	gfx_display_text (180, y, unit_name[stock_market[i].units]);
	sprintf (str, "%d.%d", stock_market[i].current_price / 10,
						   stock_market[i].current_price % 10);
	gfx_display_text (256, y, str);

	if (stock_market[i].current_quantity > 0)
		sprintf (str, "%d%s", stock_market[i].current_quantity,
							  unit_name[stock_market[i].units]);
	else
		strcpy (str, "-");

	gfx_display_text (338, y, str);

	if (cmdr.current_cargo[i] > 0)
		sprintf (str, "%d%s", cmdr.current_cargo[i],
							  unit_name[stock_market[i].units]);
	else
		strcpy (str, "-");

	gfx_display_text (444, y, str);
}


void highlight_stock (int i)
{
	int y;
	
	if(gal_fam >0 && i ==0) i= 1;


	if ((hilite_item != -1) && (hilite_item != i))
	{
		y = hilite_item * 15 + 61;
		display_stock_price (hilite_item);		
	}

	y = i * 15 + 61;
	
	gfx_draw_rectangle (2, y, 510, y + 15, GFX_COL_DARK_RED);
	display_stock_price (i);		

	hilite_item = i;


	gfx_display_colour_text (16, 340, "Cash", GFX_COL_GREEN_1);
	sprintf (str, "%d.%d Cr", cmdr.credits / 10, cmdr.credits % 10);
	gfx_display_text (60, 340, str);

	sprintf (str, "%dt", cmdr.cargo_capacity);
	gfx_display_colour_text (200, 340, "Max.Cargo", GFX_COL_GREEN_1);
	gfx_display_text (290, 340, str);


}

void select_previous_stock (void)
{
	if (!docked || (hilite_item == 0))
		return;

	highlight_stock (hilite_item - 1);
}

void select_next_stock (void)
{
	if (!docked || (hilite_item == 16))
		return;

	highlight_stock (hilite_item + 1);
}


void buy_stock (void)
{
	struct stock_item *item;
	int cargo_held;
	
	if (!docked)
		return;

	item = &stock_market[hilite_item];
		
	if ((item->current_quantity == 0) ||
	    (cmdr.credits < item->current_price))
		return;

	cargo_held = total_cargo();
	
	if ((item->units == TONNES) &&
		(cargo_held == cmdr.cargo_capacity))
		return;
	
	cmdr.current_cargo[hilite_item]++;
	item->current_quantity--;
	cmdr.credits -= item->current_price;	

	highlight_stock (hilite_item);
}


void sell_stock (void)
{
	struct stock_item *item;
	
	if ((!docked) || (cmdr.current_cargo[hilite_item] == 0))
		return;

	item = &stock_market[hilite_item];

	cmdr.current_cargo[hilite_item]--;
	item->current_quantity++;
	cmdr.credits += item->current_price;	

	highlight_stock (hilite_item);
}



void display_market_prices (void)
{


    char planet_name[16];
	int i;

	current_screen = SCR_MARKET_PRICES;


	name_planet (planet_name, docked_planet);
	sprintf (str, "%s MARKET PRICES", planet_name);
	gfx_display_centre_text (10, str, 140, GFX_COL_GOLD);

	gfx_draw_line (0, 36, 511, 36);


	if(witchspace) return;
	
	if((docked_planet.d == 112) && (docked_planet.b == 95) && 
		(cmdr.galaxy_number == 0) && razaar_refugees>0) 
		return;


	gfx_display_colour_text (16,  40, "PRODUCT", GFX_COL_GREEN_1);
	gfx_display_colour_text (166, 40, "UNIT", GFX_COL_GREEN_1);
	gfx_display_colour_text (246, 40, "PRICE", GFX_COL_GREEN_1);
	gfx_display_colour_text (314, 40, "FOR SALE", GFX_COL_GREEN_1);
	gfx_display_colour_text (420, 40, "IN HOLD", GFX_COL_GREEN_1);


	for (i = 0; i < 17; i++)
		display_stock_price (i);


	if(docked)
		highlight_stock(hilite_item);
}




void display_inventory (void)
{

	int i;
	int y;
	
	current_screen = SCR_INVENTORY;

	gfx_clear_display();
	gfx_display_centre_text (10, "INVENTORY", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	sprintf (str, "%dt", cmdr.cargo_capacity);
	gfx_display_colour_text (200, 58, "Max.Cargo", GFX_COL_GREEN_1);
	gfx_display_text (290, 58, str);
	
	sprintf (str, "%d.%d Light Years", cmdr.fuel / 10, cmdr.fuel % 10);
	gfx_display_colour_text (16, 40, "Fuel", GFX_COL_GREEN_1);
	gfx_display_text (62, 40, str);

	sprintf (str, "%d.%d Cr", cmdr.credits / 10, cmdr.credits % 10);
	gfx_display_colour_text (16, 58, "Cash", GFX_COL_GREEN_1);
	gfx_display_text (62, 58, str);

	cargo_item_count= 0;

	y = 90;
	for (i = 0; i < 17; i++)
	{
		if (cmdr.current_cargo[i] > 0)
		{
			cargo_item_count++;

			
			if(gal_fam >0 && i ==0)
				gfx_display_text (16, y, "Ecology Seed");
			else
				gfx_display_text (16, y, stock_market[i].name);

			sprintf (str, "%d%s", cmdr.current_cargo[i],
							  unit_name[stock_market[i].units]);

			gfx_display_text (180, y, str);

			if(!docked && hilite_item==cargo_item_count)
			{
				gfx_display_colour_text (220, y, "-> Jettison", GFX_COL_RED_1);
				current_jettison= i;
			}
			y += 16;

		}		
	}







//**zurid pino
	if(zpino==100)
	{
		gfx_display_colour_text (400, 58, "Prisoner", GFX_COL_GREEN_1);
		gfx_display_text (400, 76, "Zurid Pino");
	}


//**refugees
	if(razaar_refugees==100 || razaar_refugees==101) //101 - after nova occured tels gfx to draw different sun, terrible
	{
		gfx_display_colour_text (358, 58, "Spec. Cargo", GFX_COL_GREEN_1);
		gfx_display_text (467, 58, "12t");

		gfx_display_text (358, 76, "Razaar Refugees");

	}


}

/***********************************************************************************/



int equip_present (int type)
{
	switch (type)
	{
		case EQ_FUEL:
			return (cmdr.fuel >= 70);
		
		case EQ_MISSILE:
			return (cmdr.missiles >= 4);
		
		case EQ_CARGO_BAY:
			return (cmdr.cargo_capacity > 20);
		
		case EQ_ECM:
			return cmdr.ecm;
		
		case EQ_FUEL_SCOOPS:
			return cmdr.fuel_scoop;
		
		case EQ_ESCAPE_POD:
			return cmdr.escape_pod;
		
		case EQ_ENERGY_BOMB:
			return cmdr.energy_bomb;

		case EQ_ENERGY_UNIT:
			return cmdr.energy_unit;
			
		case EQ_DOCK_COMP:
			return cmdr.docking_computer;
			
		case EQ_GAL_DRIVE:
			return cmdr.galactic_hyperdrive;
			
		case EQ_FRONT_PULSE:
			return (cmdr.front_laser == PULSE_LASER);
		
		case EQ_REAR_PULSE:
			return (cmdr.rear_laser == PULSE_LASER);

		case EQ_LEFT_PULSE:
			return (cmdr.left_laser == PULSE_LASER);

		case EQ_RIGHT_PULSE:
			return (cmdr.right_laser == PULSE_LASER);

		case EQ_FRONT_BEAM:
			return (cmdr.front_laser == BEAM_LASER);

		case EQ_REAR_BEAM:
			return (cmdr.rear_laser == BEAM_LASER);

		case EQ_LEFT_BEAM:
			return (cmdr.left_laser == BEAM_LASER);

		case EQ_RIGHT_BEAM:
			return (cmdr.right_laser == BEAM_LASER);

		case EQ_FRONT_MINING:
			return (cmdr.front_laser == MINING_LASER);

		case EQ_REAR_MINING:
			return (cmdr.rear_laser == MINING_LASER);

		case EQ_LEFT_MINING:
			return (cmdr.left_laser == MINING_LASER);

		case EQ_RIGHT_MINING:
			return (cmdr.right_laser == MINING_LASER);

		case EQ_FRONT_MILITARY:
			return (cmdr.front_laser == MILITARY_LASER);

		case EQ_REAR_MILITARY:
			return (cmdr.rear_laser == MILITARY_LASER);

		case EQ_LEFT_MILITARY:
			return (cmdr.left_laser == MILITARY_LASER);

		case EQ_RIGHT_MILITARY:
			return (cmdr.right_laser == MILITARY_LASER);
	}

	return 0;
}


void display_equip_price (int i)
{



	int x, y;
	int col;
	
	y = equip_stock[i].y;
	if (y == 0)
		return;

	col = equip_stock[i].canbuy ? GFX_COL_WHITE : GFX_COL_GREY_2;

	x = *(equip_stock[i].name) == '>' ? 50 : 16; 

	gfx_display_colour_text (x, y, &equip_stock[i].name[0], col);

	if (equip_stock[i].price != 0)
	{
		sprintf (str, "%d.%d", equip_stock[i].price / 10, equip_stock[i].price % 10);
		gfx_display_colour_text (338, y, str, col);
	}
}


void highlight_equip (int i)
{
	int y;
	
	if ((hilite_item != -1) && (hilite_item != i))
	{
		y = equip_stock[hilite_item].y+6;
		display_equip_price (hilite_item);		
	}

	y = equip_stock[i].y+6;
	
	gfx_draw_rectangle (2, y+1, 510, y + 15, GFX_COL_DARK_RED);
	display_equip_price (i);		

	hilite_item = i;

	gfx_display_colour_text (16, 340, "Cash", GFX_COL_GREEN_1);
	sprintf (str, "%d.%d Cr", cmdr.credits / 10, cmdr.credits % 10);
	gfx_display_text (60, 340, str);


}


void select_next_equip (void)
{
	int next;
	int i;

	if (hilite_item == (NO_OF_EQUIP_ITEMS - 1))
		return;

	next = hilite_item;
	for (i = hilite_item + 1; i < NO_OF_EQUIP_ITEMS; i++)
	{
		if (equip_stock[i].y != 0)
		{
			next = i;
			break;
		}
	}

	if (next != hilite_item)	
		highlight_equip (next);
}

void select_previous_equip (void)
{
	int i;
	int prev;
	
	if (hilite_item == 0)
		return;
	
	prev = hilite_item;
	for (i = hilite_item - 1; i >= 0; i--)
	{
		if (equip_stock[i].y != 0)
		{
			prev = i;
			break;
		}
	}

	if (prev != hilite_item)	
		highlight_equip (prev);
}


void list_equip_prices (void)
{


	if((docked_planet.d == 112) && (docked_planet.b == 95) && 
		(cmdr.galaxy_number == 0) && razaar_refugees>0) 
		return;

	int i;
	int y;
	int tech_level;

	
	tech_level = current_planet_data.techlevel + 1;

	equip_stock[0].price = (70 - cmdr.fuel) * 2;
	
	y = 55;
	for (i = 0; i < NO_OF_EQUIP_ITEMS; i++)
	{
	    equip_stock[i].canbuy = ((equip_present (equip_stock[i].type) == 0) &&
								 (equip_stock[i].price <= cmdr.credits));
	
		if (equip_stock[i].show && (tech_level >= equip_stock[i].level))
		{
			equip_stock[i].y = y;
			y += 15;
		}
		else
			equip_stock[i].y = 0;

		display_equip_price (i);
	}
	
	i = hilite_item;
	hilite_item = -1;
	highlight_equip (i);
}


void collapse_equip_list (void)
{
	int i;
	int ch;
	
	for (i = 0; i < NO_OF_EQUIP_ITEMS; i++)
	{
		ch = *(equip_stock[i].name);
		equip_stock[i].show = ((ch == ' ') || (ch == '+'));
	}
}


int laser_refund (int laser_type)
{
	switch (laser_type)
	{
		case PULSE_LASER:
			return 4000;
		
		case BEAM_LASER:
			return 10000;
		
		case MILITARY_LASER:
			return 60000;
		
		case MINING_LASER:
			return 8000;
	}

	return 0;
}


void buy_equip (void)
{

	if (equip_stock[hilite_item].canbuy == 0)
		return;
	
	switch (equip_stock[hilite_item].type)
	{
		case EQ_FUEL:
			cmdr.fuel = myship.max_fuel;
			update_console();
			break;

		case EQ_MISSILE:
			cmdr.missiles++;
			update_console();
			break;
		
		case EQ_CARGO_BAY:
			cmdr.cargo_capacity+= 15; 
			break;
		
		case EQ_ECM:
			cmdr.ecm = 1;
			break;
		
		case EQ_FUEL_SCOOPS:
			cmdr.fuel_scoop = 1;
			break;
		
		case EQ_ESCAPE_POD:
			cmdr.escape_pod = 1;
			break;
		
		case EQ_ENERGY_BOMB:
			cmdr.energy_bomb = 1;
			break;

		case EQ_ENERGY_UNIT:
			cmdr.energy_unit = 1;
			break;
			
		case EQ_DOCK_COMP:
			cmdr.docking_computer = 1;
			break;
			
		case EQ_GAL_DRIVE:
			cmdr.galactic_hyperdrive = 1;
			break;
			
		case EQ_FRONT_PULSE:
			cmdr.credits += laser_refund (cmdr.front_laser);
			cmdr.front_laser = PULSE_LASER;
			break;
		
		case EQ_REAR_PULSE:
			cmdr.credits += laser_refund (cmdr.rear_laser);
			cmdr.rear_laser = PULSE_LASER;
			break;

		case EQ_LEFT_PULSE:
			cmdr.credits += laser_refund (cmdr.left_laser);
			cmdr.left_laser = PULSE_LASER;
			break;

		case EQ_RIGHT_PULSE:
			cmdr.credits += laser_refund (cmdr.right_laser);
			cmdr.right_laser = PULSE_LASER;
			break;

		case EQ_FRONT_BEAM:
			cmdr.credits += laser_refund (cmdr.front_laser);
			cmdr.front_laser = BEAM_LASER;
			break;

		case EQ_REAR_BEAM:
			cmdr.credits += laser_refund (cmdr.rear_laser);
			cmdr.rear_laser = BEAM_LASER;
			break;

		case EQ_LEFT_BEAM:
			cmdr.credits += laser_refund (cmdr.left_laser);
			cmdr.left_laser = BEAM_LASER;
			break;

		case EQ_RIGHT_BEAM:
			cmdr.credits += laser_refund (cmdr.right_laser);
			cmdr.right_laser = BEAM_LASER;
			break;

		case EQ_FRONT_MINING:
			cmdr.credits += laser_refund (cmdr.front_laser);
			cmdr.front_laser = MINING_LASER;
			break;

		case EQ_REAR_MINING:
			cmdr.credits += laser_refund (cmdr.rear_laser);
			cmdr.rear_laser = MINING_LASER;
			break;

		case EQ_LEFT_MINING:
			cmdr.credits += laser_refund (cmdr.left_laser);
			cmdr.left_laser = MINING_LASER;
			break;

		case EQ_RIGHT_MINING:
			cmdr.credits += laser_refund (cmdr.right_laser);
			cmdr.right_laser = MINING_LASER;
			break;

		case EQ_FRONT_MILITARY:
			cmdr.credits += laser_refund (cmdr.front_laser);
			cmdr.front_laser = MILITARY_LASER;
			break;

		case EQ_REAR_MILITARY:
			cmdr.credits += laser_refund (cmdr.rear_laser);
			cmdr.rear_laser = MILITARY_LASER;
			break;

		case EQ_LEFT_MILITARY:
			cmdr.credits += laser_refund (cmdr.left_laser);
			cmdr.left_laser = MILITARY_LASER;
			break;

		case EQ_RIGHT_MILITARY:
			cmdr.credits += laser_refund (cmdr.right_laser);
			cmdr.right_laser = MILITARY_LASER;
			break;
	}

	cmdr.credits -= equip_stock[hilite_item].price;
}


	static			Matrix s_matrix;

void equip_ship (void)
{

	int i;

	current_screen = SCR_EQUIP_SHIP;

	gfx_display_centre_text (10, "EQUIP SHIP", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	if (equip_stock[hilite_item].canbuy && equip_stock[hilite_item].name[0] == '+')
	{
		collapse_equip_list();
		equip_stock[hilite_item].show = 0;
		hilite_item++;
		for (i = 0; i < 5; i++)
			equip_stock[hilite_item + i].show = 1;
		
	}

	list_equip_prices();

}

