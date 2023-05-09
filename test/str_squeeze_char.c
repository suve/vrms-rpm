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

#define testcase(input, squeezable, expected) do { \
	char buffer[] = input; \
	str_squeeze_char(buffer, squeezable); \
	assert_string_equal(buffer, expected); \
}while(0)

void test__str_squeeze_char(void **state) {
	UNUSED(state);

	// At the start
	testcase("00001", '0', "01");
	testcase("aaaaabsolutely!", 'a', "absolutely!");

	// In the middle
	testcase("Hello", 'l', "Helo");
	testcase("Boogeyman", 'o', "Bogeyman");

	// At the end
	testcase("Mamma mia!!!", '!', "Mamma mia!");
	testcase("123.3333333", '3', "123.3");

	// Multiple locations
	testcase("--- Header ---", '-', "- Header -");
}
