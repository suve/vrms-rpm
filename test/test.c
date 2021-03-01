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

#define test__trim__case(input, expected, extrachars) do { \
	char buffer[] = input; \
\
	size_t result_len; \
	char *result = trim_extra(buffer, &result_len, extrachars); \
\
	assert_string_equal(result, expected); \
	assert_int_equal((int)result_len, (int)strlen(result)); \
}while(0)

static void test__trim(void **state) {
	UNUSED(state);

	test__trim__case("newline\n", "newline", "");
	test__trim__case("    indent", "indent", "");
	test__trim__case("\tfield-one\tfield-two\t", "field-one\tfield-two", "");
	test__trim__case("(some extra chars)", "some extra chars", "()");
	test__trim__case("remove everything", "", "abcdefghijklmnopqrstuvwxyz");
}

#define test__str_starts_with__case(haystack, needle, expected) do{ \
	const char *result = str_starts_with(haystack, needle); \
	if(expected) { \
		assert_non_null(result); \
		assert_ptr_equal(haystack, result); \
	} else { \
		assert_null(result); \
	} \
}while(0)

static void test__str_starts_with(void **state) {
	UNUSED(state);

	test__str_starts_with__case("partial match", "partial", 1);
	test__str_starts_with__case("full string match", "full string match", 1);
	test__str_starts_with__case("an empty needle", "", 1);

	test__str_starts_with__case("no match", "blah", 0);
	test__str_starts_with__case("very", "very long needle", 0);
	test__str_starts_with__case("", "an empty haystack", 0);
}

#define test__str_ends_with__case(haystack, needle, expected) do{ \
	const char *result = str_ends_with(haystack, needle); \
	if(expected) { \
		assert_non_null(result); \
		assert_string_equal(result, needle); \
	} else { \
		assert_null(result); \
	} \
}while(0)

static void test__str_ends_with(void **state) {
	UNUSED(state);

	test__str_ends_with__case("partial match", "match", 1);
	test__str_ends_with__case("full string match", "full string match", 1);
	test__str_ends_with__case("an empty needle", "", 1);

	test__str_ends_with__case("no match", "blah", 0);
	test__str_ends_with__case("very", "very long needle", 0);
	test__str_ends_with__case("longer", "match at end but needle is longer", 0);
	test__str_ends_with__case("", "an empty haystack", 0);
}

#define test__replace_unicode_spaces__case(input, expected) do { \
	char buffer[] = input; \
	size_t result_len = replace_unicode_spaces(buffer); \
\
	assert_string_equal(buffer, expected); \
	assert_int_equal((int)result_len, (int)strlen(buffer)); \
}while(0)

#define NBSP "\u00A0"

static void test__replace_unicode_spaces(void **state) {
	UNUSED(state);

	test__replace_unicode_spaces__case(NBSP "single at start", " single at start");
	test__replace_unicode_spaces__case(NBSP NBSP NBSP "triple at start", "   triple at start");

	test__replace_unicode_spaces__case("single at end" NBSP, "single at end ");
	test__replace_unicode_spaces__case("triple at end" NBSP NBSP NBSP, "triple at end   ");

	test__replace_unicode_spaces__case("single in the" NBSP "middle", "single in the middle");
	test__replace_unicode_spaces__case("triple in the" NBSP NBSP NBSP "middle", "triple in the   middle");

	test__replace_unicode_spaces__case("intertwined" NBSP " " NBSP " " NBSP "with spaces", "intertwined     with spaces");
	test__replace_unicode_spaces__case(NBSP "literally" NBSP "everywhere" NBSP, " literally everywhere ");

	test__replace_unicode_spaces__case(NBSP, " ");
	test__replace_unicode_spaces__case(NBSP NBSP, "  ");
	test__replace_unicode_spaces__case(NBSP NBSP NBSP NBSP, "    ");
}

int main(void) {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test__trim),
		cmocka_unit_test(test__str_starts_with),
		cmocka_unit_test(test__str_ends_with),
		cmocka_unit_test(test__replace_unicode_spaces),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
