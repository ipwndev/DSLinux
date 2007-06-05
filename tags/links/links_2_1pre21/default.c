/* default.c
 * (c) 2002 Mikulas Patocka, Petr 'Brain' Kulhavy
 * This file is a part of the Links program, released under GPL
 */

#include "links.h"

/* prototypes */
void get_system_name(void);

void get_system_name(void)
{
	FILE *f;
	unsigned char *p;
#if defined(HAVE_SYS_UTSNAME_H) && defined(HAVE_UNAME)
	struct utsname name;
	memset(&name, 0, sizeof name);
	if (!uname(&name)) {
		unsigned char *str = init_str();
		int l = 0;
		add_to_str(&str, &l, name.sysname);
		add_to_str(&str, &l, " ");
		add_to_str(&str, &l, name.release);
		add_to_str(&str, &l, " ");
		add_to_str(&str, &l, name.machine);
		if (l >= MAX_STR_LEN) str[MAX_STR_LEN - 1] = 0;
		strcpy(system_name, str);
		mem_free(str);
		return;
	}
#endif
#ifdef HAVE_POPEN
	memset(system_name, 0, MAX_STR_LEN);
	if (!(f = popen("uname -srm", "r"))) goto fail;
	if (fread(system_name, 1, MAX_STR_LEN - 1, f) <= 0) {
		pclose(f);
		goto fail;
	}
	pclose(f);
	for (p = system_name; *p; p++) if (*p < ' ') {
		*p = 0;
		break;
	}
	if (system_name[0]) return;
	fail:
#endif
	strcpy(system_name, SYSTEM_NAME);
}

struct option {
	int p;
	unsigned char *(*rd_cmd)(struct option *, unsigned char ***, int *);
	unsigned char *(*rd_cfg)(struct option *, unsigned char *);
	void (*wr_cfg)(struct option *, unsigned char **, int *);
	int min, max;	/* for double min and max are in 1/100's (e.g. 0.1 is min==10) */
	void *ptr;
	unsigned char *cfg_name;
	unsigned char *cmd_name;
};

extern struct option links_options[];
extern struct option html_options[];

struct option *all_options[] = { links_options, html_options, NULL, };


/* prototypes */
unsigned char *get_token(unsigned char **);
void parse_config_file(unsigned char *, unsigned char *, struct option **);
unsigned char *create_config_string(struct option *);
unsigned char *get_home(int *);
void load_config_file(unsigned char *, unsigned char *);
int write_config_data(unsigned char *, unsigned char *, struct option *, struct terminal *);
void add_nm(struct option *, unsigned char **, int *);
void add_quoted_to_str(unsigned char **, int *, unsigned char *);
unsigned char *num_rd(struct option *, unsigned char *);
void num_wr(struct option *, unsigned char **, int *);
unsigned char *dbl_rd(struct option *, unsigned char *);
void dbl_wr(struct option *, unsigned char **, int *);
unsigned char *str_rd(struct option *, unsigned char *);
void str_wr(struct option *, unsigned char **, int *);
unsigned char *cp_rd(struct option *, unsigned char *);
void cp_wr(struct option *, unsigned char **, int *);
unsigned char *lang_rd(struct option *, unsigned char *);
void lang_wr(struct option *, unsigned char **, int *);
int getnum(unsigned char *, int *, int, int);
unsigned char *type_rd(struct option *, unsigned char *);
void type_wr(struct option *, unsigned char **, int *);
unsigned char *ext_rd(struct option *, unsigned char *);
void ext_wr(struct option *, unsigned char **, int *);
unsigned char *prog_rd(struct option *, unsigned char *);
void prog_wr(struct option *, unsigned char **, int *);
unsigned char *term_rd(struct option *, unsigned char *);
unsigned char *term2_rd(struct option *, unsigned char *);
void term_wr(struct option *, unsigned char **, int *);
unsigned char *dp_rd(struct option *, unsigned char *);
void dp_wr(struct option *, unsigned char **, int *);
unsigned char *gen_cmd(struct option *, unsigned char ***, int *);
unsigned char *lookup_cmd(struct option *, unsigned char ***, int *);
unsigned char *version_cmd(struct option *, unsigned char ***, int *);
unsigned char *set_cmd(struct option *, unsigned char ***, int *);
unsigned char *unset_cmd(struct option *, unsigned char ***, int *);
unsigned char *setstr_cmd(struct option *, unsigned char ***, int *);
unsigned char *force_html_cmd(struct option *, unsigned char ***, int *);
unsigned char *dump_cmd(struct option *, unsigned char ***, int *);
unsigned char *printhelp_cmd(struct option *, unsigned char ***, int *);
unsigned char *_parse_options(int, unsigned char *[], struct option **);
unsigned char *block_rd(struct option *, unsigned char *);
void block_wr(struct option *, unsigned char **, int *);

unsigned char *_parse_options(int argc, unsigned char *argv[], struct option **opt)
{
	unsigned char *e, *u = NULL;
	while (argc) {
		int i;
		argv++, argc--;
		if (argv[-1][0] == '-') {
			struct option *options;
			struct option **op;
			for (op = opt; (options = *op); op++) for (i = 0; options[i].p; i++)
				if (options[i].rd_cmd && options[i].cmd_name &&
				    !strcasecmp(options[i].cmd_name, &argv[-1][1])) {
					if ((e = options[i].rd_cmd(&options[i], &argv, &argc))) {
						if (e[0]) fprintf(stderr, "Error parsing option %s: %s\n", argv[-1], e);
						return NULL;
					}
					goto found;
				}
			uu:
#ifdef GRDRV_DIRECTFB
                        if (!strncmp(argv[-1], "--dfb:", 6))
                                goto found;
#endif
			fprintf(stderr, "Unknown option %s\n", argv[-1]);
			return NULL;
		} else if (!u) u = argv[-1];
		else goto uu;
		found:;
	}
	if (u) return u;
	return "";
}

unsigned char *parse_options(int argc, unsigned char *argv[])
{
	return _parse_options(argc, argv, all_options);
}

unsigned char *get_token(unsigned char **line)
{
	unsigned char *s = NULL;
	int l = 0;
	int escape = 0;
	int quote = 0;
	
	while (**line == ' ' || **line == 9) (*line)++;
	if (**line) {
		for (s = init_str(); **line; (*line)++) {
			if (escape)
				escape = 0;
			else if (**line == '\\') {
				escape = 1;
				continue;
			}	
			else if (**line == '"') {
				quote = !quote;
				continue;
			}
			else if ((**line == ' ' || **line == 9) && !quote)
				break;
			add_chr_to_str(&s, &l, **line);
		}
	}
	return s;
}

void parse_config_file(unsigned char *name, unsigned char *file, struct option **opt)
{
	struct option *options;
	struct option **op;
	int err = 0;
	int line = 0;
	unsigned char *e;
	int i;
	unsigned char *n, *p;
	unsigned char *tok;
	int nl, pl;
	while (file[0]) {
		line++;
		while (file[0] && (file[0] == ' ' || file[0] == 9)) file++;
		n = file;
		while (file[0] && file[0] > ' ') file++;
		if (file == n) {
			if (file[0]) file++;
			continue;
		}
		nl = file - n;
		while (file[0] == 9 || file[0] == ' ') file++;
		p = file;
		while (file[0] && file[0] != 10 && file[0] != 13) file++;
		pl = file - p;
		if (file[0]) {
			if ((file[1] == 10 || file[1] == 13) && file[0] != file[1]) file++;
			file++;
		}
		tok = NULL;
		if (n[0] == '#') goto f;
		if (!(tok = get_token(&n))) goto f;
		nl = strlen(tok);
		for (op = opt; (options = *op); op++)
			for (i = 0; options[i].p; i++) if (options[i].cfg_name && (size_t)nl == strlen(options[i].cfg_name) && !casecmp(tok, options[i].cfg_name, nl)) {
				unsigned char *o = memacpy(p, pl);
				if ((e = options[i].rd_cfg(&options[i], o))) {
					if (e[0]) fprintf(stderr, "Error parsing config file %s, line %d: %s\n", name, line, e), err = 1;
				}
				mem_free(o);
				goto f;
			}
		fprintf(stderr, "Unknown option in config file %s, line %d\n", name, line);
		err = 1;
		f:
		if (tok) mem_free(tok);
	}
	if (err) fprintf(stderr, "\007"), sleep(1);
}

