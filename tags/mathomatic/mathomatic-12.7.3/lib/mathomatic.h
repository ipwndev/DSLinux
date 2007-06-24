/*
 * Include file for the Mathomatic symbolic math library.
 */

int matho_init(void);
int matho_process(char *input, char **outputp);
int matho_parse(char *input, char **outputp);
int clear_all(void);

extern int	cur_equation;			/* current equation space number (origin 0) */
