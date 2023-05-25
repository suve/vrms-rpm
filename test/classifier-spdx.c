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
#include "test/licences.h"

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

// Test licence strings that evaluate to a single-level and/or chain.
void test__spdxStrict_one_level(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->spdxStrictClassifier;

	// Test some simple "A OR B" licences.
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(expected, 1, LTNT_OR, first, second);

		test_licence("Good OR Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Awful");
		make_ltn(expected, 1, LTNT_OR, first, second);

		test_licence("Good OR Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(expected, 1, LTNT_OR, first, second);

		test_licence("Bad OR Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 0, "Awful");
		make_ltn(expected, 0, LTNT_OR, first, second);

		test_licence("Bad OR Awful", expected);
	}

	// Test some simple "A AND B" licences.
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(expected, 1, LTNT_AND, first, second);

		test_licence("Good AND Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Awful");
		make_ltn(expected, 0, LTNT_AND, first, second);

		test_licence("Good AND Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(expected, 0, LTNT_AND, first, second);

		test_licence("Bad AND Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 0, "Awful");
		make_ltn(expected, 0, LTNT_AND, first, second);

		test_licence("Bad AND Awful", expected);
	}

	// Test chains with more than 2 sub-licences.
	{
		struct LicenceTreeNode *first, *second, *third, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn_simple(third, 0, "Bad");
		make_ltn(expected, 0, LTNT_AND, first, second, third);

		test_licence("Good AND Awesome AND Bad", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *third, *fourth, *expected;
		make_ltn_simple(first, 0, "Awful");
		make_ltn_simple(second, 0, "Bad");
		make_ltn_simple(third, 0, "Unknown");
		make_ltn_simple(fourth, 1, "Good");
		make_ltn(expected, 1, LTNT_OR, first, second, third, fourth);

		test_licence("Awful OR Bad OR Unknown OR Good", expected);
	}
}

// Test licence strings that evaluate to a tree.
void test__spdxStrict_tree(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->spdxStrictClassifier;

	// Test some complex licences with parentheses.
	{
		struct LicenceTreeNode *left;
		make_ltn_simple(left, 1, "Good");

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Bad");
		make_ltn(right, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_AND, left, right);
		test_licence("Good AND (Good OR Bad)", expected);
	}
	{
		struct LicenceTreeNode *left;
		make_ltn_simple(left, 1, "Good");

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Bad");
		make_ltn(right, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_AND, left, right);
		test_licence("Good AND (Good AND Bad)", expected);
	}
	{
		struct LicenceTreeNode *left;
		make_ltn_simple(left, 0, "Bad");

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Bad");
		make_ltn(right, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("Bad OR (Good OR Bad)", expected);
	}
	{
		struct LicenceTreeNode *left;
		make_ltn_simple(left, 0, "Bad");

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Bad");
		make_ltn(right, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_OR, left, right);
		test_licence("Bad OR (Good AND Bad)", expected);
	}

	// Test licences where both sides are parenthesized.
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Bad");
		make_ltn(left, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good");
		make_ltn_simple(fourth, 1, "Awesome");
		make_ltn(right, 1, LTNT_OR, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_AND, left, right);
		test_licence("(Good OR Bad) AND (Good OR Awesome)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Bad");
		make_ltn(left, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good");
		make_ltn_simple(fourth, 1, "Awesome");
		make_ltn(right, 1, LTNT_AND, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_AND, left, right);
		test_licence("(Good OR Bad) AND (Good AND Awesome)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Bad");
		make_ltn(left, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 0, "Bad");
		make_ltn_simple(fourth, 0, "Awful");
		make_ltn(right, 0, LTNT_OR, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_AND, left, right);
		test_licence("(Good OR Bad) AND (Bad OR Awful)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Bad");
		make_ltn(left, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 0, "Bad");
		make_ltn_simple(fourth, 0, "Awful");
		make_ltn(right, 0, LTNT_AND, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_AND, left, right);
		test_licence("(Good OR Bad) AND (Bad AND Awful)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(left, 1, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good");
		make_ltn_simple(fourth, 1, "Long name with spaces");
		make_ltn(right, 1, LTNT_OR, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("(Good AND Awesome) OR (Good OR Long name with spaces)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Bad");
		make_ltn(left, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good");
		make_ltn_simple(fourth, 1, "Awesome");
		make_ltn(right, 1, LTNT_AND, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("(Good AND Bad) OR (Good AND Awesome)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Bad");
		make_ltn(left, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good");
		make_ltn_simple(fourth, 0, "Awful");
		make_ltn(right, 1, LTNT_OR, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("(Good AND Bad) OR (Good OR Awful)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Bad");
		make_ltn(left, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good");
		make_ltn_simple(fourth, 0, "Awful");
		make_ltn(right, 0, LTNT_AND, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_OR, left, right);
		test_licence("(Good AND Bad) OR (Good AND Awful)", expected);
	}
}

// The SPDX spec says that the "AND" operator takes precedence over "OR".
// This means that AND/OR can be mixed without parentheses.
void test__spdxStrict_precedence(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->spdxStrictClassifier;

	// Single AND pair
	{
		struct LicenceTreeNode *left;
		make_ltn_simple(left, 1, "Awesome");

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 1, "Good");
		make_ltn(right, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("Awesome OR Bad AND Good", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(left, 1, LTNT_AND, first, second);

		struct LicenceTreeNode *right;
		make_ltn_simple(right, 0, "Bad WITH Extra badness");

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("Good AND Awesome OR Bad WITH Extra badness", expected);
	}

	// Two AND pairs
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 0, "Awful");
		make_ltn(left, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good+ WITH More goodies");
		make_ltn_simple(fourth, 0, "Bad");
		make_ltn(right, 0, LTNT_AND, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_OR, left, right);
		test_licence("Bad AND Awful OR Good+ WITH More goodies AND Bad", expected);
	}

	// Three AND groups
	{
		struct LicenceTreeNode *first, *second, *third, *left;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn_simple(third, 1, "Long name with spaces");
		make_ltn(left, 1, LTNT_AND, first, second, third);

		struct LicenceTreeNode *fourth, *fifth, *middle;
		make_ltn_simple(fourth, 0, "Bad");
		make_ltn_simple(fifth, 0, "Awful");
		make_ltn(middle, 0, LTNT_AND, fourth, fifth);

		struct LicenceTreeNode *sixth, *seventh, *right;
		make_ltn_simple(sixth, 1, "Good");
		make_ltn_simple(seventh, 0, "Bad");
		make_ltn(right, 0, LTNT_AND, sixth, seventh);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, middle, right);
		test_licence("Good AND Awesome AND Long name with spaces OR Bad AND Awful OR Good AND Bad", expected);
	}
}
