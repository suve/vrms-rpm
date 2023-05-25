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

#define testcase(haystack, needle, expected) do{ \
	const char *result = str_ends_with(haystack, needle); \
	if(expected) { \
		assert_non_null(result); \
		assert_string_equal(result, needle); \
	} else { \
		assert_null(result); \
	} \
}while(0)

void test__str_ends_with(void **state) {
	UNUSED(state);

	testcase("partial match", "match", 1);
	testcase("full string match", "full string match", 1);
	testcase("an empty needle", "", 1);

	testcase("no match", "blah", 0);
	testcase("very", "very long needle", 0);
	testcase("longer", "match at end but needle is longer", 0);
	testcase("", "an empty haystack", 0);
}
