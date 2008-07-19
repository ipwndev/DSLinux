

void gfx_draw_colour_line (int x1, int y1, int x2, int y2, int line_colour);


// - - - - - - - - - - -  F I L E   E N D - - - - - - - - - - - 




#define MAX_POLYS	5000

static int start_poly;
static int total_polys;

struct poly_data
{
	int z;
	int no_points;
	int face_colour;
	int point_list[16];
	int next;
};

static struct poly_data poly_chain[MAX_POLYS];


/*
 * Draw the cross hairs at the specified position.
 */

void draw_cross (int cx, int cy)
{

	if (current_screen == SCR_SHORT_RANGE)
	{
		if (cx < 1)	cx = 1;
		if (cx > 510) cx = 510;
		if (cy < 37) cy = 37;
		if (cy > 383) cy = 383;

		gfx_draw_colour_line (cx - 16, cy, cx + 16, cy, GFX_COL_RED);
		gfx_draw_colour_line (cx, cy - 16, cx, cy + 16, GFX_COL_RED);
	}
	else	
	if (current_screen == SCR_GALACTIC_CHART)
	{
		if (cross_x < 1) cross_x = 1;
		if (cross_x > 510)cross_x = 510;
		if (cross_y < 37) cross_y = 37;
		if (cross_y > 293) cross_y = 293;

		gfx_draw_colour_line (cx - 8, cy, cx + 8, cy, GFX_COL_RED);
		gfx_draw_colour_line (cx, cy - 8, cx, cy + 8, GFX_COL_RED);
	}
}



void gfx_set_color(int col)
{

	switch(col){


		case GFX_COL_BLACK: glColor3f(0,0,0); break;
		case GFX_COL_WHITE: glColor3f(1,1,1); break;
		case GFX_COL_WHITE_2: glColor3f(0.9,1,0.7); break;

		case GFX_COL_RED_1: glColor3f(1,0,0); break;
		case GFX_COL_RED_2: glColor3f(0.7,0,0); break;
		case GFX_COL_RED_3: glColor3f(0.5,0,0); break;
		case GFX_COL_RED_4: glColor3f(0.3,0,0); break;

		case GFX_COL_CYAN: glColor3f(0,1,1); break;

		case GFX_COL_GREY_0: glColor3f(0.55,0.55,0.55); break;
		case GFX_COL_GREY_1: glColor3f(0.5,0.5,0.5); break;
		case GFX_COL_GREY_2: glColor3f(0.45,0.45,0.45); break;
		case GFX_COL_GREY_3: glColor3f(0.4,0.4,0.4); break;
		case GFX_COL_GREY_4: glColor3f(0.25,0.1,0.2); break;
		case GFX_COL_GREY_5: glColor3f(0.2,0.1,0.3); break;
		case GFX_COL_GREY_6: glColor3f(0.2,0.2,0.2); break;

		case GFX_COL_BLUE_0: glColor3f(0,0,0.9); break;
		case GFX_COL_BLUE_1: glColor3f(0,0,1); break;
		case GFX_COL_BLUE_2: glColor3f(0,0,0.7); break;
		case GFX_COL_BLUE_3: glColor3f(0,0.2,0.5); break;
		case GFX_COL_BLUE_4: glColor3f(0,0.7,1); break;
		case GFX_COL_BLUE_5: glColor3f(0.1,0.2,0.3); break;
		case GFX_COL_BLUE_6: glColor3f(0.1,0.18,0.28); break;
		
		case GFX_COL_YELLOW_1: glColor3f(1,1,0); break;
		case GFX_COL_YELLOW_2: glColor3f(1,0.9,0); break;
		case GFX_COL_YELLOW_3: glColor3f(0.8,0.9,0); break;
		case GFX_COL_YELLOW_4: glColor3f(0.7,8,0); break;
		case GFX_COL_YELLOW_5: glColor3f(0.8,0.8,0); break;
		

		case GFX_COL_ORANGE_1: glColor3f(1,0.7,0); break;
		case GFX_COL_ORANGE_2: glColor3f(0.8,0.5,0); break;
		case GFX_COL_ORANGE_3: glColor3f(0.75,0.45,0); break;
		case GFX_COL_ORANGE_4: glColor3f(0.3,0.1,0); break;
		case GFX_COL_ORANGE_5: glColor3f(1,0.5,0.3); break;

		
		case GFX_COL_GREEN_1: glColor3f(0,1,0); break;
		case GFX_COL_GREEN_2: glColor3f(0,0.7,0); break;
		case GFX_COL_GREEN_3: glColor3f(0,0.4,0); break;
		case GFX_COL_GREEN_4: glColor3f(0.1,0.2,0.2); break;
		case GFX_COL_GREEN_0: glColor3f(0,0.8,0); break;


		case GFX_COL_PINK_1: glColor3f(1,0,1); break;
		case GFX_COL_PINK_2: glColor3f(0.4,0.2,0.5); break;
		case GFX_COL_PINK_3: glColor3f(0.7,0.5,1); break;

	}


}




void gfx_plot_pixel (int x, int y, int col)
{

	gfx_set_color(col);
	glBegin(GL_POINTS);

		glVertex2f(x + GFX_X_OFFSET, y + GFX_Y_OFFSET);
	glEnd();
}


