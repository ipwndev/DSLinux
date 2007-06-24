/*
 * This file contains the C library functions for the Mathomatic library.
 * Refer to this, if you are going to use the Mathomatic code in other projects.
 *
 * Be sure and define "LIBRARY" when compiling the Mathomatic code for this.
 * And be sure to call "clear_all()" after completing each group of operations,
 * otherwise the equation spaces will fill up.
 *
 * Mathomatic Copyright (C) 1987-2007 George Gesslein II.
 */

#include "../includes.h"
#include "mathomatic.h"

/*
 * Initialize Mathomatic.
 * Call this once before calling any Mathomatic code.
 *
 * Returns true if successful.
 * If this returns false, there was not enough memory
 * and Mathomatic cannot be used.
 */
int
matho_init()
{
	init_gvars();
	gfp = stdout;
	if (!init_mem()) {
		return false;
	}
	signal(SIGFPE, fphandler);
	return true;
}

/*
 * Process a Mathomatic command or input equation.
 * Input string is in "input", output string is stored in "*outputp".
 *
 * This function works just like typing something into the Mathomatic prompt.
 *
 * If this returns true, the command or input was successful,
 * and the resulting equation output is stored in "*outputp".
 * This is a malloc()ed equation text string which must be free()d after use.
 *
 * If this returns false, the command or input failed and a text error
 * message is stored in "*outputp".  The error message must not
 * be free()d.
 *
 * This routine will set "*outputp" to NULL,
 * if there is no resulting equation or error message.
 */
int
matho_process(char *input, char **outputp)
{
	int	i;
	int	rv;

	*outputp = NULL;
	input = strdup(input);
	if ((i = setjmp(jmp_save)) != 0) {
		clean_up();	/* Mathomatic processing was interrupted, so do a clean up. */
		if (i == 14) {
			error(_("Expression too large."));
		}
		*outputp = error_str;
		free(input);
		return false;
	}
	result_str = NULL;
	error_str = NULL;
	set_error_level(input);
	if ((rv = process(input))) {
		*outputp = result_str;
	} else {
		*outputp = error_str;
	}
	free(input);
	return rv;
}

/*
 * Parse an equation or expression and store in the next available equation space,
 * making it the current equation.
 *
 * Input string is in "input", output string is stored in "*outputp".
 *
 * Same as matho_process() above, except commands are not allowed, so that variables
 * are not confused with commands.
 *
 * Returns true if successful.
 */
int
matho_parse(char *input, char **outputp)
{
	int	i;
	int	rv;

	*outputp = NULL;
	input = strdup(input);
	if ((i = setjmp(jmp_save)) != 0) {
		clean_up();	/* Mathomatic processing was interrupted, so do a clean up. */
		if (i == 14) {
			error(_("Expression too large."));
		}
		*outputp = error_str;
		free(input);
		return false;
	}
	result_str = NULL;
	error_str = NULL;
	set_error_level(input);
	i = next_espace();
#if	true	/* set this true if you want to be able to enter single variable expressions with no solving */
	if ((rv = parse(i, input))) {
#else
	if ((rv = process_parse(i, input))) {
#endif
		*outputp = result_str;
	} else {
		*outputp = error_str;
	}
	free(input);
	return rv;
}

/*
 * Floating point exception handler.
 * Usually doesn't work in most operating systems.
 */
void
fphandler(int sig)
{
        error(_("Floating point exception."));
}