unsigned char *create_config_string(struct option *options)
{
	unsigned char *s = init_str();
	int l = 0;
	int i;
	add_to_str(&s, &l, "# This file is automatically generated by Links -- please do not edit.");
	for (i = 0; options[i].p; i++) if (options[i].wr_cfg)
		options[i].wr_cfg(&options[i], &s, &l);
	add_to_str(&s, &l, NEWLINE);
	return s;
}

#define FILE_BUF	1024

unsigned char cfg_buffer[FILE_BUF];

unsigned char *read_config_file(unsigned char *name)
{
	int h, r;
	int l = 0;
	unsigned char *s;
	if ((h = open(name, O_RDONLY | O_NOCTTY)) == -1) return NULL;
	set_bin(h);
	s = init_str();
	while ((r = read(h, cfg_buffer, FILE_BUF)) > 0) {
		int i;
		for (i = 0; i < r; i++) if (!cfg_buffer[i]) cfg_buffer[i] = ' ';
		add_bytes_to_str(&s, &l, cfg_buffer, r);
	}
	if (r == -1) mem_free(s), s = NULL;
	close(h);
	return s;
}

int write_to_config_file(unsigned char *name, unsigned char *c)
{
	int rr;
	int r;
	int h, w;
	unsigned char *tmp_name = stracpy(name);
	unsigned char *ds, *dt, *dot;
	for (dt = ds = tmp_name; *dt; dt++) if (dir_sep(*dt)) ds = dt;
	if ((dot = strchr(ds, '.'))) *dot = 0;
	add_to_strn(&tmp_name, ".tmp");
	if ((h = open(tmp_name, O_WRONLY | O_NOCTTY | O_CREAT | O_TRUNC, 0666)) == -1) {
		mem_free(tmp_name);
		return errno;
	}
	set_bin(h);
	rr = strlen(c);
	r = rr;
	while (r > 0) {
		if ((w = write(h, c + rr - r, r)) <= 0) {
			int err = !w ? ENOSPC : errno;
			close(h);
			unlink(tmp_name);
			mem_free(tmp_name);
			return err;
		}
		r -= w;
	}
	close(h);
#ifndef RENAME_OVER_EXISTING_FILES
	unlink(name);
#endif
	if (rename(tmp_name, name)) {
		int err = errno;
		unlink(tmp_name);
		mem_free(tmp_name);
		return err;
	}
	mem_free(tmp_name);
	return 0;
}

unsigned char *get_home(int *n)
{
	struct stat st;
	unsigned char *home = stracpy(getenv("HOME"));
	unsigned char *home_links;
	unsigned char *config_dir = stracpy(getenv("CONFIG_DIR"));

	if (n) *n = 1;
	if (!home) {
		int i;
		home = stracpy(path_to_exe);
		if (!home) {
			if (config_dir) mem_free(config_dir);
			return NULL;
		}
		for (i = strlen(home) - 1; i >= 0; i--) if (dir_sep(home[i])) {
			home[i + 1] = 0;
			goto br;
		}
		home[0] = 0;
		br:;
	}
	while (home[0] && dir_sep(home[strlen(home) - 1])) home[strlen(home) - 1] = 0;
	if (home[0]) add_to_strn(&home, "/");
	home_links = stracpy(home);
	if (config_dir)
	{
		add_to_strn(&home_links, config_dir);
		while (home_links[0] && dir_sep(home_links[strlen(home_links) - 1])) home_links[strlen(home_links) - 1] = 0;
		if (stat(home_links, &st) != -1 && S_ISDIR(st.st_mode)) {
			add_to_strn(&home_links, "/links");
		} else {
			fprintf(stderr, "CONFIG_DIR set to %s. But directory %s doesn't exist.\n\007", config_dir, home_links);
			sleep(3);
			mem_free(home_links);
			home_links = stracpy(home);
			add_to_strn(&home_links, ".links");		
		}
		mem_free(config_dir);
	} else add_to_strn(&home_links, ".links");
	if (stat(home_links, &st)) {
#ifdef HAVE_MKDIR
		if (!mkdir(home_links, 0777)) goto home_creat;
#endif
		if (config_dir) goto failed;
		goto first_failed;
	}
	if (S_ISDIR(st.st_mode)) goto home_ok;
	first_failed:
	mem_free(home_links);
	home_links = stracpy(home);
	add_to_strn(&home_links, "links");
	if (stat(home_links, &st)) {
#ifdef HAVE_MKDIR
		if (!mkdir(home_links, 0777)) goto home_creat;
#else
		mem_free(home_links);
		home_links = stracpy(home);
		goto home_ok;
#endif
		goto failed;
	}
	if (S_ISDIR(st.st_mode)) goto home_ok;
	failed:
	mem_free(home_links);
	mem_free(home);
	return NULL;

	home_ok:
	if (n) *n = 0;
	home_creat:
#ifdef HAVE_CHMOD
	chmod(home_links, 0700);
#endif
	add_to_strn(&home_links, "/");
	mem_free(home);
	return home_links;
}

void init_home(void)
{
	get_system_name();
	links_home = get_home(&first_use);
	if (!links_home) {
		fprintf(stderr, "Unable to find or create links config directory. Please check, that you have $HOME variable set correctly and that you have write permission to your home directory.\n\007");
		sleep(3);
		return;
	}
}

void load_config_file(unsigned char *prefix, unsigned char *name)
{
	unsigned char *c, *config_file;
	config_file = stracpy(prefix);
	if (!config_file) return;
	add_to_strn(&config_file, name);
	if ((c = read_config_file(config_file))) goto ok;
	mem_free(config_file);
	config_file = stracpy(prefix);
	if (!config_file) return;
	add_to_strn(&config_file, ".");
	add_to_strn(&config_file, name);
	if ((c = read_config_file(config_file))) goto ok;
	mem_free(config_file);
	return;
	ok:
	parse_config_file(config_file, c, all_options);
	mem_free(c);
	mem_free(config_file);
}

void load_config(void)
{
#ifdef SHARED_CONFIG_DIR
	load_config_file(SHARED_CONFIG_DIR, "links.cfg");
#endif
	load_config_file(links_home, "links.cfg");
	load_config_file(links_home, "html.cfg");
	load_config_file(links_home, "user.cfg");
}

int write_config_data(unsigned char *prefix, unsigned char *name, struct option *o, struct terminal *term)
{
	int err;
	unsigned char *c, *config_file;
	if (!(c = create_config_string(o))) return -1;
	config_file = stracpy(prefix);
	if (!config_file) {
		mem_free(c);
		return -1;
	}
	add_to_strn(&config_file, name);
	if ((err = write_to_config_file(config_file, c))) {
		if (term) msg_box(term, NULL, TEXT(T_CONFIG_ERROR), AL_CENTER | AL_EXTD_TEXT, TEXT(T_UNABLE_TO_WRITE_TO_CONFIG_FILE), ": ", get_err_msg(-err), NULL, NULL, 1, TEXT(T_CANCEL), NULL, B_ENTER | B_ESC);
		mem_free(c);
		mem_free(config_file);
		return -1;
	}
	mem_free(c);
	mem_free(config_file);
	return 0;
}

void write_config(struct terminal *term)
{
#ifdef G
	if (F) update_driver_param();
#endif
	write_config_data(links_home, "links.cfg", links_options, term);
}

