/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2021, 2023 suve (a.k.a. Artur Frenszek-Iwicki)
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

// The arg/def/jmp includes are required by cmocka.
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>
#include <time.h>

#include "src/buffers.h"

#define UNUSED(x) ((void)(x))

int test_setup__rebuffer(void **state) {
	srand(time(NULL));

	struct ReBuffer *rb = rebuf_init();
	assert_non_null(rb);
	*state = rb;

	return 0;
}

int test_teardown__rebuffer(void **state) {
	struct ReBuffer *rb = *state;
	rebuf_free(rb);

	return 0;
}

static int random(const int min, const int max) {
	return min + (rand() % (max - min + 1));
}

static void randomize_string(char *buffer, const size_t bufsize) {
	for(size_t i = 0; i < bufsize - 1; ++i) {
		buffer[i] = random(0x21, 0x7e);
	}
	buffer[bufsize - 1] = '\0';
}

void test__rebuffer(void **state) {
	struct ReBuffer *rb = *state;

	const char first_buffer[] = "some data at the start";
	const size_t first_offset = rb->used;
	char *first_append = rebuf_append(rb, first_buffer, sizeof(first_buffer));
	assert_string_equal(first_append, first_buffer);

	// Perform many randomized appends
	for(int i = 0; i < 1000; ++i) {
		char buffer[64];
		const int size = random(16, 64);
		randomize_string(buffer, size);

		char *append = rebuf_append(rb, buffer, size);
		assert_string_equal(append, buffer);
	}
	
	const char second_buffer[] = "some data in the middle";
	const size_t second_offset = rb->used;
	char *second_append = rebuf_append(rb, second_buffer, sizeof(second_buffer));
	assert_string_equal(second_append, second_buffer);

	// Perform a few very large (anywhere from 0.5x to 8x REBUF_STEP) appends.
	for(int i = 0; i < 10; ++i) {
		const size_t size = random(5, 80) * REBUF_STEP / 10;
		char *buffer = test_malloc(size);
		randomize_string(buffer, size);

		char *append = rebuf_append(rb, buffer, size);
		assert_memory_equal(append, buffer, size);

		test_free(buffer);
	}

	// Check if nothing bad happened to our earlier appends.
	// NOTE: Due to how ReBuffers work, the "nth_append" pointers are no longer valid.
	//       We need to determine the current location of the copies using "nth_offset".
	assert_string_equal((char*)rb->data + first_offset, first_buffer);
	assert_string_equal((char*)rb->data + second_offset, second_buffer);
}