void gfx_draw_filled_circle (int cx, int cy, int radius, int col)
{

	gfx_set_color(col);

	if(radius<1) radius = 1;

	int i;
	float angle;

	int nP= 20+(radius)/5;
	if(nP>50) nP=50;
	
	glBegin(GL_TRIANGLE_FAN);

		for(i= 0; i< nP; i++)
		{
			angle= 2*PI*i/nP;
			glVertex2f(cx + GFX_X_OFFSET+ radius*cos(angle), cy + GFX_Y_OFFSET+ radius*sin(angle));
		}
	glEnd();
}


void gfx_draw_circle (int cx, int cy, int radius, int col)
{

	gfx_set_color(col);

	if(radius<1) radius = 0.5;

	int i;
	float angle;

	int nP= 10 + (radius)/5;
	if(nP>50) nP=50;
	
	glBegin(GL_LINE_LOOP);

		for(i= 0; i< nP; i++)
		{
			angle= 2*PI*i/nP;
			glVertex2f(cx + GFX_X_OFFSET+ radius*cos(angle), cy + GFX_Y_OFFSET+ radius*sin(angle));
		}
	glEnd();

}



void gfx_draw_line (int x1, int y1, int x2, int y2)
{
	glColor3f(1,1,1);

	glBegin(GL_LINES);
		glVertex2f(x1+ GFX_X_OFFSET, y1+ GFX_Y_OFFSET);
		glVertex2f(x2+ GFX_X_OFFSET, y2+ GFX_Y_OFFSET);
	glEnd();
}



void gfx_draw_colour_line (int x1, int y1, int x2, int y2, int col)
{
	gfx_set_color(col);
	glBegin(GL_LINES);
		glVertex2f(x1+ GFX_X_OFFSET, y1+ GFX_Y_OFFSET);
		glVertex2f(x2+ GFX_X_OFFSET, y2+ GFX_Y_OFFSET);
	glEnd();
}





void gfx_display_text (int x, int y, char *txt)
{
	glColor3f(1,1,1);
	txtOut( x + GFX_X_OFFSET, y  + GFX_Y_OFFSET, txt, 2);
}


void gfx_display_colour_text (int x, int y, char *txt, int col)
{
	gfx_set_color(col);
	txtOut( x + GFX_X_OFFSET, y + GFX_Y_OFFSET, txt, 2);
}



void gfx_display_centre_text (int y, char *str, int psize, int col)
{
	gfx_set_color(col);
	txtOut( 10 + GFX_X_OFFSET, y * GFX_SCALE/2 + GFX_Y_OFFSET, str, 3);

}