void write_html_config(struct terminal *term)
{
	write_config_data(links_home, "html.cfg", html_options, term);
}

void add_nm(struct option *o, unsigned char **s, int *l)
{
	if (*l) add_to_str(s, l, NEWLINE);
	add_to_str(s, l, o->cfg_name);
	add_to_str(s, l, " ");
}

void add_quoted_to_str(unsigned char **s, int *l, unsigned char *q)
{
	add_chr_to_str(s, l, '"');
	while (*q) {
		if (*q == '"' || *q == '\\') add_chr_to_str(s, l, '\\');
		add_chr_to_str(s, l, *q);
		q++;
	}
	add_chr_to_str(s, l, '"');
}

unsigned char *num_rd(struct option *o, unsigned char *c)
{
	unsigned char *tok = get_token(&c);
	unsigned char *end;
	long l;
	if (!tok) return "Missing argument";
	l = strtolx(tok, &end);
	if (*end) {
		mem_free(tok);
		return "Number expected";
	}
	if (l < o->min || l > o->max) {
		mem_free(tok);
		return "Out of range";
	}
	*(int *)o->ptr = l;
	mem_free(tok);
	return NULL;
}

void num_wr(struct option *o, unsigned char **s, int *l)
{
	add_nm(o, s, l);
	add_knum_to_str(s, l, *(int *)o->ptr);
}

#define DBL_PRECISION 10000


unsigned char *dbl_rd(struct option *o, unsigned char *c)
{
	unsigned char *tok = get_token(&c);
	char *end;
	double d;
	
	if (!tok) return "Missing argument";
	d = strtod(tok, &end);

	if (*end) {
		mem_free(tok);
		return "Number expected";
	}
	if (100*d < o->min || 100*d > o->max) {
		mem_free(tok);
		return "Out of range";
	}
	*(double *)o->ptr = d;
	mem_free(tok);
	return NULL;
}

void dbl_wr(struct option *o, unsigned char **s, int *l)
{
	long y=*(double*)o->ptr;
	long x=((*(double*)o->ptr)-y)*DBL_PRECISION;

	add_nm(o, s, l);
	add_knum_to_str(s, l, y);
	add_chr_to_str(s, l, '.');
	add_knum_to_str(s, l, x);
}

unsigned char *str_rd(struct option *o, unsigned char *c)
{
	unsigned char *tok = get_token(&c);
	unsigned char *e = NULL;
	if (!tok) return NULL;
	if (strlen(tok) + 1 > (size_t)o->max) e = "String too long";
	else strcpy(o->ptr, tok);
	mem_free(tok);
	return e;
}

void str_wr(struct option *o, unsigned char **s, int *l)
{
	add_nm(o, s, l);
	if (strlen(o->ptr) + 1 > (size_t)o->max) {
		unsigned char *s1 = init_str();
		int l1 = 0;
		add_bytes_to_str(&s1, &l1, o->ptr, o->max - 1);
		add_quoted_to_str(s, l, s1);
		mem_free(s1);
	}
	else add_quoted_to_str(s, l, o->ptr);
}

unsigned char *cp_rd(struct option *o, unsigned char *c)
{
	unsigned char *tok = get_token(&c);
	unsigned char *e = NULL;
	int i;
	if (!tok) return "Missing argument";
	if ((i = get_cp_index(tok)) == -1) e = "Unknown codepage";
	else if (o->min == 1 && is_cp_special(i)) e = "UTF-8 can't be here";
	else *(int *)o->ptr = i;
	mem_free(tok);
	return e;
}

void cp_wr(struct option *o, unsigned char **s, int *l)
{
	unsigned char *n = get_cp_mime_name(*(int *)o->ptr);
	add_nm(o, s, l);
	add_to_str(s, l, n);
}

unsigned char *lang_rd(struct option *o, unsigned char *c)
{
	int i;
	unsigned char *tok = get_token(&c);
	if (!tok) return "Missing argument";
	for (i = 0; i < n_languages(); i++)
		if (!(strcasecmp(language_name(i), tok))) {
			set_language(i);
			mem_free(tok);
			return NULL;
		}
	mem_free(tok);
	return "Unknown language";
}

void lang_wr(struct option *o, unsigned char **s, int *l)
{
	add_nm(o, s, l);
	add_quoted_to_str(s, l, language_name(current_language));
}

int getnum(unsigned char *s, int *n, int r1, int r2)
{
	unsigned char *e;
	long l = strtol(s, (char **)(void *)&e, 10);
	if (*e || !*s) return -1;
	if (l < r1 || l >= r2) return -1;
	*n = (int)l;
	return 0;
}

unsigned char *type_rd(struct option *o, unsigned char *c)
{
	unsigned char *err = "Error reading association specification";
	struct assoc new;
	unsigned char *w;
	int n;
	memset(&new, 0, sizeof(struct assoc));
	if (!(new.label = get_token(&c))) goto err;
	if (!(new.ct = get_token(&c))) goto err;
	if (!(new.prog = get_token(&c))) goto err;
	if (!(w = get_token(&c))) goto err;
	if (getnum(w, &n, 0, 32)) goto err_f;
	mem_free(w);
	new.cons = !!(n & 1);
	new.xwin = !!(n & 2);
	new.ask = !!(n & 4);
	if ((n & 8) || (n & 16)) new.block = !!(n & 16);
	else new.block = !new.xwin || new.cons;
	if (!(w = get_token(&c))) goto err;
	if (strlen(w) != 1 || w[0] < '0' || w[0] > '9') goto err_f;
	new.system = w[0] - '0';
	mem_free(w);
	update_assoc(&new);
	err = NULL;
	err:
	if (new.label) mem_free(new.label);
	if (new.ct) mem_free(new.ct);
	if (new.prog) mem_free(new.prog);
	return err;
	err_f:
	mem_free(w);
	goto err;
}


unsigned char *block_rd(struct option *o, unsigned char *c)
{
	unsigned char *err = "Error reading image block specification";
	unsigned char* url;

	if(!(url = get_token(&c)))
		return err;
	
	block_add_URL_fn(NULL, url);

	mem_free(url);

	return NULL;
}

void block_wr(struct option *o, unsigned char **s, int *l)
{
	struct block *a;
	foreachback(a, blocks) {
		add_nm(o, s, l);
		add_quoted_to_str(s, l, a->url);
	}
}




void type_wr(struct option *o, unsigned char **s, int *l)
{
	struct assoc *a;
	foreachback(a, assoc) {
		add_nm(o, s, l);
		add_quoted_to_str(s, l, a->label);
		add_to_str(s, l, " ");
		add_quoted_to_str(s, l, a->ct);
		add_to_str(s, l, " ");
		add_quoted_to_str(s, l, a->prog);
		add_to_str(s, l, " ");
		add_num_to_str(s, l, (!!a->cons) + (!!a->xwin) * 2 + (!!a->ask) * 4 + (!a->block) * 8 + (!!a->block) * 16);
		add_to_str(s, l, " ");
		add_num_to_str(s, l, a->system);
	}
}

unsigned char *ext_rd(struct option *o, unsigned char *c)
{
	unsigned char *err = "Error reading extension specification";
	struct extension new;
	memset(&new, 0, sizeof(struct extension));
	if (!(new.ext = get_token(&c))) goto err;
	if (!(new.ct = get_token(&c))) goto err;
	update_ext(&new);
	err = NULL;
	err:
	if (new.ext) mem_free(new.ext);
	if (new.ct) mem_free(new.ct);
	return err;
}

void ext_wr(struct option *o, unsigned char **s, int *l)
{
	struct extension *a;
	foreachback(a, extensions) {
		add_nm(o, s, l);
		add_quoted_to_str(s, l, a->ext);
		add_to_str(s, l, " ");
		add_quoted_to_str(s, l, a->ct);
	}
}

