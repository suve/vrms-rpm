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
void test__spdxStrict_single(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->spdxStrictClassifier;

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

// Test licence strings using "WITH" and "+" operators.
void test__spdxStrict_suffixes(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->spdxStrictClassifier;

	// A good licence with the "+" operator should be recognized as a good licence.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good+", NULL);
		test_licence("Good+", expected);
	}
	// In strict mode, there can be no whitespace between the licence name and the "+".
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Good +", NULL);
		test_licence("Good +", expected);
	}
	// A bad licence with the "+" operator is still bad.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad+", NULL);
		test_licence("Bad+", expected);
	}

	// A good licence with the "WITH" operator should be recognized as a good licence.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good", "Extra-Permissions");
		test_licence("Good WITH Extra-Permissions", expected);
	}
	// The "WITH" operator can be preceded by even more whitespace.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good", "Extra-Permissions");
		test_licence("Good   WITH Extra-Permissions", expected);
	}
	// In strict mode, "WITH" must be matched case-sensitively.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Good With Extra-Permissions", NULL);
		test_licence("Good With Extra-Permissions", expected);
	}
	// A bad licence with the "WITH" operator is still bad.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad", "Even-More-Badness");
		test_licence("Bad WITH Even-More-Badness", expected);
	}
}

// Test licence strings that evaluate to a single-level and/or chain.
void test__spdxStrict_one_level(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->spdxStrictClassifier;

	// Test some simple "A OR B" licences.
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(expected, 1, LTNT_OR, first, second);

		test_licence("Good OR Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(expected, 1, LTNT_OR, first, second);

		test_licence("Good OR Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(expected, 1, LTNT_OR, first, second);

		test_licence("Bad OR Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(expected, 0, LTNT_OR, first, second);

		test_licence("Bad OR Awful", expected);
	}

	// Test some simple "A AND B" licences.
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(expected, 1, LTNT_AND, first, second);

		test_licence("Good AND Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(expected, 0, LTNT_AND, first, second);

		test_licence("Good AND Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(expected, 0, LTNT_AND, first, second);

		test_licence("Bad AND Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(expected, 0, LTNT_AND, first, second);

		test_licence("Bad AND Awful", expected);
	}

	// Test chains with more than 2 sub-licences.
	{
		struct LicenceTreeNode *first, *second, *third, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn_simple(third, 0, "Bad", NULL);
		make_ltn(expected, 0, LTNT_AND, first, second, third);

		test_licence("Good AND Awesome AND Bad", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *third, *fourth, *expected;
		make_ltn_simple(first, 0, "Awful", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn_simple(third, 0, "Unknown", NULL);
		make_ltn_simple(fourth, 1, "Good", NULL);
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
		make_ltn_simple(left, 1, "Good", NULL);

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(right, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_AND, left, right);
		test_licence("Good AND (Good OR Bad)", expected);
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
		test_licence("Good AND (Good AND Bad)", expected);
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
		test_licence("Bad OR (Good OR Bad)", expected);
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
		test_licence("Bad OR (Good AND Bad)", expected);
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
		test_licence("(Good OR Bad) AND (Good OR Awesome)", expected);
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
		test_licence("(Good OR Bad) AND (Good AND Awesome)", expected);
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
		test_licence("(Good OR Bad) AND (Bad OR Awful)", expected);
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
		test_licence("(Good OR Bad) AND (Bad AND Awful)", expected);
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
		test_licence("(Good AND Awesome) OR (Good OR Long name with spaces)", expected);
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
		test_licence("(Good AND Bad) OR (Good AND Awesome)", expected);
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
		test_licence("(Good AND Bad) OR (Good OR Awful)", expected);
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
		make_ltn_simple(left, 1, "Awesome", NULL);

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 1, "Good", NULL);
		make_ltn(right, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("Awesome OR Bad AND Good", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(left, 1, LTNT_AND, first, second);

		struct LicenceTreeNode *right;
		make_ltn_simple(right, 0, "Bad", "Extra badness");

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("Good AND Awesome OR Bad WITH Extra badness", expected);
	}

	// Two AND pairs
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(left, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good+", "More goodies");
		make_ltn_simple(fourth, 0, "Bad", NULL);
		make_ltn(right, 0, LTNT_AND, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_OR, left, right);
		test_licence("Bad AND Awful OR Good+ WITH More goodies AND Bad", expected);
	}

	// Three AND groups
	{
		struct LicenceTreeNode *first, *second, *third, *left;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn_simple(third, 1, "Long name with spaces", NULL);
		make_ltn(left, 1, LTNT_AND, first, second, third);

		struct LicenceTreeNode *fourth, *fifth, *middle;
		make_ltn_simple(fourth, 0, "Bad", NULL);
		make_ltn_simple(fifth, 0, "Awful", NULL);
		make_ltn(middle, 0, LTNT_AND, fourth, fifth);

		struct LicenceTreeNode *sixth, *seventh, *right;
		make_ltn_simple(sixth, 1, "Good", NULL);
		make_ltn_simple(seventh, 0, "Bad", NULL);
		make_ltn(right, 0, LTNT_AND, sixth, seventh);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, middle, right);
		test_licence("Good AND Awesome AND Long name with spaces OR Bad AND Awful OR Good AND Bad", expected);
	}
}

// Quote from the SPDX spec (Annex D: SPDX license expressions):
//   > There MUST be white space and/or parentheses on either side of the operators AND and OR.
// This means that, when the AND/OR operator appears near parentheses, whitespace is not required.
void test__spdxStrict_whitespace(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->spdxStrictClassifier;

	// No whitespace before operator, simple licence expression after operator,
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 1, "Awesome", NULL);
		make_ltn(left, 1, LTNT_AND, first, second);

		struct LicenceTreeNode *right;
		make_ltn_simple(right, 0, "Bad", NULL);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("(Good AND Awesome)OR Bad", expected);
	}
	// No whitespace before operator, parenthesized expression after operator.
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Bad", NULL);
		make_ltn(left, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 0, "Awful", NULL);
		make_ltn_simple(fourth, 1, "Awesome", NULL);
		make_ltn(right, 0, LTNT_AND, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_OR, left, right);
		test_licence("(Good AND Bad)OR (Awful AND Awesome)", expected);
	}

	// No whitespace after operator. Simple licence expression before it.
	{
		struct LicenceTreeNode *left;
		make_ltn_simple(left, 1, "Awesome", NULL);

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 1, "Good", NULL);
		make_ltn(right, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_AND, left, right);
		test_licence("Awesome AND(Bad OR Good)", expected);
	}
	// No whitespace after operator. Parenthesized expression before it.
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(left, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good", NULL);
		make_ltn_simple(fourth, 0, "Atrocious", NULL);
		make_ltn(right, 1, LTNT_OR, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_OR, left, right);
		test_licence("(Bad AND Awful) OR(Good OR Atrocious)", expected);
	}

	// No whitespace on either side.
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 1, "Good", NULL);
		make_ltn(left, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 0, "Awful", NULL);
		make_ltn_simple(fourth, 1, "Awesome", NULL);
		make_ltn(right, 1, LTNT_OR, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 1, LTNT_AND, left, right);
		test_licence("(Bad OR Good)AND(Awful OR Awesome)", expected);
	}

	// While we're at it, test if extra whitespace around parentheses won't mess anything up.
	{
		struct LicenceTreeNode *first, *second, *inner;
		make_ltn_simple(first, 0, "Bad", NULL);
		make_ltn_simple(second, 1, "Good", NULL);
		make_ltn(inner, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *third, *left;
		make_ltn_simple(third, 0, "Atrocious", NULL);
		make_ltn(left, 0, LTNT_AND, inner, third);

		struct LicenceTreeNode *fourth, *fifth, *right;
		make_ltn_simple(fourth, 0, "Awful", NULL);
		make_ltn_simple(fifth, 1, "Awesome", NULL);
		make_ltn(right, 0, LTNT_AND, fourth, fifth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_OR, left, right);
		test_licence("( ( Bad OR Good ) AND Atrocious ) OR ( Awful AND Awesome )", expected);
	}
}

void test__spdxStrict_caseSensitivity(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->spdxStrictClassifier;

	// The SPDX spec mandates that licence names must be matched case-insensitively.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "good", NULL);
		test_licence("good", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "AwEsOmE", NULL);
		test_licence("AwEsOmE", expected);
	}

	// The SPDX spec mandates that operators (AND/OR) must be matched in a case-sensitive manner.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Good or Awful", NULL);
		test_licence("Good or Awful", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Awesome and Good", NULL);
		test_licence("Awesome and Good", expected);
	}
}

