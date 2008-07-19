


// - - - - - - - - - - -  F I L E   E N D - - - - - - - - - - - 



char *mission1_brief_a =
	"Greetings Commander, I am Captain Curruthers of "
	"Her Majesty's Space Navy and I beg a moment of your "
	"valuable time. We would like you to do a little job "
	"for us. The ship you see here is a new model, the "
	"Constrictor, equiped with a  top secret new shield "
	"generator. Unfortunately it's been stolen.";

char *mission1_brief_b =
	"It went missing from our ship yard on Xeer five months ago "
	"and was last seen at Reesdice. Your mission should you decide "
	"to accept it, is to seek and destroy this ship. You are "
	"cautioned that only Military Lasers will get through the new "
	"shields and that the Constrictor is fitted with an E.C.M. "
	"System. Good Luck, Commander. ---MESSAGE ENDS.";


char *mission1_debrief =
	"Reward of 2000.0 Cr has been transferred, spend it wisely Commander. "
	"There will always be a place for you in Her Majesty's Space Navy, "
	"maybe sooner than you think... ---MESSAGE ENDS.";

char *mission1_pdesc[] =
{
	"THE CONSTRICTOR WAS LAST SEEN AT REESDICE, COMMANDER.",
	"A STRANGE LOOKING SHIP LEFT HERE A WHILE BACK. LOOKED BOUND FOR AREXE.",
	"YEP, AN UNUSUAL NEW SHIP HAD A GALACTIC HYPERDRIVE FITTED HERE, USED IT TOO.",
	"I HEAR A WEIRD LOOKING SHIP WAS SEEN AT ERRIUS.",
	"THIS STRANGE SHIP DEHYPED HERE FROM NOWHERE, SUN SKIMMED AND JUMPED. I HEAR IT WENT TO INBIBE.",
	"ROGUE SHIP WENT FOR ME AT AUSAR. MY LASERS DIDN'T EVEN SCRATCH ITS HULL.",
	"OH DEAR ME YES. A FRIGHTFUL ROGUE WITH WHAT I BELIEVE YOU PEOPLE CALL A LEAD "
		"POSTERIOR SHOT UP LOTS OF THOSE BEASTLY PIRATES AND WENT TO USLERI.",
	"YOU CAN TACKLE THE VICIOUS SCOUNDRELS IF YOU LIKE. HE'S AT ORARRA.",
	"A REAL DEADLY PIRATES LAUNCHED A MINUTE AGO. THEY ALMOST COUGHT THEM HERE AT THE STATION "
	"WHEN THEY DOCKED TO RE-FUEL. I DON'T KNOW WHERE THEY CAN GO WITHOUT THE FUEL, NOT TOO FAR I GUESS.",
	"BOY ARE YOU IN THE WRONG GALAXY!",
};

char *mission2_brief_a =
	"Recent visit of two Space Dredgers to this station has confirmed our fears. "
	"GalFam humanitarian convoy was ambushed by Thargoids. Most of the fleet was pulled to miss-jump and dissapeared "
	"in the deep blackenss of Witchspace. Later on, by some chance Dredgers recovered some of the Genesis Capsules "
	"that were on a way to Zaisedan in the next Galaxy. We need "
	"to find some other way of releasing Ecology Seed and "
	"dispersing it in the planet's atmosphere. Simultaneous activation at low altitude "
	"is the best we can try now without proper equipment. Capsules are being loaded in your ship in special food containers. "
	"---MESSAGE ENDS.";

/*
"Last night there was this guy, I think his name was Jedi. "
"He showed me his Light Sabre and sliced us some cheese with it. "
"I don't know, he seemed quite a nice guy to me, but others say he killed Younglings... "
"He went to 10th Galaxy. ---MESSAGE ENDS."


*/

char *mission2_brief_b =
	"Good day Commander. I am Agent Blake of Naval Intelligence. As you know, "
	"the Navy have been keeping the Thargoids off your ass out in deep space "
	"for many years now. Well the situation has changed. Our boys are ready "
	"for a push right to the home system of those murderers.";

char *mission2_brief_c =
	"I have obtained the defence plans for their Hive Worlds. The beetles "
	"know we've got something but not what. If I transmit the plans to our "
	"base on Birera they'll intercept the transmission. I need a ship to "
	"make the run. You're elected. The plans are unipulse coded within "
	"this transmission. You will be paid. Good luck Commander. ---MESSAGE ENDS.";



