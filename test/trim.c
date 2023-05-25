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

#define testcase(input, expected, extrachars) do { \
	char buffer[] = input; \
\
	size_t result_len; \
	char *result = trim_extra(buffer, &result_len, extrachars); \
\
	assert_string_equal(result, expected); \
	assert_int_equal((int)result_len, (int)strlen(result)); \
}while(0)

void test__trim(void **state) {
	UNUSED(state);

	testcase("newline\n", "newline", "");
	testcase("    indent", "indent", "");
	testcase("\tfield-one\tfield-two\t", "field-one\tfield-two", "");
	testcase("(some extra chars)", "some extra chars", "()");
	testcase("remove everything", "", "abcdefghijklmnopqrstuvwxyz");
}
