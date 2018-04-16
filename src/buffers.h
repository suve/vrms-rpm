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

#define BUFFER_SIZEOF    4096
#define BUFFER_CAPACITY  (BUFFER_SIZEOF - sizeof(int) - sizeof(void*))

struct Buffer {
	char data[BUFFER_CAPACITY];
	int used;
	struct Buffer *previous;
};

struct Buffer* buffer_init(void);
void buffer_free(struct Buffer *buf);

char* buffer_insert(struct Buffer **buf, char *data);

#endif
