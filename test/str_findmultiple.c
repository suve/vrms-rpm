/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2021 Artur "suve" Iwicki
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

#include "../src/stringutils.h"

#define UNUSED(x) ((void)(x))

#define testcase(haystack, expected_index, ...) do{ \
	const char *const needle_array[] = { __VA_ARGS__ }; \
	size_t needle_count = sizeof(needle_array) / sizeof(needle_array[0]); \
\
	char *result_str; \
	const char *result_needle; \
	str_findmultiple(haystack, needle_count, needle_array, &result_str, &result_needle); \
\
	if((expected_index) >= 0) { \
		assert_ptr_equal(result_needle, needle_array[(expected_index)]); \
		assert_memory_equal(result_str, result_needle, strlen(result_needle)); \
	} else { \
		assert_null(result_str); \
		assert_null(result_needle); \
	} \
}while(0)

void test__str_findmultiple(void **state) {
	UNUSED(state);

	testcase("Hello world!", 0, "Hello", "world!");
	testcase("Hello world!", 2, "Poland!", "Europe!", "world!");

	testcase("No matches here!", -1, "Where", "are", "those", "needles");
	testcase("Look mom, zero needles!", -1, );

	testcase("", -1, "Empty", "haystack");
	testcase("An empty needle", 1, "empty", "", "needle");
}
