/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2021, 2023 suve (a.k.a. Artur Frenszek-Iwicki)
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

#include "src/buffers.h"

struct ChainBuffer* chainbuf_init(size_t capacity) {
	if(capacity == 0) return NULL;

	struct ChainBuffer *buf = malloc(sizeof(struct ChainBuffer) + capacity);
	if(buf != NULL) {
		memset(buf->data, 0, capacity);
		buf->capacity = capacity;
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
	const size_t dataLength = strlen(data) + 1;
	const size_t remaining = (*buf)->capacity - (*buf)->used;

	if(dataLength > remaining) {
		// TODO: Allow to allocate larger chunks if needed
		if(dataLength > (*buf)->capacity) return NULL;

		struct ChainBuffer *newbuf = chainbuf_init((*buf)->capacity);
		if(newbuf == NULL) return NULL;

		newbuf->previous = *buf;
		*buf = newbuf;
	}
	
	char *insert_pos = (*buf)->data + (*buf)->used;
	memcpy(insert_pos, data, dataLength);
	(*buf)->used += dataLength;
	
	return insert_pos;
}

struct ReBuffer* rebuf_init(const size_t stepSize) {
	if(stepSize == 0) return NULL;

	void* mem = malloc(stepSize);
	if(mem == NULL) return NULL;
	
	struct ReBuffer *buf = malloc(sizeof(struct ReBuffer));
	if(buf == NULL) {
		free(mem);
		return NULL;
	}
	
	buf->data = mem;
	buf->step = stepSize;
	buf->capacity = stepSize;
	buf->used = 0;
	
	return buf;
}

void rebuf_free(struct ReBuffer *buf) {
	if(buf != NULL) {
		if(buf->data != NULL) free(buf->data);
		free(buf);
	}
}

void* rebuf_append(struct ReBuffer *const buf, const void *const data, const size_t dataLength) {
	if(buf->used + dataLength > buf->capacity) {
		const size_t steps = (dataLength / buf->step) + !!(dataLength % buf->step);
		const size_t memsize = buf->capacity + (steps * buf->step);
		
		void* newmem = realloc(buf->data, memsize);
		if(newmem == NULL) return NULL;
		
		buf->data = newmem;
		buf->capacity = memsize;
	}
	
	char *insert_pos = (char*)buf->data + buf->used;
	memcpy(insert_pos, data, dataLength);
	buf->used += dataLength;
	
	return (void*)insert_pos;
}
