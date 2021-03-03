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

#include "../src/licences.h"
#include "../src/options.h"

#define UNUSED(x) ((void)(x))

int test_setup__licences(void **state) {
	// Create the path to the test licence list.
	// Assumes that the test-suite executable is inside build/.
	char buffer[1024];
	strcpy(buffer, *state);
	*strrchr(buffer, '/') = '\0'; // remove the executable name
	*strrchr(buffer, '/') = '\0'; // remove the build/ directory
	const size_t buflen = strlen(buffer);
	strcpy(buffer + buflen, "/test/licences.txt");

	opt_licencelist = buffer;
	assert_int_not_equal(licences_read(), -1);

	return 0;
}

int test_teardown__licences(void **state) {
	UNUSED(state);

	licences_free();
	opt_licencelist = NULL;
	return 0;
}

// The licence text must be writable, hence we use the buffer[] trick.
#define testcase(text, expected_type, expected_free) do{ \
	char buffer[] = (text); \
	struct LicenceTreeNode *ltn = licence_classify(buffer); \
	assert_non_null(ltn); \
	assert_int_equal(ltn->type, (expected_type)); \
	if(expected_free) \
		assert_true(ltn->is_free); \
	else \
		assert_false(ltn->is_free); \
	licence_freeTree(ltn); \
} while(0)

void test__licences(void **state) {
	UNUSED(state);

	testcase("Good", LTNT_LICENCE, 1);
	testcase("Awesome", LTNT_LICENCE, 1);
	testcase("Long name with spaces", LTNT_LICENCE, 1);

	testcase("Bad", LTNT_LICENCE, 0);
	testcase("Awful", LTNT_LICENCE, 0);

	testcase("Good or Awesome", LTNT_OR, 1);
	testcase("Good or Bad", LTNT_OR, 1);
	testcase("Bad or Good", LTNT_OR, 1);
	testcase("Bad or Awful", LTNT_OR, 0);

	testcase("Good and Awesome", LTNT_AND, 1);
	testcase("Good and Bad", LTNT_AND, 0);
	testcase("Bad and Good", LTNT_AND, 0);
	testcase("Bad and Awful", LTNT_AND, 0);

	testcase("(Good or Bad) and (Good or Awesome)", LTNT_AND, 1);
	testcase("(Good or Bad) and (Good and Awesome)", LTNT_AND, 1);
	testcase("(Good or Bad) and (Bad or Awful)", LTNT_AND, 0);
	testcase("(Good or Bad) and (Bad and Awful)", LTNT_AND, 0);

	testcase("(Good and Awesome) or (Good or Long name with spaces)", LTNT_OR, 1);
	testcase("(Good and Bad) or (Good and Awesome)", LTNT_OR, 1);
	testcase("(Good and Bad) or (Good or Awful)", LTNT_OR, 1);
	testcase("(Good and Bad) or (Good and Awful)", LTNT_OR, 0);

	testcase("(Unnecessary parentheses)", LTNT_LICENCE, 0);
	testcase("(Good)", LTNT_LICENCE, 1);
	testcase("((Awesome))", LTNT_LICENCE, 1);

	// Acceptable suffixes
	testcase("Good with acknowledgement", LTNT_LICENCE, 1);
	testcase("Awesome with additional permissions", LTNT_LICENCE, 1);
	testcase("Long name with spaces with linking exception", LTNT_LICENCE, 1);
	testcase("Bad with acknowledgement", LTNT_LICENCE, 0);
	testcase("Bad with additional permissions", LTNT_LICENCE, 0);
	testcase("Bad with linking exception", LTNT_LICENCE, 0);
}