unsigned char *prog_rd(struct option *o, unsigned char *c)
{
	unsigned char *err = "Error reading program specification";
	unsigned char *prog, *w;
	if (!(prog = get_token(&c))) goto err_1;
	if (!(w = get_token(&c))) goto err_2;
	if (strlen(w) != 1 || w[0] < '0' || w[0] > '9') goto err_3;
	update_prog(o->ptr, prog, w[0] - '0');
	err = NULL;
	err_3:
	mem_free(w);
	err_2:
	mem_free(prog);
	err_1:
	return err;
}

void prog_wr(struct option *o, unsigned char **s, int *l)
{
	struct protocol_program *a;
	foreachback(a, *(struct list_head *)o->ptr) {
		if (!*a->prog) continue;
		add_nm(o, s, l);
		add_quoted_to_str(s, l, a->prog);
		add_to_str(s, l, " ");
		add_num_to_str(s, l, a->system);
	}
}

unsigned char *term_rd(struct option *o, unsigned char *c)
{
	struct term_spec *ts;
	unsigned char *w;
	int i;
	if (!(w = get_token(&c))) goto err;
	if (!(ts = new_term_spec(w))) {
		mem_free(w);
		goto end;
	}
	mem_free(w);
	if (!(w = get_token(&c))) goto err;
	if (strlen(w) != 1 || w[0] < '0' || w[0] > '4') goto err_f;
	ts->mode = w[0] - '0';
	mem_free(w);
	if (!(w = get_token(&c))) goto err;
	if (strlen(w) != 1 || w[0] < '0' || w[0] > '3') goto err_f;
	ts->m11_hack = (w[0] - '0') & 1;
	ts->braille = !!((w[0] - '0') & 2);
	mem_free(w);
	if (!(w = get_token(&c))) goto err;
	if (strlen(w) != 1 || w[0] < '0' || w[0] > '7') goto err_f;
	ts->col = (w[0] - '0') & 1;
	ts->restrict_852 = !!((w[0] - '0') & 2);
	ts->block_cursor = !!((w[0] - '0') & 4);
	mem_free(w);
	if (!(w = get_token(&c))) goto err;
	if ((i = get_cp_index(w)) == -1 || is_cp_special(i)) goto err_f;
	ts->charset = i;
	mem_free(w);
	end:
	return NULL;
	err_f:
	mem_free(w);
	err:
	return "Error reading terminal specification";
}

unsigned char *term2_rd(struct option *o, unsigned char *c)
{
	struct term_spec *ts;
	unsigned char *w;
	int i;
	if (!(w = get_token(&c))) goto err;
	if (!(ts = new_term_spec(w))) {
		mem_free(w);
		goto end;
	}
	mem_free(w);
	if (!(w = get_token(&c))) goto err;
	if (strlen(w) != 1 || w[0] < '0' || w[0] > '3') goto err_f;
	ts->mode = w[0] - '0';
	mem_free(w);
	if (!(w = get_token(&c))) goto err;
	if (strlen(w) != 1 || w[0] < '0' || w[0] > '1') goto err_f;
	ts->m11_hack = w[0] - '0';
	mem_free(w);
	if (!(w = get_token(&c))) goto err;
	if (strlen(w) != 1 || w[0] < '0' || w[0] > '1') goto err_f;
	ts->restrict_852 = w[0] - '0';
	mem_free(w);
	if (!(w = get_token(&c))) goto err;
	if (strlen(w) != 1 || w[0] < '0' || w[0] > '1') goto err_f;
	ts->col = w[0] - '0';
	mem_free(w);
	if (!(w = get_token(&c))) goto err;
	if ((i = get_cp_index(w)) == -1 || is_cp_special(i)) goto err_f;
	ts->charset = i;
	mem_free(w);
	end:
	return NULL;
	err_f:
	mem_free(w);
	err:
	return "Error reading terminal specification";
}

void term_wr(struct option *o, unsigned char **s, int *l)
{
	struct term_spec *ts;
	foreachback(ts, term_specs) {
		add_nm(o, s, l);
		add_quoted_to_str(s, l, ts->term);
		add_to_str(s, l, " ");
		add_num_to_str(s, l, ts->mode);
		add_to_str(s, l, " ");
		add_num_to_str(s, l, !!ts->m11_hack + !!ts->braille * 2);
		add_to_str(s, l, " ");
		add_num_to_str(s, l, !!ts->col + !!ts->restrict_852 * 2 + !!ts->block_cursor * 4);
		add_to_str(s, l, " ");
		add_to_str(s, l, get_cp_mime_name(ts->charset));
	}
}

struct list_head driver_params = { &driver_params, &driver_params };

struct driver_param *get_driver_param(unsigned char *n)
{
	struct driver_param *dp;
	foreach(dp, driver_params) if (!strcasecmp(dp->name, n)) return dp;
	dp = mem_calloc(sizeof(struct driver_param) + strlen(n) + 1);
	dp->codepage = get_cp_index("iso-8859-1");
	strcpy(dp->name, n);
	dp->shell = mem_calloc(1);
	dp->nosave = 1;
	add_to_list(driver_params, dp);
	return dp;
}

unsigned char *dp_rd(struct option *o, unsigned char *c)
{
	int cc;
	unsigned char *n, *param, *cp, *shell;
	struct driver_param *dp;
	if (!(n = get_token(&c))) goto err;
	if (!(param = get_token(&c))) {
		mem_free(n);
		goto err;
	}
	if (!(shell = get_token(&c))){
		mem_free(n);
		mem_free(param);
		goto err;
	}
	if (!(cp = get_token(&c))) {
		mem_free(n);
		mem_free(param);
		mem_free(shell);
		goto err;
	}
	if ((cc=get_cp_index(cp)) == -1 || is_cp_special(cc)) {
		mem_free(n);
		mem_free(param);
		mem_free(shell);
		mem_free(cp);
		goto err;
	}
	dp=get_driver_param(n);
	dp->codepage=cc;
	if (dp->param) mem_free(dp->param);
	dp->param=param;
	if (dp->shell) mem_free(dp->shell);
	dp->shell=shell;
	dp->nosave = 0;
	mem_free(cp);
	mem_free(n);
	return NULL;
	err:
	return "Error reading driver mode specification";
}

void dp_wr(struct option *o, unsigned char **s, int *l)
{
	struct driver_param *dp;
	foreachback(dp, driver_params) {
		if ((!dp->param || !*dp->param) && !dp->codepage && !*dp->shell) continue;
		if (dp->nosave) continue;
		add_nm(o, s, l);
		add_quoted_to_str(s, l, dp->name);
		add_to_str(s, l, " ");
		add_quoted_to_str(s, l, dp->param ? dp->param : (unsigned char*)"");
		add_to_str(s, l, " ");
		add_quoted_to_str(s, l, dp->shell);
		add_to_str(s, l, " ");
		add_to_str(s, l, get_cp_mime_name(dp->codepage));
	}
}

unsigned char *gen_cmd(struct option *o, unsigned char ***argv, int *argc)
{
	unsigned char *r;
	if (!*argc) return "Parameter expected";
	(*argv)++; (*argc)--;
	if (!(r = o->rd_cfg(o, *(*argv - 1)))) return NULL;
	(*argv)--; (*argc)++;
	return r;
}

unsigned char *lookup_cmd(struct option *o, unsigned char ***argv, int *argc)
{
	ip__address addr;
	unsigned char *p = (unsigned char *)&addr;
	if (!*argc) return "Parameter expected";
	if (*argc >= 2) return "Too many parameters";
	(*argv)++; (*argc)--;
	if (do_real_lookup(*(*argv - 1), &addr)) {
#ifdef HAVE_HERROR
		herror("error");
#else
		fprintf(stderr, "error: host not found\n");
#endif
		return "";
	}
	printf("%d.%d.%d.%d\n", (int)p[0], (int)p[1], (int)p[2], (int)p[3]);
	fflush(stdout);
	return "";
}

