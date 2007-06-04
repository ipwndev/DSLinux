/* ns.h
 * Javascript namespace
 * (c) 2002 Martin 'PerM' Pergel
 * This file is a part of the Links program, released under GPL
 */

abuf* getarg(abuf**);
void add_to_parlist(lns*,lns*);
void zrusargy(abuf*,js_context*);
void delete_from_parlist(lns*,lns*);
lns* llookup(char*,js_id_name **,plns*,js_context*);
lns* cllookup(char*,js_id_name **,plns*,js_context*);
lns* loklookup(long,plns*,js_context*);
char *key_2_name(long, js_context *);
char* find_var_name(long,js_id_name*);
