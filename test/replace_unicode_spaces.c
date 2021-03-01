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

#include "../src/stringutils.h"

#define UNUSED(x) ((void)(x))

#define testcase(input, expected) do { \
	char buffer[] = input; \
	size_t result_len = replace_unicode_spaces(buffer); \
\
	assert_string_equal(buffer, expected); \
	assert_int_equal((int)result_len, (int)strlen(buffer)); \
}while(0)

#define NBSP "\u00A0"

void test__replace_unicode_spaces(void **state) {
	UNUSED(state);

	testcase(NBSP "single at start", " single at start");
	testcase(NBSP NBSP NBSP "triple at start", "   triple at start");

	testcase("single at end" NBSP, "single at end ");
	testcase("triple at end" NBSP NBSP NBSP, "triple at end   ");

	testcase("single in the" NBSP "middle", "single in the middle");
	testcase("triple in the" NBSP NBSP NBSP "middle", "triple in the   middle");

	testcase("intertwined" NBSP " " NBSP " " NBSP "with spaces", "intertwined     with spaces");
	testcase(NBSP "literally" NBSP "everywhere" NBSP, " literally everywhere ");

	testcase(NBSP, " ");
	testcase(NBSP NBSP, "  ");
	testcase(NBSP NBSP NBSP NBSP, "    ");
}