unsigned char *version_cmd(struct option *o, unsigned char ***argv, int *argc)
{
	printf("Links " VERSION_STRING "\n");
	fflush(stdout);
	return "";
}

unsigned char *set_cmd(struct option *o, unsigned char ***argv, int *argc)
{
	*(int *)o->ptr = 1;
	return NULL;
}

unsigned char *unset_cmd(struct option *o, unsigned char ***argv, int *argc)
{
	*(int *)o->ptr = 0;
	return NULL;
}

unsigned char *setstr_cmd(struct option *o, unsigned char ***argv, int *argc)
{
	if (!*argc) return "Parameter expected";
	strncpy(o->ptr, **argv, o->max);
	((unsigned char *)o->ptr)[o->max - 1] = 0;
	(*argv)++; (*argc)--;
	return NULL;
}

unsigned char *force_html_cmd(struct option *o, unsigned char ***argv, int *argc)
{
	force_html = 1;
	return NULL;
}

unsigned char *dump_cmd(struct option *o, unsigned char ***argv, int *argc)
{
	if (dmp != o->min && dmp) return "Can't use both -dump and -source";
	dmp = o->min;
	no_connect = 1;
	return NULL;
}

unsigned char *printhelp_cmd(struct option *o, unsigned char ***argv, int *argc)
{
/* Changed and splited - translation is much easier.
 * Print to stdout instead stderr (,,links -help | more''
 * is much better than ,,links -help 2>&1 | more'').
 */
fprintf(stdout, "%s%s%s%s%s%s\n",

("links [options] URL\n"
"Options are:\n"
"\n"
" -g\n"
"  Run in graphics mode.\n"
"\n"
" -no-g\n"
"  Run in text mode (overrides previous -g).\n"
"\n"
" -driver <driver name>\n"
"  Graphics driver to use. Drivers are: x, svgalib, fb, directfb, pmshell,\n"
"    atheos, sdl.\n"
"  List of drivers will be shown if you give it an unknown driver.\n"
"  Available drivers depend on your operating system and available libraries.\n"
"\n"
" -mode <graphics mode>\n"
"  Graphics mode. For SVGALIB it is in format COLUMNSxROWSxCOLORS --\n"
"    for example 640x480x256, 800x600x64k, 960x720x16M, 1024x768x16M32\n"
"    List of modes will be shown if you give it an unknown videomode.\n"
"  For framebuffer it is number of pixels in border --- LEFT,TOP,RIGHT,BOTTOM\n"
"    other 3 values are optional --- i.e. -mode 10 will set all borders to 10,\n"
"    -mode 10,20 will set left & right border to 10 and top & bottom to 20.\n"
"  For Xwindow it is size of a window in format WIDTHxHEIGHT.\n"
"\n"
" -display <x-display>\n"
"  Set Xwindow display.\n"
"\n"
" -force-html\n"
"  Treat files with unknown type as html rather than text.\n"
"    (can be toggled with '\\' key)\n"
"\n"
" -source <url>\n"
"  Write unformatted data stream to stdout.\n"
"\n"
" -dump <url>\n"
"  Write formatted document to stdout.\n"
"\n"
" -width <number>\n"
"  For dump, document will be formatted to this screen width (but it can still\n"
"    exceed it if lines can't be broken).\n"
"\n"
" -codepage <codepage>\n"
"  For dump, convert output to specified character set --\n"
"    for eaxmple iso-8859-2, windows-1250.\n"
"\n"
" -anonymous\n"
"  Restrict links so that it can run on an anonymous account.\n"
"  No local file browsing. No downloads. Executing viewers\n"
"    is allowed, but user can't add or modify entries in\n"
"    association table.\n"
"\n"
" -no-connect\n"
"  Runs links as a separate instance - instead of connecting to\n"
"    existing instance.\n"
"\n"
" -download-utime <0>/<1>\n"
"  Set time of downloaded files to last modification time reported by server.\n"
"\n"
" -async-dns <0>/<1>\n"
"  Asynchronous DNS resolver on(1)/off(0).\n"
"\n"
" -max-connections <max>\n"
"  Maximum number of concurrent connections.\n"
"    (default: 10)\n"
"\n"),
(" -max-connections-to-host <max>\n"
"  Maximum number of concurrent connection to a given host.\n"
"    (default: 2)\n"
"\n"
" -retries <retry>\n"
"  Number of retries.\n"
"    (default: 3)\n"
"\n"
" -receive-timeout <sec>\n"
"  Timeout on receive.\n"
"    (default: 120)\n"
"\n"),
(" -unrestartable-receive-timeout <sec>\n"
"  Timeout on non restartable connections.\n"
"    (default: 600)\n"
"\n"
" -format-cache-size <num>\n"
"  Number of formatted document pages cached.\n"
"    (default: 5)\n"
"\n"
" -memory-cache-size <bytes>\n"
"  Cache memory in bytes.\n"
"    (default: 1048576)\n"
"\n"
" -image-cache-size <bytes>\n"
"  Cache memory in bytes.\n"
"    (default: 1048576)\n"
"\n"),
(" -http-proxy <host:port>\n"
"  Host and port number of the HTTP proxy, or blank.\n"
"    (default: blank)\n"
"\n"
" -ftp-proxy <host:port>\n"
"  Host and port number of the FTP proxy, or blank.\n"
"    (default: blank)\n"
"\n"
" -download-dir <path>\n"
"  Default download directory.\n"
"    (default: actual dir)\n"
"\n"),
(" -language <language>\n"
"  Set user interface language.\n"
"\n"
" -http-bugs.http10 <0>/<1>\n"
"    (default 0)\n"
"  \"1\" forces using only HTTP/1.0 protocol. (useful for buggy servers\n"
"    that claim to be HTTP/1.1 compliant but are not)\n"
"  \"0\" use both HTTP/1.0 and HTTP/1.1.\n"
"\n"
" -http-bugs.allow-blacklist <0>/<1>\n"
"    (default 1)\n"
"  \"1\" defaults to using list of servers that have broken HTTP/1.1 support.\n"
"     When links finds such server, it will retry the request with HTTP/1.0.\n"
"\n"
" -http-bugs.bug-302-redirect <0>/<1>\n"
"    (default 1)\n"
"  Process 302 redirect in a way that is incompatible with RFC1945 and RFC2068,\n"
"    but the same as Netscape and MSIE. Many pages depend on it.\n"
"\n"
" -http-bugs.bug-post-no-keepalive <0>/<1>\n"
"    (default 0)\n"
"  No keepalive connection after post requests. For some buggy servers.\n"
"\n"
" -http-bugs.bug-no-accept-charset <0>/<1>\n"
"    (default 0)\n"
"  Do not send Accept-Charset field of HTTP header. Because it is too long\n"
"    some servers will deny the request. Other servers will convert content\n"
"    to plain ascii when Accept-Charset is missing.\n"
"\n"
" -http-bugs.retry-internal-errors <0>/<1>\n"
"    (default 0)\n"
"  Retry on internal server errors (50x).\n"
"\n"
" -http-bugs.aggressive-cache <0>/<1>\n"
"    (default 1)\n"
"  Always cache everything regardless of server's caching recomendations.\n"
"    Many servers deny caching even if their content is not changing\n"
"    just to get more hits and more money from ads.\n"
"\n"
" -http-referer <0>/<1>/<2>/<3>\n"
"    (default 0)\n"
"  0 - do not send referer\n"
"  1 - send the requested URL as referer\n"
"  2 - send fake referer\n"
"  3 - send real referer\n"
"  4 - send real referer only to the same server\n"
"\n"
" -fake-referer <string>\n"
"  Fake referer value.\n"
"\n"
" -fake-user-agent <string>\n"
"  Fake user agent value.\n"
"\n"
" -ftp.anonymous-password <string>\n"
"  Password for anonymous ftp access.\n"
"\n"
" -ftp.use-passive <0>/<1>\n"
"  Use ftp PASV command to bypass firewalls.\n"
"\n"
" -ftp.fast <0>/<1>\n"
"  Send more ftp commands simultaneously. Faster response when\n"
"    browsing ftp directories, but it is incompatible with RFC\n"
"    and some servers don't like it.\n"
"\n"
" -ftp.set-iptos <0>/<1>\n"
"  Set IP Type-of-service to high throughput on ftp connections.\n"
"\n"
" -menu-font-size <size>\n"
"  Size of font in menu.\n"
"\n"
" -background-color 0xRRGGBB\n"
"  Set menu background color in graphics mode, RRGGBB are hex.\n"
"\n"
" -foreground-color 0xRRGGBB\n"
"  Set menu foreground color in graphics mode.\n"
"\n"
" -scroll-bar-area-color 0xRRGGBB\n"
"  Set color of scroll bar area.\n"
"\n"
" -scroll-bar-bar-color 0xRRGGBB\n"
"  Set color of scroll bar.\n"
"\n"
" -scroll-bar-frame-color 0xRRGGBB\n"
"  Set color of scroll bar frame.\n"
"\n"
" -display-red-gamma <fp-value>\n"
"  Red gamma of display.\n"
"    (default 2.2)\n"
"\n"
" -display-green-gamma <fp-value>\n"
"  Green gamma of display.\n"
"    (default 2.2)\n"
"\n"
" -display-blue-gamma <fp-value>\n"
"  Blue gamma of display.\n"
"    (default 2.2)\n"
"\n"
" -user-gamma <fp-value>\n"
"  Additional gamma.\n"
"    (default 1)\n"
"\n"
" -bfu-aspect <fp-value>\n"
"  Display aspect ration.\n"
"\n"
" -aspect-on <0>/<1>\n"
"  Enable aspect ratio correction.\n"
"\n"
" -dither-letters <0>/<1>\n"
"  Do letter dithering.\n"
"\n"
" -dither-images <0>/<1>\n"
"  Do image dithering.\n"
"\n"
" -display-optimize <0>/<1>/<2>\n"
"  Optimize for CRT (0), LCD RGB (1), LCD BGR (2).\n"
"\n"
" -enable-javascript <0>/<1>\n"
"  Enable javascript.\n"
"\n"
" -js.verbose-errors <0>/<1>\n"
"  Display javascript errors.\n"
"\n"
" -js.verbose-warnings <0>/<1>\n"
"  Display javascript warnings.\n"
"\n"
" -js.enable-all-conversions <0>/<1>\n"
"  Enable conversions between all types in javascript.\n"
"\n"
" -js.enable-global-resolution <0>/<1>\n"
"  Resolve global names.\n"
"\n"
" -js.manual-confirmation <0>/<1>\n"
"  Ask user to confirm potentially dangerous operations.\n"
"    (opening windows, going to url etc.) Default 1.\n"
"\n"
" -js.recursion-depth <integer>\n"
"  Depth of javascript call stack.\n"
"\n"
" -js.memory-limit <memory amount>\n"
"  Amount of kilobytes the javascript may allocate.\n"
"\n"
" -bookmarks-codepage <codepage>\n"
"  Character set of bookmarks file.\n"
"\n"
" -bookmarks-file <file>\n"
"  File to store bookmarks.\n"
"\n"
" -html-assume-codepage <codepage>\n"
"  If server didn't specify document character set, assume this.\n"
"\n"
" -html-hard-assume <0>/<1>\n"
"  Use always character set from \"-html-assume-codepage\" no matter\n"
"    what server sent.\n"
"\n"
" -html-tables <0>/<1>\n"
"  Render tables. (0) causes tables being rendered like in lynx.\n"
"\n"
" -html-frames <0>/<1>\n"
"  Render frames. (0) causes frames  rendered like in lynx.\n"
"\n"
" -html-images <0>/<1>\n"
"  Display links to unnamed images as [IMG].\n"
"\n"
" -html-image-names <0>/<1>\n"
"  Display filename of an image instead of [IMG].\n"
"\n"
" -html-display-images <0>/<1>\n"
"  Display images in graphics mode.\n"
"\n"
" -html-image-scale <percent>\n"
"  Scale images in graphics mode.\n"
"\n"
" -html-numbered-links <0>/<1>\n"
"  Number links in text mode. Allow quick link selection by typing\n"
"    link number and enter.\n"
"\n"
" -html-table-order <0>/<1>\n"
"  In text mode, walk through table by rows (0) or columns (1).\n"
"\n"
" -html-auto-refresh <0>/<1>\n"
"  Process refresh to other page (1), or display link to that page (0).\n"
"\n"
" -html-target-in-new-window <0>/<1>\n"
"  Allow opening new windows from html.\n"
"\n"
" -html-margin <number of spaces>\n"
"  Margin in text mode.\n"
"\n"
" -html-user-font-size <size>\n"
"  Size of font on pages in graphics mode.\n"
"\n"
" -lookup <hostname>\n"
"  Does name lookup, like command \"host\".\n"
"\n"
" -version\n"
"  Prints the links version number and exit.\n"
"\n"
" -help\n"
"  Prints this help screen\n"
"\n"
"\n"),
("Keys:\n"
"	ESC	  display menu\n"
"	^C	  quit\n"
"	^P	  scroll up\n"
"	^N	  scroll down\n"
"	[, ]	  scroll left, right\n"
"	up, down  select link\n"
"	->, enter follow link\n"
"	<-, z	  go back\n"
"	g	  go to url\n"
"	G	  go to url based on current url\n"
"	^G	  go to url based on current link\n"
"	^R	  reload\n"
"	/	  search\n"
"	?	  search back\n"
"	n	  find next\n"
"	N	  find previous\n"
"	=	  document info\n"
"	\\	  document source\n"
"	|	  HTTP header\n"
"	*	  toggle displaying of image links (text mode)\n"
"	d	  download\n"
"	s	  bookmarks\n"
"	q	  quit or close current window\n"
"	^X	  cut to clipboard\n"
"	^V	  paste from clipboard\n"
"	^K	  cut line (in textarea) or text to the end (in field)\n"
"	^U	  cut all text before cursor\n"
"	^W	  autocomplete url\n"
"	Alt-1 .. Alt-9\n"
"		  switch virtual screens (svgalib and framebuffer)\n"
"\n"
"Keys for braille terminal:\n"
"       arrows	  move the cursor\n"
"       enter	  follow link\n"
"	a	  cursor to status line\n"
"	w	  cursor to title\n"
"	^Y	  next link\n"
"	^T	  previous link\n"
"	y	  next word\n"
"	t	  previous word\n"
"	^O	  next form field entry\n"
));

	fflush(stdout);
	return "";
}

