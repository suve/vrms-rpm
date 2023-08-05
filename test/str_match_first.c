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

#include <string.h>

#include "src/stringutils.h"

#define UNUSED(x) ((void)(x))

#define testcase(haystack, expected_index, ...) do{ \
	int (*const func_array[])(const char*) = { __VA_ARGS__, NULL }; \
\
	char *result_str; \
	int result_index = str_match_first(haystack, func_array, &result_str); \
\
	assert_int_equal(result_index, expected_index); \
	if(expected_index >= 0) { \
		assert_true(func_array[expected_index](result_str)); \
	} \
}while(0)

static int is_Hello(const char *ptr) {
	return strncmp(ptr, "Hello", 5) == 0;
}

static int is_World(const char *ptr) {
	return strncmp(ptr, "World", 5) == 0;
}

static int is_abc(const char *ptr) {
	if((ptr[0] != 'a') && (ptr[0] != 'A')) return 0;
	if((ptr[1] != 'b') && (ptr[1] != 'B')) return 0;
	if((ptr[2] != 'c') && (ptr[2] != 'C')) return 0;
	return 1;
}

void test__str_match_first(void **state) {
	UNUSED(state);

	testcase("Hello World!", 0, is_Hello, is_World, is_abc);
	testcase("hello World!", 1, is_Hello, is_World, is_abc);
	testcase("hello world!", -1, is_Hello, is_World, is_abc);

	testcase("abcdef", 2, is_Hello, is_World, is_abc);
	testcase("hello ABCdef", 2, is_Hello, is_World, is_abc);
	testcase("Welcome to AbCdef World!", 2, is_Hello, is_World, is_abc);
}
