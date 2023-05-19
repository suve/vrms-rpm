/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2021-2023 suve (a.k.a. Artur Frenszek-Iwicki)
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
#include "licences.h"

// Test license strings that evaluate to a single licence.
void test__spdxStrict_single(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->spdxStrictClassifier;

	// Test some simple good licences.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Awesome");
		test_licence("Awesome", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good");
		test_licence("Good", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Long name with spaces");
		test_licence("Long name with spaces", expected);
	}

	// Test some simple bad licences.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Awful");
		test_licence("Awful", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad");
		test_licence("Bad", expected);
	}
}

// Test licence strings using "WITH" and "+" operators.
void test__spdxStrict_suffixes(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->spdxStrictClassifier;

	// A good licence with the "+" operator should be recognized as a good licence.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good+");
		test_licence("Good+", expected);
	}
	// In strict mode, there can be no whitespace between the licence name and the "+".
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Good +");
		test_licence("Good +", expected);
	}
	// A bad licence with the "+" operator is still bad.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad+");
		test_licence("Bad+", expected);
	}

	// A good licence with the "WITH" operator should be recognized as a good licence.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good WITH Extra-Permissions");
		test_licence("Good WITH Extra-Permissions", expected);
	}
	// The "WITH" operator can be preceded by even more whitespace.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good   WITH Extra-Permissions");
		test_licence("Good   WITH Extra-Permissions", expected);
	}
	// In strict mode, "WITH" must be matched case-sensitively.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Good With Extra-Permissions");
		test_licence("Good With Extra-Permissions", expected);
	}
	// A bad licence with the "WITH" operator is still bad.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad WITH Even-More-Badness");
		test_licence("Bad WITH Even-More-Badness", expected);
	}
}