void end_config(void)
{
	struct driver_param *dp;
	foreach(dp,driver_params)
	{
		if (dp->param)mem_free(dp->param);
		if (dp->shell)mem_free(dp->shell);
	}
	free_list(driver_params);
	if (links_home) mem_free(links_home);
}

int ggr = 0;
unsigned char ggr_drv[MAX_STR_LEN] = "";
unsigned char ggr_mode[MAX_STR_LEN] = "";
unsigned char ggr_display[MAX_STR_LEN] = "";

int anonymous = 0;

unsigned char system_name[MAX_STR_LEN];

unsigned char default_target[MAX_STR_LEN] ="";

unsigned char *links_home = NULL;
int first_use = 0;
int created_home = 0;

int no_connect = 0;
int base_session = 0;
int dmp = 0;
int force_html = 0;

int async_lookup = 1;
int download_utime = 0;
int max_connections = 10;
int max_connections_to_host = 8;
int max_tries = 3;
int receive_timeout = 120;
int unrestartable_receive_timeout = 600;

int screen_width = 80;
int dump_codepage = -1;

int max_format_cache_entries = 5;
long memory_cache_size = 1048576;
long image_cache_size = 1048576;

int enable_html_tables = 1;
int enable_html_frames = 1;

struct document_setup dds = { 0, 0, 1, 1, 0, 0, 3, 0, 0, 0, 18, 1, 100, 0 };

