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

static void test_trim_case(const char *input, const char *expected, const char *extrachars) {
	size_t input_len = strlen(input);
	char buffer[input_len+1];
	memcpy(buffer, input, input_len+1);

	size_t result_len;
	char *result = trim_extra(buffer, &result_len, extrachars);

	assert_string_equal(result, expected);
	assert_int_equal((int)result_len, (int)strlen(result));
}

static void test_trim(void **state) {
	UNUSED(state);

	test_trim_case("newline\n", "newline", "");
	test_trim_case("    indent", "indent", "");
	test_trim_case("\tfield-one\tfield-two\t", "field-one\tfield-two", "");
	test_trim_case("(some extra chars)", "some extra chars", "()");
	test_trim_case("remove everything", "", "abcdefghijklmnopqrstuvwxyz");
}

int main(void) {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_trim),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
