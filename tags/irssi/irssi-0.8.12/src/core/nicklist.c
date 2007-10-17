/*
 nicklist.c : irssi

    Copyright (C) 1999-2000 Timo Sirainen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "module.h"
#include "signals.h"
#include "misc.h"

#include "servers.h"
#include "channels.h"
#include "nicklist.h"
#include "masks.h"

#define isalnumhigh(a) \
        (i_isalnum(a) || (unsigned char) (a) >= 128)

static void nick_hash_add(CHANNEL_REC *channel, NICK_REC *nick)
{
	NICK_REC *list;

	nick->next = NULL;

	list = g_hash_table_lookup(channel->nicks, nick->nick);
        if (list == NULL)
		g_hash_table_insert(channel->nicks, nick->nick, nick);
	else {
                /* multiple nicks with same name */
		while (list->next != NULL)
			list = list->next;
		list->next = nick;
	}

	if (nick == channel->ownnick) {
                /* move our own nick to beginning of the nick list.. */
		nicklist_set_own(channel, nick);
	}
}

static void nick_hash_remove(CHANNEL_REC *channel, NICK_REC *nick)
{
	NICK_REC *list;

	list = g_hash_table_lookup(channel->nicks, nick->nick);
	if (list == NULL)
		return;

	if (list == nick || list->next == NULL) {
		g_hash_table_remove(channel->nicks, nick->nick);
		if (list->next != NULL) {
			g_hash_table_insert(channel->nicks, nick->next->nick,
					    nick->next);
		}
	} else {
		while (list->next != nick)
			list = list->next;
		list->next = nick->next;
	}
}

/* Add new nick to list */
void nicklist_insert(CHANNEL_REC *channel, NICK_REC *nick)
{
	/*MODULE_DATA_INIT(nick);*/

	nick->type = module_get_uniq_id("NICK", 0);
        nick->chat_type = channel->chat_type;

        nick_hash_add(channel, nick);
	signal_emit("nicklist new", 2, channel, nick);
}

/* Set host address for nick */
void nicklist_set_host(CHANNEL_REC *channel, NICK_REC *nick, const char *host)
{
        g_return_if_fail(channel != NULL);
        g_return_if_fail(nick != NULL);
	g_return_if_fail(host != NULL);

        g_free_not_null(nick->host);
	nick->host = g_strdup(host);

        signal_emit("nicklist host changed", 2, channel, nick);
}

static void nicklist_destroy(CHANNEL_REC *channel, NICK_REC *nick)
{
	signal_emit("nicklist remove", 2, channel, nick);

	if (channel->ownnick == nick)
                channel->ownnick = NULL;

        /*MODULE_DATA_DEINIT(nick);*/
	g_free(nick->nick);
	g_free_not_null(nick->realname);
	g_free_not_null(nick->host);
	g_free(nick);
}

/* Remove nick from list */
void nicklist_remove(CHANNEL_REC *channel, NICK_REC *nick)
{
	g_return_if_fail(IS_CHANNEL(channel));
	g_return_if_fail(nick != NULL);

        nick_hash_remove(channel, nick);
	nicklist_destroy(channel, nick);
}

static void nicklist_rename_list(SERVER_REC *server, void *new_nick_id,
				 const char *old_nick, const char *new_nick,
				 GSList *nicks)
{
	CHANNEL_REC *channel;
	NICK_REC *nickrec;
	GSList *tmp;

	for (tmp = nicks; tmp != NULL; tmp = tmp->next->next) {
		channel = tmp->data;
		nickrec = tmp->next->data;

		/* remove old nick from hash table */
                nick_hash_remove(channel, nickrec);

		if (new_nick_id != NULL)
			nickrec->unique_id = new_nick_id;

		g_free(nickrec->nick);
		nickrec->nick = g_strdup(new_nick);

		/* add new nick to hash table */
                nick_hash_add(channel, nickrec);

		signal_emit("nicklist changed", 3, channel, nickrec, old_nick);
	}
	g_slist_free(nicks);
}

void nicklist_rename(SERVER_REC *server, const char *old_nick,
		     const char *new_nick)
{
	nicklist_rename_list(server, NULL, old_nick, new_nick,
			     nicklist_get_same(server, old_nick));
}

void nicklist_rename_unique(SERVER_REC *server,
			    void *old_nick_id, const char *old_nick,
			    void *new_nick_id, const char *new_nick)
{
	nicklist_rename_list(server, new_nick_id, old_nick, new_nick,
			     nicklist_get_same_unique(server, old_nick_id));
}

