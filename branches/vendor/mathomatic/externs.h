/*
 * Mathomatic global variable extern definitions, from file "globals.c".
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

extern int		n_tokens;
extern int		n_equations;
extern int		cur_equation;

extern token_type	*lhs[N_EQUATIONS];
extern token_type	*rhs[N_EQUATIONS];

extern int		n_lhs[N_EQUATIONS];
extern int		n_rhs[N_EQUATIONS];

extern token_type	*tlhs;
extern token_type	*trhs;
extern token_type	*tes;

extern int		n_tlhs;
extern int		n_trhs;
extern int		n_tes;

extern token_type	*scratch;

extern token_type	zero_token;
extern token_type	one_token;

extern char		*var_names[MAX_VAR_NAMES];

extern int		precision;
extern int		case_sensitive_flag;
extern int		factor_int_flag;
extern int		display2d;
extern int		approximate_roots;
extern int		preserve_roots;
extern int		true_modulus;
extern int		screen_columns;
extern int		screen_rows;
extern int		finance_option;
extern int		autosolve;
extern int		autocalc;
extern char		special_variable_character;
extern int		debug_level;
extern int		domain_check;
extern int		color_flag;
extern int		bold_colors;
extern int		cur_color;
extern int		html_flag;
extern int		readline_enabled;
extern int		partial_flag;
extern int		symb_flag;
extern int		high_prec;
extern int		input_column;
extern int		sign_flag;
extern double		small_epsilon;
extern double		epsilon;
extern char		var_str[MAX_VAR_LEN+80];
extern char		prompt_str[MAX_PROMPT_LEN];
extern char		*dir_path;
extern char		*prog_name;

extern double		unique[];
extern int		ucnt[];
extern int		uno;

extern sign_array_type	sign_array;
extern FILE		*gfp;
extern jmp_buf		jmp_save;
extern int		test_mode;
extern int		quiet_mode;

extern char		*error_str;
extern char		*result_str;