//... take mthis energy bomb, 3 waves of thargoids
//zurid pino escaped seen in modified thargoid ship/cloack


int been_to_orarra= FALSE;

char *mission_planet_desc (struct galaxy_seed planet)
{
	int pnum;


	if (!docked)
		return NULL;

	if ((planet.a != docked_planet.a) ||
	    (planet.b != docked_planet.b) ||
	    (planet.c != docked_planet.c) ||
	    (planet.d != docked_planet.d) ||
	    (planet.e != docked_planet.e) ||
	    (planet.f != docked_planet.f))
		return NULL;
	

	pnum = find_planet_number (planet);
	
	if (cmdr.mission==1 && cmdr.galaxy_number == 0)
	{
		switch (pnum)
 		{
			case 150:
				sprintf(find_name,"REESDICE");
				return mission1_pdesc[0];
			
			case 36:
				sprintf(find_name,"AREXE");
				return mission1_pdesc[1];

			case 28:
				return mission1_pdesc[2];							
		}
	}



	if (cmdr.mission == 1 && cmdr.galaxy_number == 1)
	{

		if(been_to_orarra && pnum !=193)
		{
			sprintf(find_name,"ORARRA");
			return NULL; 
		}
		switch (pnum)
		{
			case 179:

			case 32:
			case 68:
			case 164:
			case 220:
			case 106:
			case 16:
			case 162:
			case 3:
			case 107:
			case 26:
			case 192:
			case 184:
			case 5:
				sprintf(find_name,"ERRIUS");
				return mission1_pdesc[3];
		
			case 253:
				sprintf(find_name,"INBIBE");
				return mission1_pdesc[4];
			
			case 79:
				sprintf(find_name,"AUSAR");
				return mission1_pdesc[5];

			case 53:
				sprintf(find_name,"USLERI");
				return mission1_pdesc[6];							

			case 118:
				sprintf(find_name,"ORARRA");
				return mission1_pdesc[7];							

			case 193:
				been_to_orarra= TRUE;
				return mission1_pdesc[8];							
		}
	}

	
	return NULL;
}




//*** missions *********************************************************

void constrictor_mission_brief (void)
{


	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	gfx_display_pretty_text (16, 50, 290, 350, mission1_brief_a);
	gfx_display_pretty_text (16, 210, 450, 350, mission1_brief_b);
		
	move_univ_object (&universe[3]);
	univ_oM = universe[3];
	draw_wireframe_ship(&univ_oM);
}	


void constrictor_mission_debrief (void)
{

	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	gfx_display_centre_text (45, "Congratulations Commander.", 140, GFX_COL_BLUE_4);
	gfx_display_pretty_text (16, 70, 440, 384, mission1_debrief);


	//thargoid 1st brief
	gfx_display_pretty_text (16, 152, 430, 384, mission2_brief_a);
}




void thargoid_mission_second_brief (void)
{

	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	gfx_display_pretty_text (16, 40, 300, 384, mission2_brief_b);
	gfx_display_pretty_text (16, 185, 440, 384, mission2_brief_c);


	move_univ_object (&universe[3]);
	univ_oM = universe[3];
	draw_wireframe_ship(&univ_oM);
	
}


