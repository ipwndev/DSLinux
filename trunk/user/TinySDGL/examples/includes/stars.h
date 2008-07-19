


void create_new_stars (void)
{
	int i;
	int nstars;
	
	nstars = witchspace ? 3 : 10;
//	if(myship.altitude <100) nstars = (310-myship.altitude*3);

	for (i = 0; i < nstars; i++)
	{
		stars[i].x = (rand255() - 128) | 8;
		stars[i].y = (rand255() - 128) | 4;
		stars[i].z = rand255() | 0x90;
	}

	warp_stars = 0;
}


void front_starfield (void)
{
	int i;
	float Q;
	float delta;
	float alpha = 0;
	float beta = 0;
	float xx,yy,zz;
	int sx;
	int sy;
	int nstars;
	
	nstars = witchspace ? 3 : 10;
//	if(myship.altitude <100) nstars = (310-myship.altitude*3);

	delta = warp_stars ? 50 : flight_speed;	
	alpha = (float)flight_roll;
	beta = (float)flight_climb;

	alpha /= 256.0;
	delta /= 2.0;
	
	for (i = 0; i < nstars; i++)
	{
		/* Plot the stars in their current locations... */

		sy = stars[i].y;
		sx = stars[i].x;
		zz = stars[i].z;

		sx += 128;
		sy += 96;

		sx *= GFX_SCALE;
		sy *= GFX_SCALE;


		int col= GFX_COL_WHITE;

		if ((!warp_stars) &&
			(sx >= GFX_VIEW_TX) && (sx <= GFX_VIEW_BX) &&
			(sy >= GFX_VIEW_TY) && (sy <= GFX_VIEW_BY))
		{

			gfx_plot_pixel (sx, sy, col);

			if (zz < 0xC0)
				gfx_plot_pixel (sx+1, sy, col);

			
			if (zz < 0x90)
			{

				gfx_plot_pixel (sx, sy+1, col);
				gfx_plot_pixel (sx+1, sy+1, col);
			}
		}


		/* Move the stars to their new locations...*/

		Q = delta / stars[i].z;

		stars[i].z -= delta;
		yy = stars[i].y + (stars[i].y * Q);
		xx = stars[i].x + (stars[i].x * Q);
		zz = stars[i].z;

		yy = yy + (xx * alpha);
		xx = xx - (yy * alpha);

/*
		tx = yy * beta;
		xx = xx + (tx * tx * 2);
*/
		yy = yy + beta;

		stars[i].y = yy;
		stars[i].x = xx;

		
		if (warp_stars)
			gfx_draw_line (sx, sy, (xx + 128) * GFX_SCALE, (yy + 96) * GFX_SCALE);
		
		sx = xx;
		sy = yy;


		if ((sx > 120) || (sx < -120) ||
			(sy > 120) || (sy < -120) || (zz < 16))
		{
			stars[i].x = (rand255() - 128) | 8;
			stars[i].y = (rand255() - 128) | 4;
			stars[i].z = rand255() | 0x90;
			continue;
		}

	}

	warp_stars = 0;
}



void rear_starfield (void)
{
	int i;
	float Q;
	float delta;
	float alpha = 0;
	float beta = 0;
	float xx,yy,zz;
	int sx,sy;
	int ex,ey;
	int nstars;
	
	nstars = witchspace ? 3 : 10; //was 12 - IN ALL VIEWS
//	if(myship.altitude <100) nstars = (310-myship.altitude*3); 

	delta = warp_stars ? 50 : flight_speed;	
	alpha = -flight_roll;
	beta = -flight_climb;

	alpha /= 256.0;
	delta /= 2.0;
	
	for (i = 0; i < nstars; i++)
	{
		/* Plot the stars in their current locations... */

		sy = stars[i].y;
		sx = stars[i].x;
		zz = stars[i].z;

		sx += 128;
		sy += 96;

		sx *= GFX_SCALE;
		sy *= GFX_SCALE;

		if ((!warp_stars) &&
			(sx >= GFX_VIEW_TX) && (sx <= GFX_VIEW_BX) &&
			(sy >= GFX_VIEW_TY) && (sy <= GFX_VIEW_BY))
		{
			gfx_plot_pixel (sx, sy, GFX_COL_WHITE);

			if (zz < 0xC0)
				gfx_plot_pixel (sx+1, sy, GFX_COL_WHITE);

			if (zz < 0x90)
			{
				gfx_plot_pixel (sx, sy+1, GFX_COL_WHITE);
				gfx_plot_pixel (sx+1, sy+1, GFX_COL_WHITE);
			}
		}


		/* Move the stars to their new locations...*/

		Q = delta / stars[i].z;

		stars[i].z += delta;
		yy = stars[i].y - (stars[i].y * Q);
		xx = stars[i].x - (stars[i].x * Q);
		zz = stars[i].z;

		yy = yy + (xx * alpha);
		xx = xx - (yy * alpha);

/*
		tx = yy * beta;
		xx = xx + (tx * tx * 2);
*/
		yy = yy + beta;
		
		if (warp_stars)
		{
			ey = yy;
			ex = xx;
			ex = (ex + 128) * GFX_SCALE;
			ey = (ey + 96) * GFX_SCALE;

			if ((sx >= GFX_VIEW_TX) && (sx <= GFX_VIEW_BX) &&
			   (sy >= GFX_VIEW_TY) && (sy <= GFX_VIEW_BY) &&
			   (ex >= GFX_VIEW_TX) && (ex <= GFX_VIEW_BX) &&
			   (ey >= GFX_VIEW_TY) && (ey <= GFX_VIEW_BY))
				gfx_draw_line (sx, sy, (xx + 128) * GFX_SCALE, (yy + 96) * GFX_SCALE);
		}
		
		stars[i].y = yy;
		stars[i].x = xx;

		if ((zz >= 300) || (abs(yy) >= 110))
		{
			stars[i].z = (rand255() & 127) + 51;
			
			if (rand255() & 1)
			{
				stars[i].x = rand255() - 128;
				stars[i].y = (rand255() & 1) ? -115 : 115;
			}
			else
			{
				stars[i].x = (rand255() & 1) ? -126 : 126;
				stars[i].y = rand255() - 128; 
			}
		}

	}

	warp_stars = 0;
}


