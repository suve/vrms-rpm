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

#include "src/stringutils.h"

#define UNUSED(x) ((void)(x))

#define testcase(str, max_count, sep, expected_count, ...) do{ \
	char buffer[] = str; \
\
	char *result_parts[max_count]; \
	const int result_count = str_split(buffer, sep, result_parts, max_count); \
\
	assert_int_equal(expected_count, result_count); \
\
	const char *expected_parts[] = { __VA_ARGS__ }; \
	for(int i = 0; i < expected_count; ++i) { \
		assert_string_equal(result_parts[i], expected_parts[i]); \
	} \
}while(0)

void test__str_split(void **state) {
	UNUSED(state);

	testcase("this-is-a-test", 4, '-', 4, "this", "is", "a", "test");

	testcase("less parts than max", 8, ' ', 4, "less", "parts", "than", "max");
	testcase("more parts than max", 3, ' ', 3, "more", "parts", "than max");

	testcase("separator not found", 4, ':', 1, "separator not found");
	testcase("", 4, ':', 1, ""); // test splitting empty string

	testcase("-separator-at-start", 8, '-', 4, "", "separator", "at", "start");
	testcase("separator-at-end-", 8, '-', 4, "separator", "at", "end", "");

	testcase("multiple::separators", 8, ':', 3, "multiple", "", "separators");
}