struct rgb default_fg = { 191, 191, 191, 0 };
struct rgb default_bg = { 0, 0, 0, 0 };
struct rgb default_link = { 255, 255, 255, 0 };
struct rgb default_vlink = { 255, 255, 0, 0 };

struct rgb default_fg_g = { 0, 0, 0, 0 };
struct rgb default_bg_g = { 192, 192, 192, 0 };
struct rgb default_link_g = { 0, 0, 255, 0 };
struct rgb default_vlink_g = { 0, 0, 128, 0 };

int default_left_margin = HTML_LEFT_MARGIN;

unsigned char http_proxy[MAX_STR_LEN] = "";
unsigned char ftp_proxy[MAX_STR_LEN] = "";
unsigned char no_proxy_for[MAX_STR_LEN] = "";
int js_enable=1;   /* 0=disable javascript */
int js_verbose_errors=0;   /* 1=create dialog on every javascript error, 0=be quiet and continue */
int js_verbose_warnings=0;   /* 1=create dialog on every javascript warning, 0=be quiet and continue */
int js_all_conversions=1;
int js_global_resolve=1;	/* resolvovani v globalnim adresnim prostoru, kdyz BFU vomitne document */
int js_manual_confirmation=1; /* !0==annoying dialog on every goto url etc. */

int display_optimize=0;	/*0=CRT, 1=LCD RGB, 2=LCD BGR */
double bfu_aspect=1; /* 0.1 to 10.0, 1.0 default. >1 makes circle wider */
int aspect_on=1;

unsigned char download_dir[MAX_STR_LEN] = "";

struct ftp_options ftp_options = { "somebody@host.domain", 0, 0, 1 };

/* These are workarounds for some CGI script bugs */
struct http_bugs http_bugs = { 0, 1, 1, 0, 0, 0, 1, "", "", REFERER_NONE };
/*int bug_302_redirect = 0;*/
	/* When got 301 or 302 from POST request, change it to GET
	   - this violates RFC2068, but some buggy message board scripts rely on it */
/*int bug_post_no_keepalive = 0;*/
	/* No keepalive connection after POST request. Some buggy PHP databases report bad
	   results if GET wants to retreive data POSTed in the same connection */

struct option links_options[] = {
	{1, printhelp_cmd, NULL, NULL, 0, 0, NULL, NULL, "?"},
	{1, printhelp_cmd, NULL, NULL, 0, 0, NULL, NULL, "h"},
	{1, printhelp_cmd, NULL, NULL, 0, 0, NULL, NULL, "help"},
	{1, printhelp_cmd, NULL, NULL, 0, 0, NULL, NULL, "-help"},
	{1, lookup_cmd, NULL, NULL, 0, 0, NULL, NULL, "lookup"},
	{1, version_cmd, NULL, NULL, 0, 0, NULL, NULL, "version"},
	{1, set_cmd, NULL, NULL, 0, 0, &no_connect, NULL, "no-connect"},
	{1, set_cmd, NULL, NULL, 0, 0, &anonymous, NULL, "anonymous"},
	{1, set_cmd, NULL, NULL, 0, 0, &ggr, NULL, "g"},
	{1, unset_cmd, NULL, NULL, 0, 0, &ggr, NULL, "no-g"},
	{1, setstr_cmd, NULL, NULL, 0, MAX_STR_LEN, &ggr_drv, NULL, "driver"},
	{1, setstr_cmd, NULL, NULL, 0, MAX_STR_LEN, &default_target, NULL, "target"},
	{1, setstr_cmd, NULL, NULL, 0, MAX_STR_LEN, &ggr_mode, NULL, "mode"},
	{1, setstr_cmd, NULL, NULL, 0, MAX_STR_LEN, &ggr_display, NULL, "display"},
	{1, gen_cmd, num_rd, NULL, 0, MAXINT, &base_session, NULL, "base-session"},
	{1, force_html_cmd, NULL, NULL, 0, 0, NULL, NULL, "force-html"},
	{1, dump_cmd, NULL, NULL, D_SOURCE, 0, NULL, NULL, "source"},
	{1, dump_cmd, NULL, NULL, D_DUMP, 0, NULL, NULL, "dump"},
	{1, gen_cmd, num_rd, NULL, 10, 512, &screen_width, "dump_width", "width" },
	{1, gen_cmd, cp_rd, NULL, 1, 0, &dump_codepage, "dump_codepage", "codepage" },
	{1, gen_cmd, num_rd, num_wr, 0, 1, &async_lookup, "async_dns", "async-dns"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &download_utime, "download_utime", "download-utime"},
	{1, gen_cmd, num_rd, num_wr, 1, 99, &max_connections, "max_connections", "max-connections"},
	{1, gen_cmd, num_rd, num_wr, 1, 99, &max_connections_to_host, "max_connections_to_host", "max-connections-to-host"},
	{1, gen_cmd, num_rd, num_wr, 0, 16, &max_tries, "retries", "retries"},
	{1, gen_cmd, num_rd, num_wr, 1, 1800, &receive_timeout, "receive_timeout", "receive-timeout"},
	{1, gen_cmd, num_rd, num_wr, 1, 1800, &unrestartable_receive_timeout, "unrestartable_receive_timeout", "unrestartable-receive-timeout"},
	{1, gen_cmd, num_rd, num_wr, 0, 256, &max_format_cache_entries, "format_cache_size", "format-cache-size"},
	{1, gen_cmd, num_rd, num_wr, 0, MAXINT, &memory_cache_size, "memory_cache_size", "memory-cache-size"},
	{1, gen_cmd, num_rd, num_wr, 0, MAXINT, &image_cache_size, "image_cache_size", "image-cache-size"},
	{1, gen_cmd, str_rd, str_wr, 0, MAX_STR_LEN, http_proxy, "http_proxy", "http-proxy"},
	{1, gen_cmd, str_rd, str_wr, 0, MAX_STR_LEN, ftp_proxy, "ftp_proxy", "ftp-proxy"},
	{1, gen_cmd, str_rd, str_wr, 0, MAX_STR_LEN, download_dir, "download_dir", "download-dir"},
	{1, gen_cmd, lang_rd, lang_wr, 0, 0, &current_language, "language", "language"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &http_bugs.http10, "http_bugs.http10", "http-bugs.http10"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &http_bugs.allow_blacklist, "http_bugs.allow_blacklist", "http-bugs.allow-blacklist"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &http_bugs.bug_302_redirect, "http_bugs.bug_302_redirect", "http-bugs.bug-302-redirect"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &http_bugs.bug_post_no_keepalive, "http_bugs.bug_post_no_keepalive", "http-bugs.bug-post-no-keepalive"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &http_bugs.no_accept_charset, "http_bugs.no_accept_charset", "http-bugs.bug-no-accept-charset"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &http_bugs.retry_internal_errors, "http_bugs.retry_internal_errors", "http-bugs.retry-internal-errors"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &http_bugs.aggressive_cache, "http_bugs.aggressive_cache", "http-bugs.aggressive-cache"},
	{1, gen_cmd, num_rd, num_wr, 0, 4, &http_bugs.referer, "http_referer", "http-referer"},
	{1, gen_cmd, str_rd, str_wr, 0, MAX_STR_LEN, http_bugs.fake_useragent, "fake_useragent", "fake-user-agent"},
	{1, gen_cmd, str_rd, str_wr, 0, MAX_STR_LEN, http_bugs.fake_referer, "fake_referer", "fake-referer"},
	{1, gen_cmd, str_rd, str_wr, 0, MAX_STR_LEN, ftp_options.anon_pass, "ftp.anonymous_password", "ftp.anonymous-password"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &ftp_options.passive_ftp, "ftp.use_passive", "ftp.use-passive"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &ftp_options.fast_ftp, "ftp.fast", "ftp.fast"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &ftp_options.set_tos, "ftp.set_iptos", "ftp.set-iptos"},
	{1, gen_cmd, num_rd, num_wr, 1, 999, &menu_font_size, "menu_font_size", "menu-font-size"},
	{1, gen_cmd, num_rd, num_wr, 0, 0xffffff, &G_BFU_BG_COLOR, "background_color", "background-color"},
	{1, gen_cmd, num_rd, num_wr, 0, 0xffffff, &G_BFU_FG_COLOR, "foreground_color", "foreground-color"},
	{1, gen_cmd, num_rd, num_wr, 0, 0xffffff, &G_SCROLL_BAR_AREA_COLOR, "scroll_bar_area_color", "scroll-bar-area-color"},
	{1, gen_cmd, num_rd, num_wr, 0, 0xffffff, &G_SCROLL_BAR_BAR_COLOR, "scroll_bar_bar_color", "scroll-bar-bar-color"},
	{1, gen_cmd, num_rd, num_wr, 0, 0xffffff, &G_SCROLL_BAR_FRAME_COLOR, "scroll_bar_frame_color", "scroll-bar-frame-color"},
	{1, gen_cmd, dbl_rd, dbl_wr, 1, 10000, &display_red_gamma, "display_red_gamma", "display-red-gamma"},
	{1, gen_cmd, dbl_rd, dbl_wr, 1, 10000, &display_green_gamma, "display_green_gamma", "display-green-gamma"},
	{1, gen_cmd, dbl_rd, dbl_wr, 1, 10000, &display_blue_gamma, "display_blue_gamma", "display-blue-gamma"},
	{1, gen_cmd, dbl_rd, dbl_wr, 1, 10000, &user_gamma, "user_gamma", "user-gamma"},
	{1, gen_cmd, dbl_rd, dbl_wr, 25, 400, &bfu_aspect, "bfu_aspect", "bfu-aspect"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &aspect_on, "aspect_on", "aspect-on"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &dither_letters, "dither_letters", "dither-letters"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &dither_images, "dither_images", "dither-images"},
	{1, gen_cmd, num_rd, num_wr, 0, 2, &display_optimize, "display_optimize", "display-optimize"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &js_enable, "enable_javascript", "enable-javascript"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &js_verbose_errors, "verbose_javascript_errors", "js.verbose-errors"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &js_verbose_warnings, "verbose_javascript_warnings", "js.verbose-warnings"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &js_all_conversions, "enable_all_conversions", "js.enable-all-conversions"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &js_global_resolve, "enable_global_resolution", "js.enable-global-resolution"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &js_manual_confirmation, "javascript_manual_confirmation", "js.manual-confirmation"},
	{1, gen_cmd, num_rd, num_wr, 0, 999999, &js_fun_depth, "js_recursion_depth", "js.recursion-depth"},
	{1, gen_cmd, num_rd, num_wr, 1024, 30*1024, &js_memory_limit, "js_memory_limit", "js.memory-limit"},
	{1, gen_cmd, cp_rd, cp_wr, 0, 0, &bookmarks_codepage, "bookmarks_codepage", "bookmarks-codepage"},
	{1, gen_cmd, str_rd, str_wr, 0, MAX_STR_LEN, bookmarks_file, "bookmarks_file", "bookmarks-file"},
	{1, gen_cmd, cp_rd, NULL, 0, 0, &dds.assume_cp, "assume_codepage", "assume-codepage"},
	{1, NULL, term_rd, term_wr, 0, 0, NULL, "terminal", NULL},
	{1, NULL, term2_rd, NULL, 0, 0, NULL, "terminal2", NULL},
	{1, NULL, type_rd, type_wr, 0, 0, NULL, "association", NULL},
	{1, NULL, ext_rd, ext_wr, 0, 0, NULL, "extension", NULL},
	{1, NULL, prog_rd, prog_wr, 0, 0, &mailto_prog, "mailto", NULL},
	{1, NULL, prog_rd, prog_wr, 0, 0, &telnet_prog, "telnet", NULL},
	{1, NULL, prog_rd, prog_wr, 0, 0, &tn3270_prog, "tn3270", NULL},
	{1, NULL, prog_rd, prog_wr, 0, 0, &mms_prog, "mms", NULL},
	{1, NULL, block_rd, block_wr, 0, 0, NULL, "imageblock", NULL},
	{1, NULL, dp_rd, dp_wr, 0, 0, NULL, "video_driver", NULL},
	{0, NULL, NULL, NULL, 0, 0, NULL, NULL, NULL},
};