static NICK_REC *nicklist_find_wildcards(CHANNEL_REC *channel,
					 const char *mask)
{
	GSList *nicks, *tmp;
	NICK_REC *nick;

	nicks = nicklist_getnicks(channel);
	nick = NULL;
	for (tmp = nicks; tmp != NULL; tmp = tmp->next) {
		nick = tmp->data;

		if (mask_match_address(channel->server, mask,
				       nick->nick, nick->host))
			break;
	}
	g_slist_free(nicks);
	return tmp == NULL ? NULL : nick;
}

GSList *nicklist_find_multiple(CHANNEL_REC *channel, const char *mask)
{
	GSList *nicks, *tmp, *next;

	g_return_val_if_fail(IS_CHANNEL(channel), NULL);
	g_return_val_if_fail(mask != NULL, NULL);

	nicks = nicklist_getnicks(channel);
	for (tmp = nicks; tmp != NULL; tmp = next) {
		NICK_REC *nick = tmp->data;

		next = tmp->next;
		if (!mask_match_address(channel->server, mask,
					nick->nick, nick->host))
                        nicks = g_slist_remove(nicks, tmp->data);
	}

	return nicks;
}

/* Find nick */
NICK_REC *nicklist_find(CHANNEL_REC *channel, const char *nick)
{
	g_return_val_if_fail(IS_CHANNEL(channel), NULL);
	g_return_val_if_fail(nick != NULL, NULL);

	return g_hash_table_lookup(channel->nicks, nick);
}

NICK_REC *nicklist_find_unique(CHANNEL_REC *channel, const char *nick,
			       void *id)
{
	NICK_REC *rec;

	g_return_val_if_fail(IS_CHANNEL(channel), NULL);
	g_return_val_if_fail(nick != NULL, NULL);

	rec = g_hash_table_lookup(channel->nicks, nick);
	while (rec != NULL && rec->unique_id != id)
                rec = rec->next;

        return rec;
}

/* Find nick mask, wildcards allowed */
NICK_REC *nicklist_find_mask(CHANNEL_REC *channel, const char *mask)
{
	NICK_REC *nickrec;
	char *nick, *host;

	g_return_val_if_fail(IS_CHANNEL(channel), NULL);
	g_return_val_if_fail(mask != NULL, NULL);

	nick = g_strdup(mask);
	host = strchr(nick, '!');
	if (host != NULL) *host++ = '\0';

	if (strchr(nick, '*') || strchr(nick, '?')) {
		g_free(nick);
		return nicklist_find_wildcards(channel, mask);
	}

	nickrec = g_hash_table_lookup(channel->nicks, nick);

	if (host != NULL) {
		while (nickrec != NULL) {
			if (nickrec->host != NULL &&
			    match_wildcards(host, nickrec->host))
				break; /* match */
			nickrec = nickrec->next;
		}
	}
	g_free(nick);
	return nickrec;
}

static void get_nicks_hash(gpointer key, NICK_REC *rec, GSList **list)
{
	while (rec != NULL) {
		*list = g_slist_append(*list, rec);
                rec = rec->next;
	}
}

/* Get list of nicks */
GSList *nicklist_getnicks(CHANNEL_REC *channel)
{
	GSList *list;

	g_return_val_if_fail(IS_CHANNEL(channel), NULL);

	list = NULL;
	g_hash_table_foreach(channel->nicks, (GHFunc) get_nicks_hash, &list);
	return list;
}

typedef struct {
        CHANNEL_REC *channel;
	const char *nick;
	GSList *list;
} NICKLIST_GET_SAME_REC;

static void get_nicks_same_hash(gpointer key, NICK_REC *nick,
				NICKLIST_GET_SAME_REC *rec)
{
	while (nick != NULL) {
		if (g_strcasecmp(nick->nick, rec->nick) == 0) {
			rec->list = g_slist_append(rec->list, rec->channel);
			rec->list = g_slist_append(rec->list, nick);
		}

		nick = nick->next;
	}
}

GSList *nicklist_get_same(SERVER_REC *server, const char *nick)
{
	NICKLIST_GET_SAME_REC rec;
	GSList *tmp;

	g_return_val_if_fail(IS_SERVER(server), NULL);

	rec.nick = nick;
	rec.list = NULL;
	for (tmp = server->channels; tmp != NULL; tmp = tmp->next) {
		rec.channel = tmp->data;
		g_hash_table_foreach(rec.channel->nicks,
				     (GHFunc) get_nicks_same_hash, &rec);
	}
	return rec.list;
}