void gfx_clear_display(void)
{
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void gfx_clear_text_area (void)
{
//	rectfill (gfx_screen, GFX_X_OFFSET + 1, GFX_Y_OFFSET + 340, 510 + GFX_X_OFFSET, 383 + GFX_Y_OFFSET, GFX_COL_BLACK);
return;

	int x1= GFX_X_OFFSET+1;
	int y1= GFX_Y_OFFSET+340;
	int x2= GFX_X_OFFSET+510;
	int y2= GFX_Y_OFFSET+383;

	glColor3f(1,0,0);
	glBegin(GL_QUADS);
		glVertex2f(x1, y1);

		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
	glEnd();

}



void gfx_draw_rectangle (int tx, int ty, int bx, int by, int col)
{

	int x1= tx + GFX_X_OFFSET;
	int y1= ty + GFX_Y_OFFSET;
	int x2= bx + GFX_X_OFFSET;
	int y2= by + GFX_Y_OFFSET;

	gfx_set_color(col);



	glBegin(GL_QUADS);
		glVertex2f(x1, y1);

		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
	glEnd();

}




void gfx_display_pretty_text (int tx, int ty, int bx, int by, char *txt)
{

	glColor3f(0,1,1);

	char strbuf[100];
	char *str;
	char *bptr;
	int len;
	int pos;
	int maxlen;
	
	maxlen = (bx - tx) / 8;

	str = txt;	
	len = strlen(txt);
	
	while (len > 0)
	{
		pos = maxlen;
		if (pos > len)
			pos = len;

		while ((str[pos] != ' ') && (str[pos] != ',') &&
			   (str[pos] != '.') && (str[pos] != '\0'))
		{
			pos--;
		}

		len = len - pos - 1;
	
		for (bptr = strbuf; pos >= 0; pos--)
			*bptr++ = *str++;

		*bptr = '\0';


		txtOut( tx + GFX_X_OFFSET, ty  + GFX_Y_OFFSET, strbuf, 2);

		ty += (8 * GFX_SCALE);
	}
}



void gfx_draw_scanner (void)
{

	int i;
	int x1= GFX_X_OFFSET;
	int y1= GFX_Y_OFFSET+394;
	int x2= x1 + 511;
	int y2= y1 + 119;

	glColor3f(1,0,0);
	glBegin(GL_QUADS);
		glVertex2f(x2-96, y1+3*17);
		glVertex2f(x2-30, y1+3*17);
		glVertex2f(x2-30, y2);
		glVertex2f(x2-96, y2);
	glEnd();

	glColor3f(1,0.7, 0);
	glBegin(GL_LINE_STRIP);

		glVertex2f(x2-96, y2);
		glVertex2f(x2-96, y1);

		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
		glVertex2f(x1, y1);
		glVertex2f(x1+94, y1);
		glVertex2f(x1+94, y2);

	glEnd();


	glBegin(GL_LINES);

		glVertex2f(x1+30, y1);
		glVertex2f(x1+30, y2);
	
		glVertex2f(x2-31, y1);
		glVertex2f(x2-31, y2);
	
		for(i=0;i<7;i++){

			glVertex2f(x1, y1+i*17);
			glVertex2f(x1+94, y1+i*17);

			glVertex2f(x2, y1+i*17);
			glVertex2f(x2-96, y1+i*17);

		}

	glEnd();




//not working yet glEnable(GL_LINE_STIPPLE);
//glLineStipple (1, 0x8484); //dotted 0x0101 --- glLineStipple (1, 0x00FF); //dashed

	float pi180= PI/180; 

	glColor3f(0,0.3,0);

	glBegin(GL_LINE_STRIP);
		glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(30* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(30* pi180));
		glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(300* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(300* pi180));
		glVertex2f(scanner_cx+GFX_X_OFFSET, scanner_cy+GFX_Y_OFFSET);
		glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(240* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(240* pi180));
		glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(150* PI/180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(150* pi180));
	
	glEnd();



	glBegin(GL_LINES);

		glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(180* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(180* pi180));
		glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(360* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(360* pi180));


	glColor3f(0,0.2,0.3);

		if(scanner_zoom==1){
			glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(150* PI/180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(150* pi180));
			glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(30* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(30* pi180));
		}

		if(scanner_zoom==1){

			glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(255* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(255* pi180));
			glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(120* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(120* pi180));

			glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(285* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(285* pi180));
			glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(60* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(60* pi180));

			glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(210* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(210* pi180));
			glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(330* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(330* pi180));

			glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(270* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(270* pi180));
			glVertex2f(scanner_cx+GFX_X_OFFSET+ scanner_w*cos(90* pi180), scanner_cy+GFX_Y_OFFSET+ scanner_h*sin(90* pi180));
		}

	glEnd();


	int nP= 20;
	float angle;
	glBegin(GL_LINE_LOOP);

		for(i= 0; i< nP; i++)
		{
			angle= 2*PI*i/nP;
			glVertex2f(scanner_cx+GFX_X_OFFSET + scanner_w*cos(angle), scanner_cy+GFX_Y_OFFSET + scanner_h*sin(angle));
		}
	glEnd();

//not working yet glDisable(GL_LINE_STIPPLE);



//scanner_zoom

		sprintf(str, "Zoom x%d", scanner_zoom);
		if(scanner_zoom ==2)
			gfx_display_colour_text(222, 400, str, GFX_COL_GREEN_3); 
		else
			gfx_display_colour_text(222, 400, str, GFX_COL_BLUE_5); 





}


void gfx_ScissorHack(void)
{
//this has to be replaced by adding glScissors function to TinySDGL

int x1= GFX_X_OFFSET;
int y1= GFX_Y_OFFSET;
int x2= x1 + 511;
int y2= y1 + 384;


glColor3f(0, 0, 0);
glBegin(GL_QUADS);

	glVertex2f(0, y2);
	glVertex2f(800, y2);
	glVertex2f(800, 600);
	glVertex2f(0, 600);
	
	glVertex2f(0, y2);
	glVertex2f(x1, y2);
	glVertex2f(x1, 0);
	glVertex2f(0, 0);
	
	glVertex2f(x2, y2);
	glVertex2f(800, y2);
	glVertex2f(800, 0);
	glVertex2f(x2, 0);
	
	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x2, 0);
	glVertex2f(x1, 0);
glEnd();

}



void gfx_set_clip_region (int tx, int ty, int bx, int by)
{
	//set_clip (gfx_screen, tx + GFX_X_OFFSET, tx + GFX_X_OFFSET, bx + GFX_X_OFFSET, by + GFX_Y_OFFSET);

	//glScissor(tx + GFX_X_OFFSET, ty + GFX_Y_OFFSET, bx + GFX_X_OFFSET, by + GFX_Y_OFFSET);

}





void gfx_draw_sprite (int sprite_no, int x, int y)
{
//	draw_sprite (gfx_screen, sprite_bmp, x + GFX_X_OFFSET, y + GFX_Y_OFFSET);
}








 //****************************** 3-D

#define MAX(x,y) (((x) > (y)) ? (x) : (y))

static struct point point_list[100];


/*
 * The following routine is used to draw a wireframe represtation of a ship.
 *
 * caveat: it is a work in progress.
 * A number of features (such as not showing detail at distance) have not yet been implemented.
 *
 */


