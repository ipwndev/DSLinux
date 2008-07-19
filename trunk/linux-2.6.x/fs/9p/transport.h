/*
 * linux/fs/9p/transport.h
 *
 * Transport Definition
 *
 *  Copyright (C) 2004 by Eric Van Hensbergen <ericvh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to:
 *  Free Software Foundation
 *  51 Franklin Street, Fifth Floor
 *  Boston, MA  02111-1301  USA
 *
 */

enum v9fs_transport_status {
	Connected,
	Disconnected,
	Hung,
};

struct v9fs_transport {
	enum v9fs_transport_status status;
	struct semaphore writelock;
	struct semaphore readlock;
	void *priv;

	int (*init) (struct v9fs_session_info *, const char *, char *);
	int (*write) (struct v9fs_transport *, void *, int);
	int (*read) (struct v9fs_transport *, void *, int);
	void (*close) (struct v9fs_transport *);
};

extern struct v9fs_transport v9fs_trans_tcp;
extern struct v9fs_transport v9fs_trans_unix;
extern struct v9fs_transport v9fs_trans_fd;
