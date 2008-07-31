/*
 * Functions used across the program.
 *
 * Copyright 2008 Andrew Wood, distributed under the Artistic License 2.0.
 */

#ifndef _PV_H
#define _PV_H 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _OPTIONS_H
struct opts_s;
typedef struct opts_s *opts_t;
#endif

double pv_getnum_d(char *);
int pv_getnum_i(char *);
long long pv_getnum_ll(char *);
int pv_getnum_check(char *, int);

void pv_screensize(opts_t);
void pv_calc_total_size(opts_t);

int pv_main_loop(opts_t);
void pv_display(opts_t, long double, long long, long long);
long pv_transfer(opts_t, int, int *, int *, unsigned long long, long *);
void pv_set_buffer_size(unsigned long long, int);
int pv_next_file(opts_t, int, int);

void pv_crs_fini(opts_t);
void pv_crs_init(opts_t);
void pv_crs_update(opts_t, char *);
#ifdef HAVE_IPC
void pv_crs_needreinit(void);
#endif

void pv_sig_allowpause(void);
void pv_sig_checkbg(void);
void pv_sig_init(void);
void pv_sig_nopause(void);


#ifdef __cplusplus
}
#endif

#endif /* _PV_H */

/* EOF */