void thargoid_mission_debrief (void)
{

	gfx_display_centre_text (360, "Press Fire or Space, Commander.", 140, GFX_COL_GOLD);
	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);


	if(!game_over)
	{

		gfx_display_centre_text (45, "Well done Commander.", 140, GFX_COL_BLUE_4);
		gfx_display_pretty_text (16, 70, 435, 384, 
			"You have served us well and we shall remember... "
			"Thanks for playing, "
			"you are welcome to post a comment or report a bug, cheers. "
			" ---MESSAGE ENDS.");


		glColor3f(1,0,0);
		txtOut(335, 190, "the end", 4);
		glColor3f(0.9,0.7,0);
		txtOut (265, 210, "www.polarelephant.blogspot.com", 2);
	

	}
	else
	{

		gfx_display_centre_text (45, "How unfortunate Commander.", 140, GFX_COL_BLUE_4);
		gfx_display_pretty_text (16, 70, 445, 384, 
		"You don't have enough money. "
		"Sorry, next time if you gonna loose your ride, make sure you leave aside at least 30.0 Cr. Remember, "
		"when your spaceship gets blasted away only equipment can be claimed back with a new ship, cargo is lost. "
		"---MESSAGE ENDS.");


		glColor3f(1,0,0);
		txtOut(315, 215, "game over", 4);

	}



	glColor3f(0,1,0);

	sprintf(str, "Score"); 
	txtOut(200,275, str, 3);

	sprintf(str, "Credits"); 
	txtOut(200,300, str, 3);

	sprintf(str, "Total"); 
	txtOut(200,330, str, 3);

	sprintf(str, "Rating"); 
	txtOut(200,355, str, 3);

	glColor3f(0,0.5,0);

	glBegin(GL_LINES);

		glVertex2f(180, 325);
		glVertex2f(440, 325);

	glEnd();


	glColor3f(1,1,1);
	sprintf(str, "%d", cmdr.score); 
	txtOut(320,275, str, 3);

	sprintf(str, "%.1f", (float)cmdr.credits/10); 
	txtOut(320,300, str, 3);

	sprintf(str, "%d", cmdr.score + cmdr.credits/100); 
	txtOut(320,330, str, 3);

	int i;
	for (i = 0; i < NO_OF_RANKS; i++)
		if (cmdr.score >= rating[i].score)
			strcpy (str, rating[i].title);

	if(cheat) 
		strcat(str, " (chEaT)");		

	txtOut(320,355, str, 3);


}


void asteroid_mission_brief(void)
{

	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);
	gfx_display_centre_text (45, "High alert Commander.", 140, GFX_COL_BLUE_4);

	gfx_display_pretty_text (16, 70, 440, 384, 
	"Wake up Commander, bad dreams, eh... speaking of which, its nightmare out there. "
	"The planet is under the threat of collision with an asteroid shower. Defence Ministry pays "
	"bonus of 5.0 Cr per destroyed Asteroid. Now, let me quickly introduce you to some modifications. "
	"Scanner has two modes of operation, use Short Range Scan to better keep track of your "
	"close range targets. Remember, you can engage Warp Jump engine even with radar contacts "
	"present as long as Closest Contact distance "
	"reads at least 100.0 units, otherwise jump will be MASS-LOCKED as it is nearby planet or a star. "
	"Closest Missile distance will display range to nearest missile and "
	"Cargo Jettison comes as standard feature on MkIII models at our shipyard. "
	"Ok, fuel re-filled, you're ready to go Commander... watch your altitude.  ---MESSAGE ENDS.");
}


void cloak_mission_brief(void)
{
	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);
	gfx_display_centre_text (45, "Whew, thank you Commander.", 140, GFX_COL_BLUE_4);

	gfx_display_pretty_text (16, 70, 450, 384, 
	"You saved many Lavians, I've never seen such flying, that was unbeliavable. "
	"Those Asteroids, they were everywhere, from here we could only see "
	"huge flaming balls when they entered Lave's atmosphere. Thank you, please recieve this Fuel Scoop "
	"unit as a bonus reward. Lave will never forget you Commander. ---MESSAGE ENDS.");


	gfx_display_centre_text (205, "Warning to all Traders.", 140, GFX_COL_BLUE_4);
	gfx_display_pretty_text (16, 230, 445, 384, 
	"Reports having been coming in from Traders in "
	"Usle sector of an unknown hostile ship. "
	"Rumours suggest that this ship is fitted with some sort of Cloaking Device "
	"that makes it almost invisible to Radar. It has claimed many Trader's lives and thier cargo, "
	"if you manage to destroy it you may scoop whatever is left of it. ---MESSAGE ENDS.");

}



void cloak_mission_brief2(void)
{
	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);
	gfx_display_centre_text (45, "Ororqu massacre.", 140, GFX_COL_BLUE_4);

	gfx_display_pretty_text (16, 70, 450, 384, 
	"Oh, dear me... what a tragedy. "
	"They tried to set up a trap near Ororqu, but somehow it all turned ugly and only one pilot "
	"managed to return. He said E.C.M. saved his life when suddenly sworms of missiles came out of nowhere. "
	"We also recieved a news that misterious ship might be piloted by infamous Zurid Pino. There is a reward "
	"on Zurid's head of 1000.0 Cr if he is captured alive. Sir, stay away from Ororqu, he may still be there. "
	"Damn pirates, they're bad for business, uh-huh. ---MESSAGE ENDS.");
}

