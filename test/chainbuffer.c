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

int test_setup__chainbuffer(void **state) {
	srand(time(NULL));

	struct ChainBuffer *cb = chainbuf_init();
	assert_non_null(cb);
	*state = cb;

	return 0;
}

int test_teardown__chainbuffer(void **state) {
	struct ChainBuffer *cb = *state;
	chainbuf_free(cb);

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

void test__chainbuffer(void **state) {
	struct ChainBuffer **cb = (void*)state;

	const char *first_data = "some text appended at start";
	char *first_append = chainbuf_append(cb, first_data);
	assert_string_equal(first_append, first_data);

	// Lots of small appends.
	for(int i = 0; i < 5000; ++i) {
		const char *short_text = "abc";
		char *append = chainbuf_append(cb, short_text);
		assert_string_equal(append, short_text);
	}

	const char *second_data = "some other text appended in the middle";
	char *second_append = chainbuf_append(cb, second_data);
	assert_string_equal(second_append, second_data);

	// A few max size appends.
	for(int i = 0; i < 4; ++i) {
		char buffer[CHAINBUF_CAPACITY];
		randomize_string(buffer, sizeof(buffer));

		char *append = chainbuf_append(cb, buffer);
		assert_memory_equal(append, buffer, sizeof(buffer));
	}

	const char *third_data = "yet another text somewhere in the middle";
	char *third_append = chainbuf_append(cb, third_data);
	assert_string_equal(third_append, third_data);

	// Couple of longer appends (anywhere from 128 to 1024 chars).
	for(int i = 0; i < 20; ++i) {
		char buffer[1024];
		const size_t size = random(128, sizeof(buffer));
		randomize_string(buffer, size);

		char *append = chainbuf_append(cb, buffer);
		assert_memory_equal(append, buffer, size);
	}

	// Check if nothing bad happened to our earlier appends.
	assert_string_equal(first_append, first_data);
	assert_string_equal(second_append, second_data);
	assert_string_equal(third_append, third_data);
}