void draw_wireframe_ship (struct univ_object *univ)
{
	Matrix trans_mat;
	int i;
	int sx,sy,ex,ey;
	float rx,ry,rz;
	int visible[32];
	Vector vec;
	Vector camera_vec;
	float cos_angle;
	float tmp;
	struct ship_face_normal *ship_norm;
	int num_faces;
	struct ship_data *ship;
	int lasv;

	ship = ship_list[univ->type];
	
	for (i = 0; i < 3; i++)
		trans_mat[i] = univ->rotmat[i];
		
	camera_vec = univ->location;
	mult_vector (&camera_vec, trans_mat);
	camera_vec = unit_vector (&camera_vec);
	
	num_faces = ship->num_faces;
	
	for (i = 0; i < num_faces; i++)
	{
		ship_norm = ship->normals;

		vec.x = ship_norm[i].x;
		vec.y = ship_norm[i].y;
		vec.z = ship_norm[i].z;

		if ((vec.x == 0) && (vec.y == 0) && (vec.z == 0))
			visible[i] = 1;
		else
		{
			vec = unit_vector (&vec);
			cos_angle = vector_dot_product (&vec, &camera_vec);
			visible[i] = (cos_angle < -0.2);
		}
	}

	tmp = trans_mat[0].y;
	trans_mat[0].y = trans_mat[1].x;
	trans_mat[1].x = tmp;

	tmp = trans_mat[0].z;
	trans_mat[0].z = trans_mat[2].x;
	trans_mat[2].x = tmp;

	tmp = trans_mat[1].z;
	trans_mat[1].z = trans_mat[2].y;
	trans_mat[2].y = tmp;

	for (i = 0; i < ship->num_points; i++)
	{
		vec.x = ship->points[i].x;
		vec.y = ship->points[i].y;
		vec.z = ship->points[i].z;

		mult_vector (&vec, trans_mat);

		rx = vec.x + univ->location.x;
		ry = vec.y + univ->location.y;
		rz = vec.z + univ->location.z;

		sx = (rx * 256) / rz;
		sy = (ry * 256) / rz;

		sy = -sy;

		sx += 128;
		sy += 96;

		sx *= GFX_SCALE;
		sy *= GFX_SCALE;

		point_list[i].x = sx;
		point_list[i].y = sy;

	}

	int ccol= GFX_COL_WHITE;

	if(univ->type==SHIP_COUGAR)
	{
	

		if(univ->energy > (ship_list[univ->type]->energy/2))
		{
				ccol= GFX_COL_GREY_5;
		}
		else
		{
			if(rand255()>210) 
				ccol= GFX_COL_GREY_4;
			else
				ccol= GFX_COL_GREY_5;
		}
	}


	if ((univ->type == SHIP_CORIOLIS) || (univ->type == SHIP_DODEC))
		ccol= GFX_COL_WHITE_2;

//draw ship	
	for (i = 0; i < ship->num_lines; i++)
	{
		if (visible[ship->lines[i].face1] ||
			visible[ship->lines[i].face2])
		{
			sx = point_list[ship->lines[i].start_point].x;
			sy = point_list[ship->lines[i].start_point].y;

			ex = point_list[ship->lines[i].end_point].x;
			ey = point_list[ship->lines[i].end_point].y;

			gfx_draw_colour_line (sx, sy, ex, ey, ccol);
		}
	}


//draw lasers

	if (univ->flags & FLG_FIRING)
	{
		lasv = ship_list[univ->type]->front_laser;

		int col = GFX_COL_WHITE;
		int l= ship_list[univ->type]->laser_strength;
		
		if(l < 9) col= GFX_COL_ORANGE_1;
		else
		if(l < 12) col= GFX_COL_GREEN_1;
		else
			col= GFX_COL_RED_1;

		if(univ->type==SHIP_VIPER && l == 12) col= GFX_COL_BLUE_4;
		if(l > 20) col= GFX_COL_PINK_1;

		gfx_draw_colour_line (point_list[lasv].x, point_list[lasv].y,
					   univ->location.x > 0 ? 0 : 511, rand255() * 2, col);
	}




//*** Razaar gadget
		
	if(razaar_refugees == 500 && missile_target == univ_obj_currently_being_updated)
	{
		sx = (univ->location.x * 256) / univ->location.z;
		sy = -(univ->location.y * 256) / univ->location.z;

		sx += 128;
		sy += 96;

		sx *= GFX_SCALE;
		sy *= GFX_SCALE;


		gfx_draw_colour_line(sx-10, sy, sx+10, sy, GFX_COL_RED_1);
		gfx_draw_colour_line(sx, sy+10, sx, sy-10, GFX_COL_RED_1);
	}


}




void gfx_render_polygon (int num_points, int *point_list, int face_colour, int zavg)
{
	int i;
	int x;
	int nx;
	
	if (total_polys == MAX_POLYS)
		return;

	x = total_polys;
	total_polys++;
	
	poly_chain[x].no_points = num_points;
	poly_chain[x].face_colour = face_colour;
	poly_chain[x].z = zavg;
	poly_chain[x].next = -1;

	for (i = 0; i < 16; i++)
		poly_chain[x].point_list[i] = point_list[i];				

	if (x == 0)
		return;

	if (zavg > poly_chain[start_poly].z)
	{
		poly_chain[x].next = start_poly;
		start_poly = x;
		return;
	} 	

	for (i = start_poly; poly_chain[i].next != -1; i = poly_chain[i].next)
	{
		nx = poly_chain[i].next;
		
		if (zavg > poly_chain[nx].z)
		{
			poly_chain[i].next = x;
			poly_chain[x].next = nx;
			return;
		}
	}	
	
	poly_chain[i].next = x;


}

 


