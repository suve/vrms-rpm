/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2021-2023, 2025 suve (a.k.a. Artur Frenszek-Iwicki)
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
#include "test/licences.h"
#include "test/packages.h"
#include "test/printers.h"
#include "test/test.h"

extern void test__chainbuffer(void **state);
extern int test_setup__chainbuffer(void **state);
extern int test_teardown__chainbuffer(void **state);

extern void test__rebuffer(void **state);
extern int test_setup__rebuffer(void **state);
extern int test_teardown__rebuffer(void **state);

extern void test__compare_versions(void **state);
extern void test__find_closing_paren(void **state);
extern void test__replace_unicode_spaces(void **state);
extern void test__str_balance_parentheses(void **state);
extern void test__str_compare_with_null_check(void **state);
extern void test__str_match_first(void **state);
extern void test__str_starts_with(void **state);
extern void test__str_ends_with(void **state);
extern void test__str_split(void **state);
extern void test__str_squeeze_char(void **state);
extern void test__trim(void **state);

extern void test__jsonTypes(void **state);
extern void test__jsonWeirdStrings(void **state);
extern void test__jsonNestedObjects(void **state);
extern void test__jsonArrays(void **state);
extern int test_setup__json(void **state);
extern int test_teardown__json(void **state);

int main(void) {
	int failures = 0;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test__chainbuffer, test_setup__chainbuffer, test_teardown__chainbuffer),
		cmocka_unit_test_setup_teardown(test__rebuffer, test_setup__rebuffer, test_teardown__rebuffer),
		cmocka_unit_test(test__compare_versions),
		cmocka_unit_test(test__find_closing_paren),
		cmocka_unit_test(test__replace_unicode_spaces),
		cmocka_unit_test(test__str_balance_parentheses),
		cmocka_unit_test(test__str_compare_with_null_check),
		cmocka_unit_test(test__str_match_first),
		cmocka_unit_test(test__str_starts_with),
		cmocka_unit_test(test__str_ends_with),
		cmocka_unit_test(test__str_split),
		cmocka_unit_test(test__str_squeeze_char),
		cmocka_unit_test(test__trim),
	};
	failures += cmocka_run_group_tests(tests, NULL, NULL);

	const struct CMUnitTest licence_tests[] = {
		cmocka_unit_test(test__looseClassifier_single),
		cmocka_unit_test(test__looseClassifier_one_level),
		cmocka_unit_test(test__looseClassifier_tree),
		cmocka_unit_test(test__looseClassifier_extra_parentheses),
		cmocka_unit_test(test__looseClassifier_case_insensitive_joiners),
		cmocka_unit_test(test__looseClassifier_acceptable_suffixes),
		cmocka_unit_test(test__looseClassifier_mismatched_parentheses),
		cmocka_unit_test(test__spdxStrict_single),
		cmocka_unit_test(test__spdxStrict_suffixes),
		cmocka_unit_test(test__spdxStrict_one_level),
		cmocka_unit_test(test__spdxStrict_tree),
		cmocka_unit_test(test__spdxStrict_precedence),
		cmocka_unit_test(test__spdxStrict_whitespace),
		cmocka_unit_test(test__spdxStrict_caseSensitivity),
		cmocka_unit_test(test__spdxStrict_mangledStrings),
		cmocka_unit_test(test__spdxLenient),
	};
	failures += cmocka_run_group_tests(licence_tests, test_setup__licences, test_teardown__licences);

	const struct CMUnitTest json_tests[] = {
		cmocka_unit_test(test__jsonTypes),
		cmocka_unit_test(test__jsonWeirdStrings),
		cmocka_unit_test(test__jsonNestedObjects),
		cmocka_unit_test(test__jsonArrays),
	};
	failures += cmocka_run_group_tests(json_tests, test_setup__json, test_teardown__json);

	const struct CMUnitTest package_tests[] = {
		cmocka_unit_test(test__packageIter),
	};
	failures += cmocka_run_group_tests(package_tests, test_setup__packages, test_teardown__packages);

	const struct CMUnitTest printer_tests[] = {
		cmocka_unit_test(test__textPrinter),
		cmocka_unit_test(test__jsonPrinter),
	};
	failures += cmocka_run_group_tests(printer_tests, test_setup__printers, test_teardown__printers);

	return !!failures;
}