void nova_mission_brief(void)
{
	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);
	gfx_display_centre_text (45, "Great job Commander", 140, GFX_COL_BLUE_4);

	gfx_display_pretty_text (16, 70, 445, 384, 
	"Thanks to you this sector is safe again. I hope you managed to salvage "
	"cargo of that ship, it must have been full of plunder. Anyway, feel free to sell your scoop "
	"at our station. You will always be "
	"welcome around here Commander. ---MESSAGE ENDS.");

	if(zpino==101)
	{
		gfx_display_pretty_text (16, 160, 435, 384, 
		"Identification confirmed, it was Zurid Pino after all. "
		"He is now being transfered to high security prison in 5th Galaxy. "
		"Hopefully he will stay there for a long, long time. 1000.0 Cr has been transfered to your account. ---MESSAGE ENDS.");
	}
	else
	{

		gfx_display_pretty_text (16, 160, 445, 384, 
		"No prisoners, eh Commander. Pino had stolen Escape Pod at Esveor system and we were hoping "
		"to capture him alive. If pressed hard enough he would have no choice but to eject from damaged ship. "
		"Shame we are not able now to confirm if that was really him. ---MESSAGE ENDS.");

	}


	gfx_display_centre_text (275, "Urgent message Commander.", 140, GFX_COL_BLUE_4);
	gfx_display_pretty_text (16, 300, 440, 384, 
	"We recieved distress call form the Razaar system. It is not clear what is going on there. We're "
	"getting some strange readings from that direction. I think you better hurry up Commander. ---MESSAGE ENDS.");

}


void nova_mission_brief2(void)
{
	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);
	gfx_display_centre_text (45, "Situation Critical Commander.", 140, GFX_COL_BLUE_4);

	gfx_display_pretty_text (16, 70, 450, 384, 
	"Our sun is going 'Supernova'. We were expecting this, "
	"but not this soon. Our scientists miscalculated. It was not supposed to happen for the next 25,000 years. "
	"Here at the station we transported all of the most importatan "
	"individuals from the planet, they must be evacuated safely Commander. Arragements have been made "
	"with Inonri goverment, they will provide Generation Ships for people of Razaar to find a new home planet. ");

	gfx_display_pretty_text (16, 210, 445, 384, 
	"Forget now whatever cargo you might have, there is not even enough time to repair your Cobra. We will "
	"provide you with another spcecraft, "
	"but the station is out of fuel too, so you will only have enough to Warp Jump away to safety. "
	"From there on its all in your hands. Well, thats the best we can do, new Cobra may not have all the equipment, "
	"but at least it will not leak fuel once you manage to find some. Our hopes lie with you now Commander. ---MESSAGE ENDS.");
	

	
}



void astr2_mission_brief(void)
{
	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);
	gfx_display_centre_text (45, "Our Hero, they say Commander.", 140, GFX_COL_BLUE_4);

	gfx_display_pretty_text (16, 70, 450, 384, 
	"Razaar Refugees have been transported to temporary accomodation where they will wait untill finally "
	"boarded into newly built Generation Ships. As a token of appreciation they paid for a little gadget to be made for you. "
	"I'm sure many Razaarian children will hear the story of a brave Commander that saved their "
	"ancestors from the exploding sun. Sheesh, that's like in some kind of movie or something, heavy stuff man."
	"---MESSAGE ENDS.");

	gfx_display_centre_text (245, "Asteroid shower on Qube system.", 140, GFX_COL_BLUE_4);
	
	gfx_display_pretty_text (16, 270, 445, 384, 
	"Greetings from Qube system Commander. We have heard of your skills and are in dire need for "
	"someone of your caliber as Asteroid shower is aproaching our system. If you choose to answer our call we will "
	"mount Beam Lasers on your ship. ---MESSAGE ENDS.");
	
	
}

void astr2_mission_brief2(void)
{

	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);
	gfx_display_centre_text (45, "Hopefully not too late Commander...", 140, GFX_COL_BLUE_4);

	gfx_display_pretty_text (16, 70, 435, 384, 
	"Asteroid shower has already started, we were expecting you sooner. As promissed, "
	"Front Beam Laser has been mounted on your ship and you are ready to go. "
	"We will pay standard bonus amount of 5.0 Cr per destroyed Asteroid. Now, hurry up and "
	"watch your altitude. Good luck Commander. ---MESSAGE ENDS.");
}

