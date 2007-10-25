/******************************************************************************
*   TinTin++                                                                  *
*   Copyright (C) 2004 (See CREDITS file)                                     *
*                                                                             *
*   This program is protected under the GNU GPL (See COPYING)                 *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
******************************************************************************/

/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/

#include "tintin.h"

struct str_hash_index_data str_hash_index[MAX_STR_HASH];

unsigned short str_hash_len;

/*
	Performs as well as Jenkin's one-at-a-time hash.- Igor
*/

unsigned short generate_hash_key(char *str)
{
	unsigned int h;

	for (h = str_hash_len = 0 ; *str != 0 ; str++, str_hash_len++)
	{
		h -= (4 - (h >> 7)) * *str;
	}

	h += str_hash_len;

	return h % MAX_STR_HASH;
}

 
char *str_hash(char *str, int lines)
{
	unsigned short hash;
	struct str_hash_data *hash_ptr;

	hash = generate_hash_key(str);

	for (hash_ptr = str_hash_index[hash].f_node ; hash_ptr ; hash_ptr = hash_ptr->next)
	{
		if (!strcmp(str, (char *) hash_ptr + gtd->str_hash_size))
		{
			hash_ptr->count++;

			return (char *) hash_ptr + gtd->str_hash_size;
		}
	}
	hash_ptr = calloc(1, str_hash_len + 1 + gtd->str_hash_size);

	hash_ptr->count = 1;
	hash_ptr->lines = lines;
	hash_ptr->hash  = hash;
	strcpy((char *) hash_ptr + gtd->str_hash_size, str);

	LINK(hash_ptr, str_hash_index[hash].f_node, str_hash_index[hash].l_node);

	return (char *) hash_ptr + gtd->str_hash_size;
}


char *str_unhash(char *str)
{
	struct str_hash_data *hash_ptr;

	hash_ptr = (struct str_hash_data *) (str - gtd->str_hash_size);

	if (--(hash_ptr->count) == 0)
	{
		UNLINK(hash_ptr, str_hash_index[hash_ptr->hash].f_node, str_hash_index[hash_ptr->hash].l_node);
		free(hash_ptr);
	}
	return NULL;
}


unsigned short str_hash_lines(char *str)
{
	return ((struct str_hash_data *) (str - gtd->str_hash_size))->lines;
}

unsigned short str_hash_grep(char *str, int write)
{
	struct str_hash_data *hash_ptr;

	hash_ptr = ((struct str_hash_data *) (str - gtd->str_hash_size));

	if (write)
	{
		SET_BIT(hash_ptr->flags, STR_HASH_FLAG_NOGREP);
	}

	return HAS_BIT(hash_ptr->flags, STR_HASH_FLAG_NOGREP);
}

void reset_hash_table(void)
{
	char temp[BUFFER_SIZE];
	struct str_hash_data *hash_ptr;
	int hash;

	for (hash = 0 ; hash < MAX_STR_HASH ; hash++)
	{
		for (hash_ptr = str_hash_index[hash].f_node ; hash_ptr ; hash_ptr = hash_ptr->next)
		{
			hash_ptr->lines = word_wrap(gtd->ses, (char *) hash_ptr + gtd->str_hash_size, temp, FALSE);
		}
	}
}

DO_COMMAND(do_hash)
{
	struct str_hash_data *hash_ptr;
	int hash, cnt_hash, max_hash, cnt_used, cnt_str, cnt_count, cnt_size, cnt_unhashed_size;

	hash = cnt_hash = max_hash = cnt_used = cnt_str = cnt_count = cnt_size = cnt_unhashed_size = 0;

	for (hash = 0 ; hash < MAX_STR_HASH ; hash++)
	{
		if (str_hash_index[hash].f_node)
		{
			cnt_used++;
		}

		for (cnt_hash = 0, hash_ptr = str_hash_index[hash].f_node ; hash_ptr ; hash_ptr = hash_ptr->next)
		{
			if (hash != generate_hash_key((char *) hash_ptr + gtd->str_hash_size))
			{
				tintin_printf2(ses, "corrupted hash node: %s", (char *) hash_ptr + gtd->str_hash_size);
			}
			cnt_hash          += 1;
			cnt_str           += 1;
			cnt_count         += hash_ptr->count;
			cnt_size          += gtd->str_hash_size + strlen((char *) hash_ptr + gtd->str_hash_size);
			cnt_unhashed_size += hash_ptr->count    * strlen((char *) hash_ptr + gtd->str_hash_size);
		}
		if (cnt_hash > max_hash)
		{
			max_hash = cnt_hash;
		}
	}
	tintin_printf2(NULL, "Strings Allocated: %8d", cnt_str);
	tintin_printf2(NULL, "Total Links:       %8d", cnt_count);
	tintin_printf2(NULL, "Max Hash:          %8d", max_hash);
	tintin_printf2(NULL, "Used Hash:         %8d", cnt_used);
	tintin_printf2(NULL, "Average Hash:      %8d", cnt_str / cnt_used);
	tintin_printf2(NULL, "Total Memory:      %8d", cnt_size);
	tintin_printf2(NULL, "Saved Memory:      %8d", cnt_unhashed_size - cnt_size);

	return ses;
}