void draw_laser_3d(int sh, int tg)
{

	if ((current_screen != SCR_FRONT_VIEW) && (current_screen != SCR_REAR_VIEW) && 
		(current_screen != SCR_LEFT_VIEW) && (current_screen != SCR_RIGHT_VIEW) &&
		(current_screen != SCR_INTRO_ONE) && (current_screen != SCR_INTRO_TWO) &&
		(current_screen != SCR_GAME_OVER) && (current_screen != SCR_ESCAPE_POD)
		&& (current_screen != SCR_ESCAPE_NOVA))
		return;


	struct univ_object ship, target;

	ship = universe[sh];
	target = universe[tg];

	switch_to_view (&target);
	switch_to_view (&ship);

	if(target.location.z <0 && ship.location.z <0) return;

	int sx = (ship.location.x * 256) / ship.location.z;
	int sy = -(ship.location.y * 256) / ship.location.z;

	int tx = (target.location.x * 256) / target.location.z;
	int ty = -(target.location.y * 256) / target.location.z;

	if((target.location.z <0 && ship.location.z <0) || 
		abs(tx)<1 || abs(ty)<1 || abs(sx)<1 || abs(sy)<1) 
		return;
		
	if(target.location.z <0 && ship.location.z >0)
	{
		tx*= -1000;
		ty*= -1000;
		
	}
	else
	if(target.location.z >0 && ship.location.z <0)
	{
		sx*= -1000;
		sy*= -1000;
	}


	int lasv = ship_list[ship.type]->front_laser;

	int col = GFX_COL_WHITE;
	int l= ship_list[ship.type]->laser_strength;
	
	if(l < 10) col= GFX_COL_ORANGE_1;
	else
	if(l < 15) col= GFX_COL_GREEN_1;
	else
		col= GFX_COL_RED_1;

	if(l > 20) col= GFX_COL_PINK_1;

	if(ship.type==SHIP_VIPER) col= GFX_COL_BLUE_4;
	if(ship.type==SHIP_THARGOID) col= GFX_COL_RED_1;


	sx += 128;
	sy += 96;
	sx *= GFX_SCALE;
	sy *= GFX_SCALE;

	tx += 128;
	ty += 96;
	tx *= GFX_SCALE;
	ty *= GFX_SCALE;

	gfx_draw_colour_line(sx, sy, tx-15+(rand255() &31), ty-15+(rand255() &31), col);

}



