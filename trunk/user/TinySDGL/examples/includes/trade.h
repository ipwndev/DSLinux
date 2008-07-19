

void generate_stock_market (void);
void set_stock_quantities(int *quant);
int total_cargo (void);
void scoop_item (int un);



// - - - - - - - - - - -  F I L E   E N D - - - - - - - - - - - 




/*
 * Generate the Elite stock market.
 * The prices and quantities are affected by the planet's economy.
 * There is also a slight amount of randomness added in.
 * The random value is changed each time we hyperspace.
 */


void generate_stock_market (void)
{
	int quant;
	int price;
	int i;

	for (i = 0; i < NO_OF_STOCK_ITEMS; i++)
	{
		price  = stock_market[i].base_price;								/* Start with the base price	*/
		price += cmdr.market_rnd & stock_market[i].mask;					/* Add in a random amount		*/
		price += current_planet_data.economy * stock_market[i].eco_adjust;	/* Adjust for planet economy	*/
		price &= 255;														/* Only need bottom 8 bits		*/

		quant  = stock_market[i].base_quantity;								/* Start with the base quantity */
		quant += cmdr.market_rnd & stock_market[i].mask;					/* Add in a random amount		*/
		quant -= current_planet_data.economy * stock_market[i].eco_adjust;	/* Adjust for planet economy	*/
		quant &= 255;														/* Only need bottom 8 bits		*/

		if (quant > 127)	/* In an 8-bit environment '>127' would be negative */
			quant = 0;		/* So we set it to a minimum of zero. */

		quant &= 63;		/* Quantities range from 0..63 */

		stock_market[i].current_price = price * 4;
		stock_market[i].current_quantity = quant;
	}


	/* Alien Items are never available for purchase... */

	stock_market[ALIEN_ITEMS_IDX].current_quantity = 0;
}



void set_stock_quantities(int *quant)
{
	int i;

	for (i = 0; i < NO_OF_STOCK_ITEMS; i++)
		stock_market[i].current_quantity = quant[i];

	stock_market[ALIEN_ITEMS_IDX].current_quantity = 0;
}

 


int total_cargo (void)
{
	int i;
	int cargo_held;

	cargo_held = 0;
	for (i = 0; i < 17; i++)
	{
		if ((cmdr.current_cargo[i] > 0) &&
			(stock_market[i].units == TONNES))
		{
			cargo_held += cmdr.current_cargo[i];
		}
	}

	return cargo_held;
}


void scoop_item (int un)
{
	int type;
	int trade;

	if(universe[un].flags & FLG_DEAD)
		return;
	
	type = universe[un].type;
	
	if(type == SHIP_MISSILE)
		return;

	if ((cmdr.fuel_scoop == 0) || (universe[un].location.y >= 0) ||
		(total_cargo() == cmdr.cargo_capacity))
	{

		if(universe[un].energy <100)
		{
			universe[un].energy-= 25;
			damage_ship (30, universe[un].location.z > 0); //damage with fuel scoop
		}

		return;
	}


	if(type == SHIP_ESCAPE_CAPSULE && zpino ==1)
	{
		message_count= 0;
		info_message ("Zurid Pino captured, bring him in for interogation.", 100);

		zpino=100;
	}


	if(type == SHIP_ESCAPE_CAPSULE && cougar_pod ==1)
	{

		cmdr.credits+= 10000;
	
		message_count= 0;
		info_message ("Pirates captured. Well done, 1000.0 Cr transfered.", 100);

		cougar_pod= 100;
		cmdr.score += 1000; // Pirates captured alive	

	}



	if (type == SHIP_CARGO)
	{
	
		trade= universe[un].bravery;
		cmdr.current_cargo[trade]++; 

		if(trade ==6 && rand255()<30) //6 = narcotics mega weed
			info_message ("M E G A - W E E D", 100);
		else
			info_message (stock_market[trade].name, 40);

		remove_ship (un);
		return;					
	}


	if (ship_list[type]->scoop_type != 0)
	{

		trade = ship_list[type]->scoop_type + 1;
		cmdr.current_cargo[trade]++;
		info_message (stock_market[trade].name, 40);
		remove_ship (un);
		return;					
	}

	
	if(universe[un].energy <100)
	{
		universe[un].energy-= 25;
		damage_ship (10, universe[un].location.z > 0); //damage with fuel scoop
	}
}


