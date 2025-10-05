/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2025 suve (a.k.a. Artur Frenszek-Iwicki)
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
#include <stdio.h>
#include <stdlib.h>

#include "src/lang.h"
#include "src/memory.h"

static inline void panic(const size_t size) {
	lang_fprint(stderr, MSG_ERR_MALLOC, size);
	exit(EXIT_FAILURE);
}

void* mem_alloc(const size_t size) {
	void *result = malloc(size);
	if(result == NULL) panic(size);
	return result;
}

void* mem_realloc(void *ptr, const size_t newsize) {
	void *result = realloc(ptr, newsize);
	if(result == NULL) panic(newsize);
	return result;
}

void mem_free(void *ptr) {
	free(ptr);
}
