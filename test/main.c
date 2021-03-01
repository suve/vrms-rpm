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

extern void test__trim(void **state);
extern void test__str_starts_with(void **state);
extern void test__str_ends_with(void **state);
extern void test__replace_unicode_spaces(void **state);

int main(void) {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test__trim),
		cmocka_unit_test(test__str_starts_with),
		cmocka_unit_test(test__str_ends_with),
		cmocka_unit_test(test__replace_unicode_spaces),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
