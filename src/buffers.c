/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2021 Artur "suve" Iwicki
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
#include <stdlib.h>
#include <string.h>

#include "buffers.h"

_Static_assert(sizeof(struct ChainBuffer) == CHAINBUF_SIZEOF, "ChainBuffer sizeof() doesn't match CHAINBUF_SIZEOF macro");


struct ChainBuffer* chainbuf_init(void) {
	struct ChainBuffer *buf = malloc(CHAINBUF_SIZEOF);
	if(buf != NULL) {
		memset(buf->data, 0, CHAINBUF_CAPACITY);
		buf->used = 0;
		buf->previous = NULL;
	}
	
	return buf;
}

void chainbuf_free(struct ChainBuffer *buf) {
	while(buf != NULL) {
		struct ChainBuffer *current = buf;
		buf = buf->previous;
		free(current);
	}
}

char* chainbuf_append(struct ChainBuffer **buf, const char *data) {
	const size_t datalen = strlen(data) + 1;
	if(datalen > CHAINBUF_CAPACITY) return NULL;
	
	if((*buf)->used + datalen > CHAINBUF_CAPACITY) {
		struct ChainBuffer *newbuf = chainbuf_init();
		if(newbuf == NULL) return NULL;
		
		newbuf->previous = *buf;
		*buf = newbuf;
	}
	
	char *insert_pos = (*buf)->data + (*buf)->used;
	memcpy(insert_pos, data, datalen);
	(*buf)->used += datalen;
	
	return insert_pos;
}

struct ReBuffer* rebuf_init(void) {
	void* mem = malloc(REBUF_STEP);
	if(mem == NULL) return NULL;
	
	struct ReBuffer *buf = malloc(sizeof(struct ReBuffer));
	if(buf == NULL) {
		free(mem);
		return NULL;
	}
	
	buf->data = mem;
	buf->capacity = REBUF_STEP;
	buf->used = 0;
	
	return buf;
}

void rebuf_free(struct ReBuffer *buf) {
	if(buf == NULL) return;
	
	free(buf->data);
	free(buf);
}

void* rebuf_append(struct ReBuffer *const buf, const void *const data, const size_t datalen) {
	if(buf->used + datalen > buf->capacity) {
		const size_t steps = (datalen / REBUF_STEP) + !!(datalen % REBUF_STEP);
		const size_t memsize = buf->capacity + (steps * REBUF_STEP);
		
		void* newmem = realloc(buf->data, memsize);
		if(newmem == NULL) return NULL;
		
		buf->data = newmem;
		buf->capacity = memsize;
	}
	
	char *insert_pos = (char*)buf->data + buf->used;
	memcpy(insert_pos, data, datalen);
	buf->used += datalen;
	
	return (void*)insert_pos;
}
