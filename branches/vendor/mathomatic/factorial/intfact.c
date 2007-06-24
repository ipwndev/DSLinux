/*
 * Factorial function in C for integers.
 *
 * Return (arg!).  Return 0 on overflow or error.
 */
int
fact(int arg)
{
	int	i;

	if (arg < 0) {
		return 0;
	}
	for (i = 1; i > 0 && arg > 1; arg--) {
		i *= arg;
	}
	if (i > 0) {
		return i;
	} else {
		return 0;
	}
}
