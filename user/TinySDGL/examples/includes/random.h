
int randint (void);
void set_rand_seed (int seed);
int get_rand_seed (void);
int rand255 (void);


// - - - - - - - - - - -  F I L E   E N D - - - - - - - - - - - 


static int rand_seed;

int randint (void)
{
	int k1;
	int ix = rand_seed;
	
	k1 = ix / 127773;
	ix = 16807 * (ix - k1 * 127773) - k1 * 2836;
	if (ix < 0)
		ix += 2147483647;
	rand_seed = ix;

	return ix; 
}
 

void set_rand_seed (int seed)
{
	rand_seed = seed;
}


int get_rand_seed (void)
{
	return rand_seed;
}

int rand255 (void)
{
	return (randint() &255);
}