void draw_solid_ship (struct univ_object *univ)
{
	int i,j;
	int sx,sy;
	float rx,ry,rz;
	struct vector vec;
	struct vector camera_vec;
	float tmp;
	struct ship_face *face_data;
	int num_faces;
	int num_points;
	int poly_list[16];
	int zavg;
	struct ship_solid *solid_data;
	struct ship_data *ship;
	Matrix trans_mat;
	int lasv;

	solid_data = &ship_solids[univ->type];
	ship = ship_list[univ->type];
	
	for (i = 0; i < 3; i++)
		trans_mat[i] = univ->rotmat[i];
		
	camera_vec = univ->location;
	mult_vector (&camera_vec, trans_mat);
	camera_vec = unit_vector (&camera_vec);

	num_faces = solid_data->num_faces;
	face_data = solid_data->face_data;


	tmp = trans_mat[0].y;
	trans_mat[0].y = trans_mat[1].x;
	trans_mat[1].x = tmp;

	tmp = trans_mat[0].z;
	trans_mat[0].z = trans_mat[2].x;
	trans_mat[2].x = tmp;

	tmp = trans_mat[1].z;
	trans_mat[1].z = trans_mat[2].y;
	trans_mat[2].y = tmp;


	for (i = 0; i < ship->num_points; i++)
	{
		vec.x = ship->points[i].x;
		vec.y = ship->points[i].y;
		vec.z = ship->points[i].z;

		mult_vector (&vec, trans_mat);

		rx = vec.x + univ->location.x;
		ry = vec.y + univ->location.y;
		rz = vec.z + univ->location.z;

		if (rz <= 0)
			rz = 1;
		
		sx = (rx * 256) / rz;
		sy = (ry * 256) / rz;

		sy = -sy;

		sx += 128;
		sy += 96;

		sx *= GFX_SCALE;
		sy *= GFX_SCALE;

		point_list[i].x = sx;
		point_list[i].y = sy;
		point_list[i].z = rz;
		
	}

	for (i = 0; i < num_faces; i++)
	{
		if (((point_list[face_data[i].p1].x - point_list[face_data[i].p2].x) * 
		     (point_list[face_data[i].p3].y - point_list[face_data[i].p2].y) -
			 (point_list[face_data[i].p1].y - point_list[face_data[i].p2].y) *
			 (point_list[face_data[i].p3].x - point_list[face_data[i].p2].x)) <= 0)
		{
			num_points = face_data[i].points;



			poly_list[0] = point_list[face_data[i].p1].x;
			poly_list[1] = point_list[face_data[i].p1].y;
			zavg = point_list[face_data[i].p1].z;

			poly_list[2] = point_list[face_data[i].p2].x;
			poly_list[3] = point_list[face_data[i].p2].y;
			zavg = MAX(zavg,point_list[face_data[i].p2].z);

			if (num_points > 2)
			{
				poly_list[4] = point_list[face_data[i].p3].x;
				poly_list[5] = point_list[face_data[i].p3].y;
				zavg = MAX(zavg,point_list[face_data[i].p3].z);
			}

			if (num_points > 3)
			{
				poly_list[6] = point_list[face_data[i].p4].x;
				poly_list[7] = point_list[face_data[i].p4].y;
				zavg = MAX(zavg,point_list[face_data[i].p4].z);
			}
			if (num_points > 4)
			{
				poly_list[8] = point_list[face_data[i].p5].x;
				poly_list[9] = point_list[face_data[i].p5].y;
				zavg = MAX(zavg,point_list[face_data[i].p5].z);
			}
			if (num_points > 5)
			{
				poly_list[10] = point_list[face_data[i].p6].x;
				poly_list[11] = point_list[face_data[i].p6].y;
				zavg = MAX(zavg,point_list[face_data[i].p6].z);
			}
			if (num_points > 6)
			{
				poly_list[12] = point_list[face_data[i].p7].x;
				poly_list[13] = point_list[face_data[i].p7].y;
				zavg = MAX(zavg,point_list[face_data[i].p7].z);
			}
			if (num_points > 7)
			{
				poly_list[14] = point_list[face_data[i].p8].x;
				poly_list[15] = point_list[face_data[i].p8].y;
				zavg = MAX(zavg,point_list[face_data[i].p8].z);
			}

			
			gfx_render_polygon (face_data[i].points, poly_list, face_data[i].colour, zavg);

			
		}
	}

//draw lasers

	if (univ->flags & FLG_FIRING)
	{
		lasv = ship_list[univ->type]->front_laser;

		int col = GFX_COL_WHITE;
		int l= ship_list[univ->type]->laser_strength;
		
		if(l < 10) col= GFX_COL_ORANGE_1;
		else
		if(l < 15) col= GFX_COL_GREEN_1;
		else
			col= GFX_COL_RED_1;

		if(l > 20) col= GFX_COL_PINK_1;

		if(univ->type==SHIP_VIPER) col= GFX_COL_BLUE_4;
		if(univ->type==SHIP_THARGOID) col= GFX_COL_RED_1;

		gfx_draw_colour_line (point_list[lasv].x, point_list[lasv].y,
					   univ->location.x > 0 ? 0 : 511, rand255() * 2, col);
	}




//*** Razaar gadget
		
	if( current_screen ==SCR_FRONT_VIEW &&
		razaar_refugees ==500 && missile_target ==univ_obj_currently_being_updated )
	{
		sx = (univ->location.x * 256) / univ->location.z;
		sy = -(univ->location.y * 256) / univ->location.z;

		sx += 128;
		sy += 96;

		sx *= GFX_SCALE;
		sy *= GFX_SCALE;

		razaar_g_x= sx; 
		razaar_g_y= sy;
	}



//*** missile trail

	i= univ_obj_currently_being_updated;
	int imt, ox, oy, count=0;

	int end= FALSE;
	if(univ->type ==SHIP_MISSILE && mm_t[i] ==TRUE)
	{

		for(imt=4; imt>0; imt--)
		{
			mm_trailX[i][imt]= mm_trailX[i][imt-1];
			mm_trailY[i][imt]= mm_trailY[i][imt-1];
			mm_trailZ[i][imt]= mm_trailZ[i][imt-1];
		}
		mm_trailX[i][0]= univ->location.x +rand255()/10;
		mm_trailY[i][0]= univ->location.y +rand255()/10;
		mm_trailZ[i][0]= univ->location.z +rand255()/10;

		mm_current[i]++;
		if(mm_current[i]<5) return;


		for(imt=4; imt>0; imt--)
		{
			ox= sx;
			oy= sy;


			sx = (mm_trailX[i][imt-1] * 256) / mm_trailZ[i][imt-1];
			sy = -(mm_trailY[i][imt-1] * 256) / mm_trailZ[i][imt-1];

			sx += 128;
			sy += 96;

			sx *= GFX_SCALE;
			sy *= GFX_SCALE;


			ox = (mm_trailX[i][imt] * 256) / mm_trailZ[i][imt];
			oy = -(mm_trailY[i][imt] * 256) / mm_trailZ[i][imt];

			ox += 128;
			oy += 96;

			ox *= GFX_SCALE;
			oy *= GFX_SCALE;
			


			if(imt > 3) 
			{
				gfx_draw_colour_line(sx, sy, ox, oy, GFX_COL_GREY_4);
				gfx_draw_colour_line(sx+rand255()/70, sy+rand255()/70, ox+rand255()/70, oy+rand255()/70, GFX_COL_BLUE_5);
			
			}
			else
			if(imt > 2) 
			{
				gfx_draw_colour_line(sx, sy, ox, oy, GFX_COL_ORANGE_1);
				gfx_draw_colour_line(sx+rand255()/70, sy+rand255()/90, ox+rand255()/90, oy+rand255()/90, GFX_COL_ORANGE_2);
			
			}
			else
			if(imt> 1) 
			{
			
				gfx_draw_colour_line(sx, sy, ox, oy, GFX_COL_YELLOW_1);
				gfx_draw_colour_line(sx+rand255()/70, sy+rand255()/120, ox+rand255()/120, oy+rand255()/120, GFX_COL_YELLOW_2);

			}

		}	


	
	}

}



void gfx_init_render()
{
	start_poly = 0;
	total_polys = 0;
	draw_laser= FALSE;
//	glEnable(GL_SCISSOR_TEST);

}


