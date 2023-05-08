/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2023 suve (a.k.a. Artur Frenszek-Iwicki)
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

#include "../src/stringutils.h"

#define UNUSED(x) ((void)(x))

#define testcase(input, expected) do { \
	char *result = find_closing_paren(input); \
	if((expected) > 0) { \
		int diff = result - input; \
		assert_int_equal(diff, (expected)); \
	} else { \
		assert_null(result); \
	} \
}while(0)

void test__find_closing_paren(void **state) {
	UNUSED(state);

	// Simple nested parens
	testcase("(single)", 7);
	testcase("((double))", 9);
	testcase("(((triple)))", 11);
	testcase("((( ))))) spurious closing parens", 6);

	// More complicated nesting
	testcase("( (one) (two) )", 14);
	testcase("( ((one)) ((two)) )", 18);

	// Mismatched parentheses
	testcase("( no closing paren", 0);
	testcase("((( still no closing paren", 0);
	testcase("((( not enough closing parens ))", 0);

	// Invalid input
	testcase("string does not start with an opening paren", 0);
}
