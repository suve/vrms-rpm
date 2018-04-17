/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018 Artur "suve" Iwicki
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program (LICENCE.txt). If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VRMS_RPM_BUFFERS_H
#define VRMS_RPM_BUFFERS_H

#define CHAINBUF_SIZEOF    4096
#define CHAINBUF_CAPACITY  (CHAINBUF_SIZEOF - sizeof(int) - sizeof(void*))

struct ChainBuffer {
	char data[CHAINBUF_CAPACITY];
	int used;
	struct ChainBuffer *previous;
};

#define REBUF_SIZEOF   4096
#define REBUF_CAPACITY (REBUF_SIZEOF / sizeof(void*))

struct ReBuffer {
	void **data;
	int capacity;
	int count;
};


struct ChainBuffer* chainbuf_init(void);
void chainbuf_free(struct ChainBuffer *buf);

char* chainbuf_append(struct ChainBuffer **buf, char *data);

struct ReBuffer* rebuf_init(void);
void rebuf_free(struct ReBuffer *buf);

void* rebuf_append(struct ReBuffer *const buf, void *data);

#endif