typedef struct {
	CHANNEL_REC *channel;
        void *id;
	GSList *list;
} NICKLIST_GET_SAME_UNIQUE_REC;

static void get_nicks_same_hash_unique(gpointer key, NICK_REC *nick,
				       NICKLIST_GET_SAME_UNIQUE_REC *rec)
{
	while (nick != NULL) {
		if (nick->unique_id == rec->id) {
			rec->list = g_slist_append(rec->list, rec->channel);
			rec->list = g_slist_append(rec->list, nick);
                        break;
		}

                nick = nick->next;
	}
}

GSList *nicklist_get_same_unique(SERVER_REC *server, void *id)
{
	NICKLIST_GET_SAME_UNIQUE_REC rec;
	GSList *tmp;

	g_return_val_if_fail(IS_SERVER(server), NULL);
	g_return_val_if_fail(id != NULL, NULL);

        rec.id = id;
	rec.list = NULL;
	for (tmp = server->channels; tmp != NULL; tmp = tmp->next) {
		rec.channel = tmp->data;
		g_hash_table_foreach(rec.channel->nicks,
				     (GHFunc) get_nicks_same_hash_unique,
				     &rec);
	}
	return rec.list;
}

/* nick record comparision for sort functions */
int nicklist_compare(NICK_REC *p1, NICK_REC *p2, const char *nick_prefix)
{
	int status1, status2;
	
	if (p1 == NULL) return -1;
	if (p2 == NULL) return 1;

	/* we assign each status (op, halfop, voice, normal) a number
	 * and compare them. this is easier than 100,000 if's and
	 * returns :-)
	 * -- yath */

	if (p1->other) {
		const char *other = (nick_prefix == NULL) ? NULL : strchr(nick_prefix, p1->other);
		status1 = (other == NULL) ? 5 : 1000 - (other - nick_prefix);
	} else if (p1->op)
		status1 = 4;
	else if (p1->halfop)
		status1 = 3;
	else if (p1->voice)
		status1 = 2;
	else
		status1 = 1;

	if (p2->other) {
		const char *other = (nick_prefix == NULL) ? NULL : strchr(nick_prefix, p2->other);
		status2 = (other == NULL) ? 5 : 1000 - (other - nick_prefix);
	} else if (p2->op)
		status2 = 4;
	else if (p2->halfop)
		status2 = 3;
	else if (p2->voice)
		status2 = 2;
	else
		status2 = 1;
	
	if (status1 < status2)
		return 1;
	else if (status1 > status2)
		return -1;
	
	return g_strcasecmp(p1->nick, p2->nick);
}

static void nicklist_update_flags_list(SERVER_REC *server, int gone,
				       int serverop, GSList *nicks)
{
	GSList *tmp;
	CHANNEL_REC *channel;
	NICK_REC *rec;

	g_return_if_fail(IS_SERVER(server));

	for (tmp = nicks; tmp != NULL; tmp = tmp->next->next) {
		channel = tmp->data;
		rec = tmp->next->data;

		rec->last_check = time(NULL);

		if (gone != -1 && (int)rec->gone != gone) {
			rec->gone = gone;
			signal_emit("nicklist gone changed", 2, channel, rec);
		}

		if (serverop != -1 && (int)rec->serverop != serverop) {
			rec->serverop = serverop;
			signal_emit("nicklist serverop changed", 2, channel, rec);
		}
	}
	g_slist_free(nicks);
}

void nicklist_update_flags(SERVER_REC *server, const char *nick,
			   int gone, int serverop)
{
	nicklist_update_flags_list(server, gone, serverop,
				   nicklist_get_same(server, nick));
}

void nicklist_update_flags_unique(SERVER_REC *server, void *id,
				  int gone, int serverop)
{
	nicklist_update_flags_list(server, gone, serverop,
				   nicklist_get_same_unique(server, id));
}

/* Specify which nick in channel is ours */
void nicklist_set_own(CHANNEL_REC *channel, NICK_REC *nick)
{
	NICK_REC *first, *next;

        channel->ownnick = nick;

	/* move our nick in the list to first, makes some things easier
	   (like handling multiple identical nicks in fe-messages.c) */
	first = g_hash_table_lookup(channel->nicks, nick->nick);
	if (first->next == NULL)
		return;

	next = nick->next;
	nick->next = first;

	while (first->next != nick)
                first = first->next;
	first->next = next;

        g_hash_table_insert(channel->nicks, nick->nick, nick);
}