void gfx_finish_render (void)
{
	int num_points;
	int *pl;
	int i, j, n;
	int col;
	
	if (total_polys >0) 		
	{
	
		for (i = start_poly; i != -1; i = poly_chain[i].next)
		{
			num_points = poly_chain[i].no_points;
			pl = poly_chain[i].point_list;
			col = poly_chain[i].face_colour;

			if (num_points == 2)
			{
				gfx_draw_colour_line (pl[0], pl[1], pl[2], pl[3], col);
				continue;
			}
			


			gfx_set_color(col);
			n= 0;
			glBegin(GL_POLYGON);
				for(j=0; j< num_points; j++)
				{

					glVertex2f(pl[n]+GFX_X_OFFSET, pl[n+1]+GFX_Y_OFFSET);
					n+= 2;
				}
			glEnd();


		};
	}


	if (draw_laser)
	{

		int col= rand255();

		if(col < 100) col= GFX_COL_ORANGE_1;
		else
			col= GFX_COL_ORANGE_5;

		gfx_draw_filled_circle(laser_x, laser_y, rand255()/100+5, col);
	}


	if(current_screen ==SCR_FRONT_VIEW && razaar_refugees == 500 && missile_target !=MISSILE_UNARMED)
	{
		gfx_draw_colour_line(razaar_g_x-10, razaar_g_y, razaar_g_x+10, razaar_g_y, GFX_COL_RED_1);
		gfx_draw_colour_line(razaar_g_x, razaar_g_y+10, razaar_g_x, razaar_g_y-10, GFX_COL_RED_1);
	}

//	glDisable(GL_SCISSOR_TEST);
}






















void draw_planet (struct univ_object *planet)
{

	int col;
	int x,y;
	int radius;
	unsigned char pl_f=0;

	x = (planet->location.x * 256) / planet->location.z;
	y = -(planet->location.y * 256) / planet->location.z;
	
	x += 128;
	y += 96;

	
	x *= GFX_SCALE;
	y *= GFX_SCALE;
	
	radius = 6291456 / planet->distance;
	radius *= GFX_SCALE;

	pl_f= docked_planet.f;

	if(pl_f<50) col= GFX_COL_RED_4;
	else
	if(pl_f<100) col= GFX_COL_BLUE_5;
	else
	if(pl_f<150) col= GFX_COL_GREEN_4; 
	else
	if(pl_f<200) col= GFX_COL_GREY_4;
	else
		col= GFX_COL_ORANGE_4;
	
	if( docked_planet.d == 150 && docked_planet.b == 130 && 
		cmdr.galaxy_number == 2 && gal_fam ==-500) //starving zaisedan
	{
		gfx_draw_filled_circle (x, y, radius+radius/10, GFX_COL_PINK_2);
		gfx_draw_filled_circle (x, y, radius+radius/25, GFX_COL_PINK_1);
	}


	gfx_draw_filled_circle (x, y, radius, col);

}


void render_sun_line (int xo, int yo, int x, int y, int radius)
{
	
	if(y%2 == 0) return;


	int sy = yo + y;
	int sx,ex;

	if ((sy < GFX_VIEW_TY + GFX_Y_OFFSET) ||
		(sy > GFX_VIEW_BY + GFX_Y_OFFSET+2)) 
		return;
	
	sx = xo - x;
	ex = xo + x;

	sx -= (radius * (2 + (randint() & 10))) >> 8;
	ex += (radius * (2 + (randint() & 10))) >> 8;
	
	if ((sx > GFX_VIEW_BX + GFX_X_OFFSET) ||
		(ex < GFX_VIEW_TX + GFX_X_OFFSET))
		return;
	
	if (sx < GFX_VIEW_TX + GFX_X_OFFSET)
		sx = GFX_VIEW_TX + GFX_X_OFFSET;
	
	if (ex > GFX_VIEW_BX + GFX_X_OFFSET+2)
		ex = GFX_VIEW_BX + GFX_X_OFFSET+2;


	float fe= (float)docked_planet.e/900;
	float fd= (float)docked_planet.d/500;
	float ff= (float)docked_planet.f/750;


	if(current_screen == SCR_PLANET_DATA)
	{
		fe= (float)hyperspace_planet.e/1900;
		fd= (float)hyperspace_planet.f/900;
		ff= (float)hyperspace_planet.a/800;
	}
	else
	{
		fe= (float)docked_planet.e/1900;
		fd= (float)docked_planet.f/700;
		ff= (float)docked_planet.a/900;
	}

	if(ff>0.2) {ff =0.7+ff; fd-= ff; }

	//	glColor3f(1,0.8,0);
	glColor3f(1-fe,1-fd, ff);


	if ( razaar_refugees>100 && (cmdr.galaxy_number == 0) && (docked_planet.d == 112) && 
		(docked_planet.b == 95) && univ_obj_currently_being_updated== 0) glColor3f(1, 0.8, 0.1);

	if ( razaar_refugees>100 && (cmdr.galaxy_number == 0) && (docked_planet.d == 112) && 
		(docked_planet.b == 95) && univ_obj_currently_being_updated== 1) glColor3f(1, 0.5, 0.1);

	glBegin(GL_LINES);
		glVertex2f(sx-1, sy); 
		glVertex2f(ex+1, sy); 
	glEnd();

	glColor3f(1-fe,1-fd-0.1, ff);

	if ( razaar_refugees>100 && (cmdr.galaxy_number == 0) && (docked_planet.d == 112) && 
		(docked_planet.b == 95) && univ_obj_currently_being_updated== 0) glColor3f(1, 0.9, 0.95);

	if ( razaar_refugees>100 && (cmdr.galaxy_number == 0) && (docked_planet.d == 112) && 
		(docked_planet.b == 95) && univ_obj_currently_being_updated== 1) glColor3f(1, 0.7, 0);

	//	glColor3f(1,0.7,0);
	glBegin(GL_LINES);
		glVertex2f(sx, sy+1); 
		glVertex2f(ex, sy+1); 
	glEnd();


}


