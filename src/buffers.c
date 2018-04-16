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
#include <stdlib.h>
#include <string.h>

#include "buffers.h"

_Static_assert(sizeof(struct Buffer) == BUFFER_SIZEOF, "Buffer sizeof() doesn't match BUFFER_SIZEOF macro");


struct Buffer* buffer_init(void) {
	struct Buffer *buf = malloc(BUFFER_SIZEOF);
	if(buf != NULL) {
		memset(buf->data, 0, BUFFER_CAPACITY);
		buf->used = 0;
		buf->previous = NULL;
	}
	
	return buf;
}

void buffer_free(struct Buffer *buf) {
	while(buf != NULL) {
		struct Buffer *current = buf;
		buf = buf->previous;
		free(current);
	}
}

char* buffer_insert(struct Buffer **buf, char *data) {
	const size_t datalen = strlen(data) + 1;
	if((*buf)->used + datalen > BUFFER_CAPACITY) {
		struct Buffer *newbuf = buffer_init();
		if(newbuf == NULL) return NULL;
		
		newbuf->previous = *buf;
		*buf = newbuf;
	}
	
	char *insert_pos = (*buf)->data + (*buf)->used;
	memcpy(insert_pos, data, datalen);
	(*buf)->used += datalen;
	
	return insert_pos;
}