// Test some strings that don't really make sense.
void test__spdxStrict_mangledStrings(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->spdxStrictClassifier;

	// Valid licence string, but wrapped in extra parentheses
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good", NULL);
		test_licence("(Good)", expected);
	}
	// Gimme more!
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good", NULL);
		test_licence("(((Good)))", expected);
	}

	// Licence string with parenthesized text - at the start
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "(Very) Bad Licence", NULL);
		test_licence("(Very) Bad Licence", expected);
	}
	// In the middle
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Unknown (Alien) Licence", NULL);
		test_licence("Unknown (Alien) Licence", expected);
	}
	// At the end
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Absolutely Proprietary (eww)", NULL);
		test_licence("Absolutely Proprietary (eww)", expected);
	}
	// Both start and end
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "(Wow!) Friendly LicenceTuber gets sued by BigCorp! (MUST SEE!)", NULL);
		test_licence("(Wow!) Friendly LicenceTuber gets sued by BigCorp! (MUST SEE!)", expected);
	}

	// WITH operator present, but no licensing exception (string ends)
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Empty Licensing Exception String", "");
		test_licence("(Empty Licensing Exception String WITH )", expected);
	}
	// WITH operator present, but no license
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "", "Lol Where's The Licence");
		test_licence("( WITH Lol Where's The Licence)", expected);
	}

	// AND joiner present, but no licence preceding it
	{
		struct LicenceTreeNode *left, *right, *expected;
		make_ltn_simple(left, 0, "", NULL);
		make_ltn_simple(right, 0, "Stuff", NULL);
		make_ltn(expected, 0, LTNT_AND, left, right);
		test_licence("( AND Stuff)", expected);
	}
	// AND joiner present, but no licence after it
	{
		struct LicenceTreeNode *left, *right, *expected;
		make_ltn_simple(left, 0, "Other Stuff", NULL);
		make_ltn_simple(right, 0, "", NULL);
		make_ltn(expected, 0, LTNT_AND, left, right);
		test_licence("(Other Stuff AND )", NULL);
	}

	// OR joiner present, but no licence preceding it
	{
		struct LicenceTreeNode *left, *right, *expected;
		make_ltn_simple(left, 0, "", NULL);
		make_ltn_simple(right, 0, "Things", NULL);
		make_ltn(expected, 0, LTNT_OR, left, right);
		test_licence("( OR Things)", NULL);
	}
	// OR joiner present, but no licence after it
	{
		struct LicenceTreeNode *left, *right, *expected;
		make_ltn_simple(left, 0, "More Things", NULL);
		make_ltn_simple(right, 0, "", NULL);
		make_ltn(expected, 0, LTNT_OR, left, right);
		test_licence("(More Things OR )", NULL);
	}
}

