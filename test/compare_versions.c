/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2021, 2023 suve (a.k.a. Artur Frenszek-Iwicki)
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

#include "src/versions.h"

#define UNUSED(x) ((void)(x))

#define testcase(a, b, expected) do { \
	const int result = compare_versions(a, b); \
	assert_int_equal(result, expected);  \
\
	if(expected) { \
		const int result_swapped = compare_versions(b, a); \
		assert_int_equal(result_swapped, 0 - expected); \
	} \
}while(0)

void test__compare_versions(void **state) {
	UNUSED(state);

#ifdef WITH_LIBRPM
	// We want to test our fallback mechanism, not librpm's behaviour.
	skip();
#else
	testcase("1.0", "0.9", +1);
	testcase("1.1", "1.0", +1);
	testcase("1.1.1", "1.1", +1);

	testcase("1.b", "1.a", +1);
	testcase("1.c", "1.b", +1);

	testcase("0.8.8", "0.9.1", -1);
	testcase("0.8.8", "1.0.5", -1);

	testcase("1", "1", 0);
	testcase("1.2", "1.2", 0);
	testcase("1.2.3", "1.2.3", 0);
	testcase("1.2.3.4", "1.2.3.4", 0);
	testcase("1.2.3.4.5", "1.2.3.4.5", 0);

	testcase("1.a", "1.a", 0);
	testcase("1.a.2", "1.a.2", 0);
	testcase("1.a.2.b", "1.a.2.b", 0);
	testcase("1.a.2.b.3", "1.a.2.b.3", 0);
#endif
}
