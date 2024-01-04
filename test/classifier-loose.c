/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2021-2024 suve (a.k.a. Artur Frenszek-Iwicki)
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
void test__looseClassifier_single(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->looseClassifier;

	// Test some simple good licences.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Awesome", NULL);
		test_licence("Awesome", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good", NULL);
		test_licence("Good", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Long name with spaces", NULL);
		test_licence("Long name with spaces", expected);
	}

	// Test some simple bad licences.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Awful", NULL);
		test_licence("Awful", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad", NULL);
		test_licence("Bad", expected);
	}
}

// Test licence strings that evaluate to a single-level and/or chain.
void test__looseClassifier_one_level(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->looseClassifier;

	// Test some simple "A or B" licences.
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(expected, 1, LTNT_OR, first, second);

		test_licence("Good or Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(expected, 1, LTNT_OR, first, second);

		test_licence("Good or Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(expected, 1, LTNT_OR, first, second);

		test_licence("Bad or Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(expected, 0, LTNT_OR, first, second);

		test_licence("Bad or Awful", expected);
	}

	// Test some simple "A and B" licences.
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(expected, 1, LTNT_AND, first, second);

		test_licence("Good and Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(expected, 0, LTNT_AND, first, second);

		test_licence("Good and Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(expected, 0, LTNT_AND, first, second);

		test_licence("Bad and Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(expected, 0, LTNT_AND, first, second);

		test_licence("Bad and Awful", expected);
	}

	// Test chains with more than 2 sub-licences.
	{
		struct LicenceTreeNode *first, *second, *third, *expected;
		make_ltn_simple(first, 0, "Awful", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn_simple(third, 0, "Unknown", NULL);
		make_ltn(expected, 0, LTNT_OR, first, second, third);

		test_licence("Awful or Bad or Unknown", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *third, *fourth, *expected;
		make_ltn_simple(first, 0, "Awful", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn_simple(third, 0, "Unknown", NULL);
		make_ltn_simple(fourth, 1, "Good", NULL);
		make_ltn(expected, 1, LTNT_OR, first, second, third, fourth);

		test_licence("Awful or Bad or Unknown or Good", expected);
	}
}

// Test licence strings that evaluate to a tree.
void test__looseClassifier_tree(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->looseClassifier;

	// Test some complex licences with parentheses.
	{
		struct LicenceTreeNode *left;
		make_ltn_simple(left, 1, "Good", NULL);

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(right, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_AND, left, right);
		test_licence("Good and (Good or Bad)", expected);
	}
	{
		struct LicenceTreeNode *left;
		make_ltn_simple(left, 1, "Good", NULL);

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(right, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_AND, left, right);
		test_licence("Good and (Good and Bad)", expected);
	}
	{
		struct LicenceTreeNode *left;
		make_ltn_simple(left, 0, "Bad", NULL);

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(right, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("Bad or (Good or Bad)", expected);
	}
	{
		struct LicenceTreeNode *left;
		make_ltn_simple(left, 0, "Bad", NULL);

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(right, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_OR, left, right);
		test_licence("Bad or (Good and Bad)", expected);
	}

	// Test licences where both sides are parenthesized.
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(left, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good", NULL);
		make_ltn_simple(fourth, 1, "Awesome", NULL);
		make_ltn(right, 1, LTNT_OR, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_AND, left, right);
		test_licence("(Good or Bad) and (Good or Awesome)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(left, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good", NULL);
		make_ltn_simple(fourth, 1, "Awesome", NULL);
		make_ltn(right, 1, LTNT_AND, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_AND, left, right);
		test_licence("(Good or Bad) and (Good and Awesome)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(left, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 0, "Bad", NULL);
		make_ltn_simple(fourth, 0, "Awful", NULL);
		make_ltn(right, 0, LTNT_OR, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_AND, left, right);
		test_licence("(Good or Bad) and (Bad or Awful)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(left, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 0, "Bad", NULL);
		make_ltn_simple(fourth, 0, "Awful", NULL);
		make_ltn(right, 0, LTNT_AND, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_AND, left, right);
		test_licence("(Good or Bad) and (Bad and Awful)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(left, 1, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good", NULL);
		make_ltn_simple(fourth, 1, "Long name with spaces", NULL);
		make_ltn(right, 1, LTNT_OR, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("(Good and Awesome) or (Good or Long name with spaces)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(left, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good", NULL);
		make_ltn_simple(fourth, 1, "Awesome", NULL);
		make_ltn(right, 1, LTNT_AND, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("(Good and Bad) or (Good and Awesome)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(left, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good", NULL);
		make_ltn_simple(fourth, 0, "Awful", NULL);
		make_ltn(right, 1, LTNT_OR, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("(Good and Bad) or (Good or Awful)", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(left, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good", NULL);
		make_ltn_simple(fourth, 0, "Awful", NULL);
		make_ltn(right, 0, LTNT_AND, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_OR, left, right);
		test_licence("(Good and Bad) or (Good and Awful)", expected);
	}
}

// Test some licence strings with spurious parentheses
void test__looseClassifier_extra_parentheses(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->looseClassifier;

	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good", NULL);
		test_licence("(Good)", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad", NULL);
		test_licence("((Bad))", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Awful", NULL);
		test_licence("((Awful))", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *third, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn_simple(third, 1, "Awesome", NULL);
		make_ltn(expected, 0, LTNT_AND, first, second, third);
		test_licence("Good and (Bad) and Awesome", expected);
	}
}

// Test whether joiners ("and"/"or") are case-insensitive
void test__looseClassifier_case_insensitive_joiners(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->looseClassifier;

	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(expected, 1, LTNT_AND, first, second);

		test_licence("Good And Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(expected, 1, LTNT_AND, first, second);

		test_licence("Good anD Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(expected, 1, LTNT_AND, first, second);

		test_licence("Good AND Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(expected, 0, LTNT_OR, first, second);

		test_licence("Bad Or Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(expected, 0, LTNT_OR, first, second);

		test_licence("Bad oR Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(expected, 0, LTNT_OR, first, second);

		test_licence("Bad OR Awful", expected);
	}
}

void test__looseClassifier_acceptable_suffixes(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->looseClassifier;

	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good", " with acknowledgement");
		test_licence("Good with acknowledgement", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Awesome", " with additional permissions");
		test_licence("Awesome with additional permissions", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Long name with spaces", " with linking exception");
		test_licence("Long name with spaces with linking exception", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad", " with acknowledgement");
		test_licence("Bad with acknowledgement", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad", " with additional permissions");
		test_licence("Bad with additional permissions", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad", " with linking exception");
		test_licence("Bad with linking exception", expected);
	}
}

// Test some licence strings with mismatched parentheses.
// We're mostly concerned about avoiding segfaults.
// Trying to make sense out of the string is a secondary concern.
void test__looseClassifier_mismatched_parentheses(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->looseClassifier;

	test_licence("(Bad", NULL);
	test_licence("Bad)", NULL);
	test_licence("(", NULL);
	test_licence(")", NULL);
	test_licence("(()", NULL);
	test_licence("())", NULL);

	test_licence("Good and (Bad", NULL);
	test_licence("Good and Bad)", NULL);

	test_licence("Good and (Bad or Awesome", NULL);
	test_licence("Good and Bad) or Awesome", NULL);

	test_licence("(Good and (Bad or Awesome)", NULL);
	test_licence("Good and (Bad or Awesome)) and Long name with spaces", NULL);

	test_licence("Good and ", NULL);
	test_licence("Good or ", NULL);
	test_licence("and Awful", NULL);
	test_licence("or Awesome", NULL);
	test_licence(" and ", NULL);
	test_licence(" or ", NULL);
}
