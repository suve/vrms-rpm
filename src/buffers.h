/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2020-2021, 2023 suve (a.k.a. Artur Frenszek-Iwicki)
 * Copyright (C) 2023 Marcin "dextero" Radomski
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

#include <string.h>

struct ChainBuffer {
	struct ChainBuffer *previous;
	size_t capacity;
	size_t used;
	char data[];
};

struct ReBuffer {
	void *data;
	size_t step;
	size_t capacity;
	size_t used;
};


extern struct ChainBuffer* chainbuf_init(size_t capacity);
extern void chainbuf_free(struct ChainBuffer *buf);

extern char* chainbuf_append(struct ChainBuffer **buf, const char *data);

extern struct ReBuffer* rebuf_init(size_t stepSize);
extern void rebuf_free(struct ReBuffer *buf);

extern void* rebuf_append(struct ReBuffer *const buf, const void *const data, const size_t dataLength);

#endif