struct option html_options[] = {
	{1, gen_cmd, num_rd, num_wr, 0, 1, &dds.hard_assume, "html_hard_assume", "html-hard-assume"},
	{1, gen_cmd, cp_rd, cp_wr, 0, 0, &dds.assume_cp, "html_assume_codepage", "html-assume-codepage"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &dds.tables, "html_tables", "html-tables"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &dds.frames, "html_frames", "html-frames"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &dds.images, "html_images", "html-images"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &dds.image_names, "html_image_names", "html-image-names"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &dds.display_images, "html_display_images", "html-display-images"},
	{1, gen_cmd, num_rd, num_wr, 1, 500, &dds.image_scale, "html_image_scale", "html-image-scale"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &dds.num_links, "html_numbered_links", "html-numbered-links"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &dds.table_order, "html_table_order", "html-table-order"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &dds.auto_refresh, "html_auto_refresh", "html-auto-refresh"},
	{1, gen_cmd, num_rd, num_wr, 0, 1, &dds.target_in_new_window, "html_target_in_new_window", "html-target-in-new-window"},
	{1, gen_cmd, num_rd, num_wr, 0, 9, &dds.margin, "html_margin", "html-margin"},
	{1, gen_cmd, num_rd, num_wr, 1, 999, &dds.font_size, "html_font_size", "html-user-font-size"},
	{0, NULL, NULL, NULL, 0, 0, NULL, NULL, NULL},
};

void load_url_history(void)
{
	unsigned char *history_file, *hs;
	unsigned char *hsp;

	if (anonymous) return;
	/* Must have been called after init_home */
	if (!links_home) return;
	history_file = stracpy(links_home);
	add_to_strn(&history_file, "links.his");
	hs = read_config_file(history_file);
	mem_free(history_file);
	if (!hs) return;
	for (hsp = hs; *hsp; ) {
		unsigned char *hsl, *hsc;
		for (hsl = hsp; *hsl && *hsl != 10 && *hsl != 13; hsl++) ;
		hsc = memacpy(hsp, hsl - hsp);
		add_to_history(&goto_url_history, hsc);
		mem_free(hsc);
		hsp = hsl;
		while (*hsp == 10 || *hsp == 13) hsp++;
	}
	mem_free(hs);
}

void save_url_history(void)
{
	struct history_item *hi;
	unsigned char *history_file;
	unsigned char *hs;
	int hsl = 0;
	int i = 0;
	if (anonymous) return;

	/* Must have been called after init_home */
	if (!links_home) return;
	history_file = stracpy(links_home);
	add_to_strn(&history_file, "links.his");
	hs = init_str();
	hsl = 0;
	foreachback(hi, goto_url_history.items) {
		if (!*hi->d || strchr(hi->d, 10) || strchr(hi->d, 13)) continue;
		if (i++ > MAX_HISTORY_ITEMS)
			break;
		else {
			add_to_str(&hs, &hsl, hi->d);
			add_to_str(&hs, &hsl, NEWLINE);
		}
	}
	write_to_config_file(history_file, hs);
	mem_free(history_file);
	mem_free(hs);
	return;
}

