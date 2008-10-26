/*
 * $Id: parser.c,v 1.2 2003/12/21 09:56:20 shadow Exp $
 *
 * functions for parsing a replimenu-type menufile.
 *
 */

/*
 * CVS log:
 *
 * $Log: parser.c,v $
 * Revision 1.2  2003/12/21 09:56:20  shadow
 * added CVS keywords to most source files
 *
 */

#include <string.h>

#define bufsize_small 256
#define bufsize_medium 1024
#define bufsize_large 1024*4

const unsigned int c_space = 0x20;
const unsigned int c_tab = 0x09;
const unsigned int c_shit = 0x0FF;
const unsigned int c_cr = 0x0d;
const unsigned int c_lf = 0x0a;
const unsigned int c_comment = '#';
const unsigned int c_zero = 0x00;

const unsigned int c_separator = '=';


/*****************************************************************************/

void nullify_surrounding_spaces(char *line) {

	unsigned int x = 0;
	unsigned int y = 0;

	unsigned int i = 0;

	while ((line[x] == c_space) || (line[x] == c_tab) || (line[x] == c_shit) || (line[x] == c_cr) || (line[x] == c_lf))
	{ x++; }

	while (line[x] != c_zero) {
		line[i] = line[x];
		i++;
		x++;
	}

	line[i] = 0;

	while (line[y] != 0)
	{ y++; }

	while ((line[y] == c_space) || (line[y] == c_tab) || (line[y] == c_shit) || (line[y] == c_zero) || (line[y] == c_cr) || (line[y] == c_lf))
	{ y--;	}

	line[y+1] = 0;
}


/*****************************************************************************/

char **separate_string(char *line) {
	static char string1[bufsize_small];
	static char string2[bufsize_large];
	static char *string_vector[] = {string1, string2};

	unsigned int i = 0;
	unsigned int variable_start;
	unsigned int variable_length;
	unsigned int value_start;
	unsigned int value_length;

	string1[0] = 0;
	string2[0] = 0;

	while ((line[i] == c_space) || (line[i] == c_tab) || (line[i] == c_shit))
	{ i++;	}

	if (line[i] == c_comment)
		return NULL;

	variable_start = i;

	while ((line[i] != c_separator) && (line[i] != c_zero) && (line[i] != c_space) && (line[i] != c_tab) && (line[i] != c_shit))
	{ i++;	}

	if (line[i] == c_zero) {
		/* it's a single variable, return it asis */
		variable_length = i - variable_start;
		if (variable_length > sizeof(string1))
			return NULL;
		memcpy(string1, &line[variable_start], variable_length);
		string1[variable_length] = 0;
		nullify_surrounding_spaces(string1);
		return string_vector;
	}

	/* else it has to have been a c_separator, c_space or c_tab - that makes it a variable definer */

	variable_length = i - variable_start;
	if (variable_length > sizeof(string1))
	{	return NULL; }
	memcpy(string1, &line[variable_start], variable_length);
	string1[variable_length] = 0;
	nullify_surrounding_spaces(string1);

	i++;	/* go beyond the c_separator, c_space or c_tab */

	while ((line[i] == c_space) || (line[i] == c_tab) || (line[i] == c_shit) || (line[i] == c_separator)) {
		if (line[i] == c_separator) {
			i++;
			break;
		}
		i++;
	}

	value_start = i;

	while ((line[i] != c_zero) && (line[i] != c_shit))
	{ i++;	}

	value_length = i - value_start;
	if (value_length > sizeof(string2))
	{	return NULL; }
	memcpy(string2, &line[value_start], value_length);
	string2[value_length] = 0;
	nullify_surrounding_spaces(string2);

	return string_vector;
}

/*****************************************************************************/
