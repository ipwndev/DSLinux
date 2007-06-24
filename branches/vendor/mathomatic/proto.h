/*
 * Global function prototypes for Mathomatic.
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

/* command function list */
int		clear_cmd(), quit_cmd(), list_cmd(), simplify_cmd(), help_cmd(), eliminate_cmd();
int		fraction_cmd(), unfactor_cmd(), compare_cmd(), extrema_cmd();
int		read_cmd(), display_cmd(), calculate_cmd(), solve_cmd();
int		factor_cmd(), derivative_cmd(), replace_cmd();
int		save_cmd(), taylor_cmd(), limit_cmd(), echo_cmd();
int		copy_cmd(), divide_cmd(), pause_cmd(), version_cmd();
int		edit_cmd(), real_cmd(), imaginary_cmd(), tally_cmd();
int		roots_cmd(), set_cmd(), code_cmd(), optimize_cmd(), push_cmd();
int		sum_cmd(), product_cmd(), integrate_cmd(), nintegrate_cmd(), laplace_cmd();

/* various functions that don't return int */
char		*dirname_win();
char		*skip_space(), *skip_param();
char		*get_string();
char		*parse_equation(), *parse_section(), *parse_var(), *parse_var2(), *parse_expr();
char		*list_expression(), *list_equation();
double		gcd(), my_round(), multiply_out_unique();
long		decstrtol();

void fphandler(int sig);
void inthandler(int sig);
void alarmhandler(int sig);
void resizehandler(int sig);
void exit_program(int exit_value);