void gen_mission_debrief(void)
{

	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);
	gfx_display_centre_text (45, "Genesis Capsule deployment.", 140, GFX_COL_BLUE_4);


	if(gal_fam ==-500)		
		gfx_display_pretty_text (16, 70, 430, 384, 
		"It equals to a miracle that Space Dredgers stumbled across our Capsules, how in a world did they come back "
		"from Witchspace, huh. Job well done, Capsules seem to have activated and reaction has started, "
		"however destiny of Zaisedan will stay uncertain for months to come. We "
		"will need to wait and see, lets hope for the best. This is poor world and can only pay "
		"500.0 Cr for your services. May the force be with you Commander. ---MESSAGE ENDS.");

	else
	if(gal_fam ==-999)
	{
	
	
		gfx_display_pretty_text (16, 70, 440, 384, 
		"Huh, it is too late now, you were responsible for the last 10 Genesis Capsules that we miracously retrieved "
		"after Thargoid ambush of GalFam humanitarian convoy. I guess we should have hired someone of true ELITE "
		"status. Real pilots are rare to find, kids these day can't even land strait without Docking Computer. Ah, "
		"life is a bitch. ---MESSAGE ENDS.");
	
	}		


	gfx_display_centre_text (245, "Ceerdi Mission Briefing.", 140, GFX_COL_BLUE_4);
	
	gfx_display_pretty_text (16, 270, 435, 384, 
	"Attention Commander, I am Captain Fortesque of Her Majesty's Space Navy. "
	"We have need of your services again. If you would be so good as to go to "
	"Ceerdi you will be briefed. If succesful, you will be rewarded. "
	"---MESSAGE ENDS.");

}

