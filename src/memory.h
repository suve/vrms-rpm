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
#ifndef VRMS_RPM_MEMORY_H
#define VRMS_RPM_MEMORY_H

#include <stddef.h>

/* 
 * A wrapper around malloc() that panics on allocation failure.
 * From the caller's perspective, this always returns a non-NULL pointer.
 */
extern void* mem_alloc(const size_t size);

/* 
 * A wrapper around realloc() that panics on allocation failure.
 * From the caller's perspective, this always returns a non-NULL pointer.
 */
extern void* mem_realloc(void *ptr, const size_t newsize);

/*
 * A wrapper around free(). Just 'cause.
 */
extern void mem_free(void *ptr);

#endif