// Test behaviour specific to SPDX classifier's lenient mode.
void test__spdxLenient(void **state) {
	struct LicenceClassifier *classifier = ((struct TestState*)*state)->spdxLenientClassifier;

	// "+" operator: in lenient mode, permit whitespace between the licence name and the "+".
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good +", NULL);
		test_licence("Good +", expected);
	}
	// Obviously, a bad licence is still bad, even with a plus.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Not Approved +", NULL);
		test_licence("Not Approved +", expected);
	}

	// "WITH" operator: in lenient mode, we perform a case-insensitive match.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Awesome", "Extra-Permissions");
		test_licence("Awesome with Extra-Permissions", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good", "More Goodies");
		test_licence("Good WitH More Goodies", expected);
	}
	// Lenient mode or not, a bad licence with extra permissions is still bad.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Awful", "Extra Stink");
		test_licence("Awful With Extra Stink", expected);
	}

	// "AND"/"OR": once again, the spec says "case matters", and we ignore that in lenient mode.
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good", NULL);
		make_ltn_simple(second, 0, "Awful", NULL);
		make_ltn(expected, 1, LTNT_OR, first, second);

		test_licence("Good or Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Awesome", NULL);
		make_ltn_simple(second, 1, "good", NULL);
		make_ltn(expected, 1, LTNT_AND, first, second);

		test_licence("Awesome and good", expected);
	}

	// Combine all of the above
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 0, "Awful", NULL);
		make_ltn_simple(second, 1, "Awesome +", NULL);
		make_ltn(left, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *third, *fourth, *right;
		make_ltn_simple(third, 1, "Good", "More Goodies");
		make_ltn_simple(fourth, 0, "Bad", NULL);
		make_ltn(right, 0, LTNT_AND, third, fourth);

		struct LicenceTreeNode *expected;
		make_ltn(expected, 0, LTNT_OR, left, right);
		test_licence("Awful And Awesome + or Good With More Goodies anD Bad", expected);
	}
}