void last_mission(void)
{


	gfx_display_centre_text (10, "MESSAGE CONSOLE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);
	gfx_display_centre_text (45, "The Battle for Birera.", 140, GFX_COL_BLUE_4);


	gfx_display_pretty_text (16, 70, 435, 384, 
	"The Station is under attack, we must prepare for urgent launching procedure. "
	"Commander, I'm afraid that Thargoid Invasion has started. Several other Outposts, "
	"including two Secret Bases, have already been destroyed. "
	"Your Cobra MkIII is being upgraded with Galactic Navy Extra Energy Unit and "
	"specially modified Energy Bomb will be at your disposal. All friendly identified craft "
	"should be safe from the detonation as well as your ship, even so, don't be too haste to use it. "
	"The Hive World plans have been "
	"retrieved from your on-board computer and de-coded successfully. The future of the whole "
	"Galaxy may now be in our hands. " 
	"Birera Station must be defended at all cost. Die hard Commander. ---MESSAGE ENDS.");


}






//*********************** -----------------
void check_mission_brief (void)
{


	if(cmdr.mission == -1001 )
	{
		cmdr.mission = -1002;

		launch_player();
		cmdr.front_laser= BEAM_LASER;
		int i;
		for(i=0; i< 50; i++)
		{
			create_pirates();
			create_viper_patrol();
			create_pirates();
		}

		return;
	}


	if ( cmdr.mission == -101 )
	{

		int pts= cmdr.score;
		
		cmdr= saved_cmdr;
		cmdr.score= pts;

		cmdr.mission = -1;
		current_screen = SCR_MISSION_A;
		asteroid_mission_brief();
		return;
	}

	if ( cmdr.mission == -102 )
	{

		cmdr.mission = -2;
		current_screen = SCR_MISSION_B;
		sprintf(find_name,"USLE");
		
		cmdr.fuel_scoop= 1;
		cloak_mission_brief();
		return;
	}



	if ( cmdr.mission == -2 && (docked_planet.d == 85) && (docked_planet.b == 99) && cmdr.galaxy_number == 0)
	{
		cmdr.mission = -3;

		current_screen = SCR_MISSION_B2;

		sprintf(find_name,"ORORQU");
		cloak_mission_brief2();
		return;
	}



	if ( cmdr.mission == -104 )
	{

		cmdr.mission = -4;
		current_screen = SCR_MISSION_C;
		sprintf(find_name,"RAZAAR");

		if(zpino ==100)
		{
			zpino= 101;
			cmdr.credits+= 10000;
			cmdr.score += 1000; // Zurid Pino captured alive AND DELIVERED

		}
		else
			zpino= 0;


		razaar_refugees= 0;
		nova_mission_brief();
		return;
	}


	if ( cmdr.mission == -4 && (docked_planet.d == 112) && (docked_planet.b == 95) && cmdr.galaxy_number == 0)
	{
		cmdr.mission = -5;
		sprintf(find_name,"INONRI");

		int i;
		for (i = 0; i < 17; i++)
			cmdr.current_cargo[i]= 0;

		cmdr.cargo_capacity= 8;

		cmdr.missiles= 1;

		cmdr.left_laser = 0;
		cmdr.right_laser = 0;
		cmdr.rear_laser = 0;
		cmdr.front_laser = PULSE_LASER;

		cmdr.ecm= 0;
		cmdr.energy_bomb= 0;
		cmdr.docking_computer = 0;
		cmdr.galactic_hyperdrive= 0;
		cmdr.energy_unit= 0;
		cmdr.fuel= 9;
		cmdr.fuel_scoop= 1;
		cmdr.legal_status= 0;

		RAZAAR= docked_planet;


		razaar_refugees= 100;
		current_screen= SCR_MISSION_C2;

		nova_mission_brief2();
		return;
	}



	if ( cmdr.mission == -5 && (docked_planet.d == 114) && (docked_planet.b == 161) && cmdr.galaxy_number == 0)
	{
		cmdr.mission = -106;

		if(razaar_refugees ==101) 
			cmdr.cargo_capacity+= 12;
		
		razaar_refugees= 500;
	
		sprintf(find_name,"QUBE");
		current_screen = SCR_MISSION_D;

		astr2_mission_brief();
		return;
	}


	if ( cmdr.mission == -106 && (docked_planet.d == 152) && (docked_planet.b == 205) && cmdr.galaxy_number == 0)
	{
		cmdr.mission = -6;
		cmdr.front_laser = BEAM_LASER;

		current_screen = SCR_MISSION_D2;

		astr2_mission_brief2();
		return;
	}




//***

	if (cmdr.mission == -107)
	{
		sprintf(find_name,"REESDICE");

		cmdr.mission = 1;
		current_screen = SCR_MISSION_1;
		cougar_pod= 0;

		clear_universe();
		set_init_matrix (intro_ship_matrix);
		add_new_ship (SHIP_CONSTRICTOR, 160, 65, 550, intro_ship_matrix, -1, -1);

		constrictor_mission_brief();
		return;
	}


	if (cmdr.mission == 2)
	{
		sprintf(find_name,"ZAISEDAN"); 
		cmdr.credits += 20000;

		current_screen = SCR_MISSION_2;

		gal_fam= 10;
		cmdr.current_cargo[0]= 10; //food (special Genesis Capsule)

		cmdr.mission = 3;	
		
		constrictor_mission_debrief();
		return;
	}






	if (cmdr.mission == 3 && gal_fam <10)
	{

		if(gal_fam ==-500) //success
			cmdr.credits += 5000; //pay for Jettison/Energy bomb job - genesis capsule
		else
			gal_fam= -999;

		cmdr.mission = 4;	
		sprintf(find_name,"CEERDI");
		current_screen = SCR_MISSION_GENESIS;
		
		gen_mission_debrief();
		return;
	}



	if ((cmdr.mission == 4) && (docked_planet.d == 215) && (docked_planet.b == 84) && cmdr.galaxy_number == 2)
	{

		sprintf(find_name,"BIRERA");
		cmdr.mission = 5;
		current_screen = SCR_MISSION_3;

		clear_universe();
		set_init_matrix (intro_ship_matrix);
		add_new_ship (SHIP_THARGOID, 410, 200, 1300, intro_ship_matrix, 1, -1);

		thargoid_mission_second_brief();
		return;
	}


	if ((cmdr.mission == 5) && (docked_planet.d == 63) && (docked_planet.b == 72) && cmdr.galaxy_number == 2)
	{

		
		cmdr.mission = 6;
		cmdr.energy_unit = 2; 
		cmdr.energy_bomb = 1;
		current_screen = SCR_LAST_MISSION;

		last_mission();
		return;
	}


	if (cmdr.mission ==107)
	{

		cmdr.mission = 7;
	
		game_over= FALSE; 
		
		finished_once= TRUE;
		current_screen = SCR_MISSION_4;

		thargoid_mission_debrief();
		return;
	}


	display_commander_status();

}