void render_sun (int xo, int yo, int radius)
{
	int x,y;
	int s;
	
	xo += GFX_X_OFFSET;
	yo += GFX_Y_OFFSET;
	
	s = -radius;
	x = radius;
	y = 0;

	// s -= x + x;
	while (y <= x)
	{
		render_sun_line (xo, yo, x, y, radius);
		render_sun_line (xo, yo, x,-y, radius);
		render_sun_line (xo, yo, y, x, radius);
		render_sun_line (xo, yo, y,-x, radius);
		
		s += y + y + 1;
		y++;
		if (s >= 0)
		{
			s -= x + x + 2;
			x--;
		}				
	}

}



void draw_sun (struct univ_object *planet)
{


	int x,y;
	int radius;


	
	x = (planet->location.x * 256) / planet->location.z;
	y = (planet->location.y * 256) / planet->location.z;

	y = -y;
	
	x += 128;
	y += 96;

	x *= GFX_SCALE;
	y *= GFX_SCALE;
	
	radius = 6291456 / planet->distance;

	radius *= GFX_SCALE;

	if ((x + radius <  0) ||
		(x - radius > 511) ||
		(y + radius < 0) ||
		(y - radius > 383))
		return; 

	radius+=2;
	render_sun (x, y, radius);
}



void draw_explosion (struct univ_object *univ)
{

	int x, y;

	x = (univ->location.x * 256) / univ->location.z +128;
	y = -(univ->location.y * 256) / univ->location.z +96;

	int r= 75;

	if(current_screen == SCR_ESCAPE_NOVA)
		r+= univ->exp_delta * 5*univ->exp_delta;
	else
	{
		r+= univ->exp_delta*5 - (int)(univ->distance/400);
	}
	
	if(univ->type == SHIP_THARGLET || (univ->type == SHIP_CARGO && (univ->bravery == -101 || univ->bravery == 2))) //Genesis Capsules or Radioactives Explosions
	{		

		if(univ_obj_currently_being_updated%2 == 0)
		{		

			if(univ->exp_delta%2 == 0 )
				gfx_draw_filled_circle (x*GFX_SCALE, y*GFX_SCALE, r+25, GFX_COL_PINK_2);
			else
			if(univ->exp_delta%3 == 0 )
				gfx_draw_filled_circle (x*GFX_SCALE, y*GFX_SCALE, r+70, GFX_COL_GREY_4);
			else
				gfx_draw_filled_circle (x*GFX_SCALE, y*GFX_SCALE, r+45, GFX_COL_PINK_1);

		}
		else
		{

			if(univ->exp_delta%2 == 0 )
				gfx_draw_filled_circle (x*GFX_SCALE, y*GFX_SCALE, r+25, GFX_COL_PINK_1);
			else
			if(univ->exp_delta%3 == 0 )
				gfx_draw_filled_circle (x*GFX_SCALE, y*GFX_SCALE, r+70, GFX_COL_PINK_2);
			else
				gfx_draw_filled_circle (x*GFX_SCALE, y*GFX_SCALE, r+45, GFX_COL_GREY_4);

		}



	}
	else 
	{

		if(univ_obj_currently_being_updated%2 == 0)
		{		

			if(univ->exp_delta%2 == 0)
				gfx_draw_filled_circle (x*GFX_SCALE, y*GFX_SCALE, r, GFX_COL_ORANGE_1);
			else
				gfx_draw_filled_circle (x*GFX_SCALE, y*GFX_SCALE, r, GFX_COL_RED_1);
		}
		else
		{

			if(univ->exp_delta%2 == 0)
				gfx_draw_filled_circle (x*GFX_SCALE, y*GFX_SCALE, r, GFX_COL_YELLOW_2);
			else
				gfx_draw_filled_circle (x*GFX_SCALE, y*GFX_SCALE, r, GFX_COL_ORANGE_2);

		}

	
	}

}



/*
 * Draws an object in the universe.
 * (Ship, Planet, Sun etc).
 */

void draw_ship (struct univ_object *ship)
{

	if ((current_screen != SCR_FRONT_VIEW) && (current_screen != SCR_REAR_VIEW) && 
		(current_screen != SCR_LEFT_VIEW) && (current_screen != SCR_RIGHT_VIEW) &&
		(current_screen != SCR_INTRO_ONE) && (current_screen != SCR_INTRO_TWO) &&
		(current_screen != SCR_GAME_OVER) && (current_screen != SCR_ESCAPE_POD)
		&& (current_screen != SCR_ESCAPE_NOVA))
		return;

	if((ship->flags & FLG_DEAD) && !(ship->flags & FLG_EXPLOSION) )
	{
		ship->flags |= FLG_EXPLOSION;
		ship->exp_delta = 0;
	}


	if((ship->flags & FLG_DEAD) && (ship->flags & FLG_EXPLOSION) )
	{
		ship->exp_delta += 1;
		
		if(ship->exp_delta> 5)
			ship->flags |= FLG_REMOVE;
	}



	if (ship->location.z <= 0)	/* Only display ships in front of us. */
		return;


	if (ship->type == SHIP_SUN)
	{

		draw_sun (ship);
		return;
	}

	if (ship->type == SHIP_PLANET)
	{
		draw_planet (ship);
		return;
	}


	
	if ((abs((int)ship->location.x) > ship->location.z) ||	/* Check for field of vision. */
		(abs((int)ship->location.y) > ship->location.z))
		return;


	if (ship->flags & FLG_EXPLOSION && (ship->type>0))
	{
		draw_explosion (ship);
		return;
	}

	draw_solid_ship (ship);
}

