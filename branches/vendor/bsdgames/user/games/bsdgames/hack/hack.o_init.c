/*	$NetBSD: hack.o_init.c,v 1.7 2003/04/02 18:36:38 jsm Exp $	*/

/*
 * Copyright (c) 1985, Stichting Centrum voor Wiskunde en Informatica,
 * Amsterdam
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Stichting Centrum voor Wiskunde en
 * Informatica, nor the names of its contributors may be used to endorse or
 * promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1982 Jay Fenlason <hack@gnu.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
__RCSID("$NetBSD: hack.o_init.c,v 1.7 2003/04/02 18:36:38 jsm Exp $");
#endif				/* not lint */

#include <string.h>
#include "hack.h"
#include "extern.h"
#include "def.objects.h"
#include "hack.onames.h"	/* for LAST_GEM */

int
letindex(let)
	char            let;
{
	int             i = 0;
	char            ch;
	while ((ch = obj_symbols[i++]) != 0)
		if (ch == let)
			return (i);
	return (0);
}

void
init_objects()
{
	int             i, j, first, last, sum, end;
	char            let;
	const char *tmp;
	/*
	 * init base; if probs given check that they add up to 100, otherwise
	 * compute probs; shuffle descriptions
	 */
	end = SIZE(objects);
	first = 0;
	while (first < end) {
		let = objects[first].oc_olet;
		last = first + 1;
		while (last < end && objects[last].oc_olet == let
		       && objects[last].oc_name != NULL)
			last++;
		i = letindex(let);
		if ((!i && let != ILLOBJ_SYM) || bases[i] != 0)
			error("initialization error");
		bases[i] = first;

		if (let == GEM_SYM)
			setgemprobs();
check:
		sum = 0;
		for (j = first; j < last; j++)
			sum += objects[j].oc_prob;
		if (sum == 0) {
			for (j = first; j < last; j++)
				objects[j].oc_prob = (100 + j - first) / (last - first);
			goto check;
		}
		if (sum != 100)
			error("init-prob error for %c", let);

		if (objects[first].oc_descr != NULL && let != TOOL_SYM) {
			/* shuffle, also some additional descriptions */
			while (last < end && objects[last].oc_olet == let)
				last++;
			j = last;
			while (--j > first) {
				i = first + rn2(j + 1 - first);
				tmp = objects[j].oc_descr;
				objects[j].oc_descr = objects[i].oc_descr;
				objects[i].oc_descr = tmp;
			}
		}
		first = last;
	}
}

int
probtype(let)
	char            let;
{
	int             i = bases[letindex(let)];
	int             prob = rn2(100);
	while ((prob -= objects[i].oc_prob) >= 0)
		i++;
	if (objects[i].oc_olet != let || !objects[i].oc_name)
		panic("probtype(%c) error, i=%d", let, i);
	return (i);
}

void
setgemprobs()
{
	int             j, first;

	first = bases[letindex(GEM_SYM)];

	for (j = 0; j < 9 - dlevel / 3; j++)
		objects[first + j].oc_prob = 0;
	first += j;
	if (first >= LAST_GEM || first >= SIZE(objects) ||
	    objects[first].oc_olet != GEM_SYM ||
	    objects[first].oc_name == NULL)
		printf("Not enough gems? - first=%d j=%d LAST_GEM=%d\n",
		       first, j, LAST_GEM);
	for (j = first; j < LAST_GEM; j++)
		objects[j].oc_prob = (20 + j - first) / (LAST_GEM - first);
}

void
oinit()
{				/* level dependent initialization */
	setgemprobs();
}

void
savenames(fd)
	int             fd;
{
	int             i;
	unsigned        len;
	bwrite(fd, (char *) bases, sizeof bases);
	bwrite(fd, (char *) objects, sizeof objects);
	/*
	 * as long as we use only one version of Hack/Quest we need not save
	 * oc_name and oc_descr, but we must save oc_uname for all objects
	 */
	for (i = 0; i < SIZE(objects); i++) {
		if (objects[i].oc_uname) {
			len = strlen(objects[i].oc_uname) + 1;
			bwrite(fd, (char *) &len, sizeof len);
			bwrite(fd, objects[i].oc_uname, len);
		}
	}
}

void
restnames(fd)
	int             fd;
{
	int             i;
	unsigned        len;
	mread(fd, (char *) bases, sizeof bases);
	mread(fd, (char *) objects, sizeof objects);
	for (i = 0; i < SIZE(objects); i++)
		if (objects[i].oc_uname) {
			mread(fd, (char *) &len, sizeof len);
			objects[i].oc_uname = (char *) alloc(len);
			mread(fd, objects[i].oc_uname, len);
		}
}

int
dodiscovered()
{				/* free after Robert Viduya */
	int             i, end;
	int             ct = 0;

	cornline(0, "Discoveries");

	end = SIZE(objects);
	for (i = 0; i < end; i++) {
		if (interesting_to_discover(i)) {
			ct++;
			cornline(1, typename(i));
		}
	}
	if (ct == 0) {
		pline("You haven't discovered anything yet...");
		cornline(3, (char *) 0);
	} else
		cornline(2, (char *) 0);

	return (0);
}

int
interesting_to_discover(i)
	int             i;
{
	return (
		objects[i].oc_uname != NULL ||
		(objects[i].oc_name_known && objects[i].oc_descr != NULL)
		);
}