void side_starfield (void)
{
	int i;
	int sx;
	int sy;
	int nstars;

	float delta;
	float alpha;
	float beta;
	float xx,yy,zz;
	float delt8;

	
	nstars = witchspace ? 3 : 10;
//	if(myship.altitude <100) nstars = (310-myship.altitude*3);
	
	delta = warp_stars ? 50 : flight_speed;	
	alpha = flight_roll;
	beta = flight_climb;

	if (current_screen == SCR_LEFT_VIEW)
	{
		delta = -delta;
		alpha = -alpha;
		beta = -beta;
	} 
	
	for (i = 0; i < nstars; i++)
	{
		sy = stars[i].y;
		sx = stars[i].x;
		zz = stars[i].z;

		sx += 128;
		sy += 96;

		sx *= GFX_SCALE;
		sy *= GFX_SCALE;

		if ((!warp_stars) &&
			(sx >= GFX_VIEW_TX) && (sx <= GFX_VIEW_BX) &&
			(sy >= GFX_VIEW_TY) && (sy <= GFX_VIEW_BY))
		{
			gfx_plot_pixel (sx, sy, GFX_COL_WHITE);

			if (zz < 0xC0)
				gfx_plot_pixel (sx+1, sy, GFX_COL_WHITE);

			if (zz < 0x90)
			{
				gfx_plot_pixel (sx, sy+1, GFX_COL_WHITE);
				gfx_plot_pixel (sx+1, sy+1, GFX_COL_WHITE);
			}
		}

		yy = stars[i].y;
		xx = stars[i].x;
		zz = stars[i].z;
		
		delt8 = delta / (zz / 32);
		xx = xx + delt8;

		xx += (yy * (beta / 256));		
		yy -= (xx * (beta / 256));

		xx += ((yy / 256) * (alpha / 256)) * (-xx);
		yy += ((yy / 256) * (alpha / 256)) * (yy);

		yy += alpha; 

		stars[i].y = yy;
		stars[i].x = xx;

		if (warp_stars)
			gfx_draw_line (sx, sy, (xx + 128) * GFX_SCALE, (yy + 96) * GFX_SCALE);

		
		if (abs(stars[i].x) >= 116)
		{
			stars[i].y = rand255() - 128;
			stars[i].x = (current_screen == SCR_LEFT_VIEW) ? 115 : -115;
			stars[i].z = rand255() | 8;
		}
		else if (abs(stars[i].y) >= 116)
		{
			stars[i].x = rand255() - 128;
			stars[i].y = (alpha > 0) ? -110 : 110;
			stars[i].z = rand255() | 8;
		} 
		
	}

	warp_stars = 0;
}


/*
 * When we change view, flip the stars over so they look like other stars.
 */

void flip_stars (void)
{
	int i;
	int nstars;
	int sx;
	int sy;
	
	nstars = witchspace ? 3 : 10;
//	if(myship.altitude <100) nstars = (310-myship.altitude*3);

	for (i = 0; i < nstars; i++)
	{
		sy = stars[i].y;
		sx = stars[i].x;
		stars[i].x = sy;
		stars[i].y = sx;
	}


	for(i= 0; i <20; i++) 
		mm_current[i]= 0;
}


void update_starfield (void)
{


	switch (current_screen)
	{
		case SCR_FRONT_VIEW:
		case SCR_INTRO_ONE:
		case SCR_INTRO_TWO:
		case SCR_ESCAPE_POD:
			front_starfield();
			break;
		
		case SCR_REAR_VIEW:
		case SCR_GAME_OVER:
		case SCR_ESCAPE_NOVA:
			rear_starfield();
			break;
		
		case SCR_LEFT_VIEW:
		case SCR_RIGHT_VIEW:
			side_starfield();
			break;
	}
}
