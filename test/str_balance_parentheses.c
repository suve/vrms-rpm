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
	char buffer[strlen(input) + 16]; \
	size_t outputLen; \
	assert_true(str_balance_parentheses(input, buffer, sizeof(buffer), &outputLen)); \
	assert_string_equal(buffer, expected); \
	assert_int_equal(strlen(buffer), outputLen); \
}while(0)

void test__str_balance_parentheses(void **state) {
	UNUSED(state);

	// More '(' than ')'
	testcase("(One (Two", "(One (Two))");
	testcase("One (Two (Three", "One (Two (Three))");

	// More ')' than '('
	testcase("One (Two))", "(One (Two))");
	testcase("(One) and (Two))", "((One) and (Two))");

	// First ')' appears before first '('
	testcase("One) or (Two", "(One) or (Two)");
	testcase("One and Two) or Three) and Four", "((One and Two) or Three) and Four");
}
