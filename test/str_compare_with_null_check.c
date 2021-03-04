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

#define testcase(first, second, func, expected) do{ \
	int result = str_compare_with_null_check((first), (second), (func)); \
	assert_int_equal(result, (expected)); \
}while(0)

void test__str_compare_with_null_check(void **state) {
	UNUSED(state);

	// No NULLs
	testcase("aaaa", "bbbb", &strcmp, -1);
	testcase("XYZ", "XYZ", &strcmp, 0);
	testcase("ez", "ee", &strcmp, +1);

	// With one NULL
	testcase(NULL, "qwerty", &strcmp, -1);
	testcase(NULL, "", &strcmp, -1);
	testcase("asdf", NULL, &strcmp, +1);
	testcase("", NULL, &strcmp, +1);

	// Two NULLs
	testcase(NULL, NULL, &strcmp, 0);
}