static void sig_channel_created(CHANNEL_REC *channel)
{
	g_return_if_fail(IS_CHANNEL(channel));

	channel->nicks = g_hash_table_new((GHashFunc) g_istr_hash,
					  (GCompareFunc) g_istr_equal);
}

static void nicklist_remove_hash(gpointer key, NICK_REC *nick,
				 CHANNEL_REC *channel)
{
	NICK_REC *next;

	while (nick != NULL) {
                next = nick->next;
		nicklist_destroy(channel, nick);
                nick = next;
	}
}

static void sig_channel_destroyed(CHANNEL_REC *channel)
{
	g_return_if_fail(IS_CHANNEL(channel));

	g_hash_table_foreach(channel->nicks,
			     (GHFunc) nicklist_remove_hash, channel);
	g_hash_table_destroy(channel->nicks);
}

static NICK_REC *nick_nfind(CHANNEL_REC *channel, const char *nick, int len)
{
        NICK_REC *rec;
	char *tmpnick;

	tmpnick = g_strndup(nick, len);
	rec = g_hash_table_lookup(channel->nicks, tmpnick);

	if (rec != NULL) {
		/* if there's multiple, get the one with identical case */
		while (rec->next != NULL) {
			if (strcmp(rec->nick, tmpnick) == 0)
				break;
                        rec = rec->next;
		}
	}

        g_free(tmpnick);
	return rec;
}

/* Check is `msg' is meant for `nick'. */
int nick_match_msg(CHANNEL_REC *channel, const char *msg, const char *nick)
{
	const char *msgstart, *orignick;
	int len, fullmatch;

	g_return_val_if_fail(nick != NULL, FALSE);
	g_return_val_if_fail(msg != NULL, FALSE);

	if (channel != NULL && channel->server->nick_match_msg != NULL)
		return channel->server->nick_match_msg(msg, nick);

	/* first check for identical match */
	len = strlen(nick);
	if (g_strncasecmp(msg, nick, len) == 0 && !isalnumhigh((int) msg[len]))
		return TRUE;

	orignick = nick;
	for (;;) {
		nick = orignick;
		msgstart = msg;
                fullmatch = TRUE;

		/* check if it matches for alphanumeric parts of nick */
		while (*nick != '\0' && *msg != '\0') {
			if (i_toupper(*nick) == i_toupper(*msg)) {
				/* total match */
				msg++;
			} else if (i_isalnum(*msg) && !i_isalnum(*nick)) {
				/* some strange char in your nick, pass it */
                                fullmatch = FALSE;
			} else
				break;

			nick++;
		}

		if (msg != msgstart && !isalnumhigh(*msg)) {
			/* at least some of the chars in line matched the
			   nick, and msg continue with non-alphanum character,
			   this might be for us.. */
			if (*nick != '\0') {
				/* remove the rest of the non-alphanum chars
				   from nick and check if it then matches. */
                                fullmatch = FALSE;
				while (*nick != '\0' && !i_isalnum(*nick))
					nick++;
			}

			if (*nick == '\0') {
				/* yes, match! */
                                break;
			}
		}

		/* no match. check if this is a message to multiple people
		   (like nick1,nick2: text) */
		while (*msg != '\0' && *msg != ' ' && *msg != ',') msg++;

		if (*msg != ',') {
                        nick = orignick;
			break;
		}

                msg++;
	}

	if (*nick != '\0')
		return FALSE; /* didn't match */

	if (fullmatch)
		return TRUE; /* matched without fuzzyness */

	/* matched with some fuzzyness .. check if there's an exact match
	   for some other nick in the same channel. */
        return nick_nfind(channel, msgstart, (int) (msg-msgstart)) == NULL;
}

void nicklist_init(void)
{
	signal_add_first("channel created", (SIGNAL_FUNC) sig_channel_created);
	signal_add("channel destroyed", (SIGNAL_FUNC) sig_channel_destroyed);
}

void nicklist_deinit(void)
{
	signal_remove("channel created", (SIGNAL_FUNC) sig_channel_created);
	signal_remove("channel destroyed", (SIGNAL_FUNC) sig_channel_destroyed);

	module_uniq_destroy("NICK");
}
