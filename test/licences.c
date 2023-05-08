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

// The arg/def/jmp includes are required by cmocka.
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/classifiers.h"
#include "../src/options.h"

#define UNUSED(x) ((void)(x))

// Dirty dirty dirty hack!
extern char *argv_zero;

int test_setup__licences(void **state) {
	// Create the path to the test licence list.
	// Assumes that the test-suite executable is inside build/.
	char buffer[1024];
	strcpy(buffer, argv_zero);
	*strrchr(buffer, '/') = '\0'; // remove the executable name
	*strrchr(buffer, '/') = '\0'; // remove the build/ directory
	const size_t buflen = strlen(buffer);
	strcpy(buffer + buflen, "/test/licences.txt");

	opt_licencelist = buffer;

	struct LicenceData *licences = licences_read();
	assert_non_null(licences);
	struct LicenceClassifier *classifier = classifier_newLoose(licences);
	assert_non_null(classifier);

	*state = classifier;
	return 0;
}

int test_teardown__licences(void **state) {
	struct LicenceClassifier *classifier = *state;
	struct LicenceData *data = (struct LicenceData*)classifier->data;

	classifier->free(classifier);
	licences_free(data);

	opt_licencelist = NULL;
	return 0;
}

static void ltn_to_str(char *buffer, const size_t bufsize, const struct LicenceTreeNode *ltn) {
	if(ltn->type == LTNT_LICENCE) {
		snprintf(buffer, bufsize, "{type = LICENCE, is_free = %d, licence = \"%s\"}", ltn->is_free, ltn->licence);
	} else {
		snprintf(buffer, bufsize, "{type = %s, is_free = %d, members = %d}", (ltn->type == LTNT_AND) ? "AND" : (ltn->type == LTNT_OR) ? "OR": "???", ltn->is_free, ltn->members);
	}
}

static void assert_ltn_equal(const struct LicenceTreeNode *actual, const struct LicenceTreeNode *expected, const char *const file, const int line) {
	char bufAct[128], bufExp[128];
	ltn_to_str(bufAct, sizeof(bufAct), actual);
	ltn_to_str(bufExp, sizeof(bufExp), expected);
	_assert_string_equal(bufAct, bufExp, file, line);

	if(expected->type != LTNT_LICENCE) {
		for(int i = 0; i < expected->members; ++i) {
			assert_ltn_equal(actual->child[i], expected->child[i], file, line);
		}
	}
}

#define make_ltn_simple(name, pop_is_free, pop_licence) do{ \
	(name) = malloc(sizeof(struct LicenceTreeNode)); \
	(name)->type = LTNT_LICENCE; \
	(name)->is_free = (pop_is_free); \
	(name)->licence = (pop_licence); \
} while(0)

#define make_ltn(name, pop_is_free, pop_type, ...) do{ \
	struct LicenceTreeNode *args[] = { __VA_ARGS__ }; \
	const int count = sizeof(args) / sizeof(struct LicenceTreeNode*); \
\
	name = malloc(sizeof(struct LicenceTreeNode) + (count * sizeof(struct LicenceTreeNode*))); \
	(name)->is_free = (pop_is_free); \
	(name)->type = (pop_type); \
	(name)->members = count; \
	for(int i = 0; i < count; ++i) { \
		(name)->child[i] = args[i]; \
	} \
} while(0)

// The licence text must be writable, hence we use the buffer[] trick.
#define testcase(text, expected) do{ \
	char buffer[] = (text); \
	struct LicenceTreeNode *ltn = classifier->classify(classifier, buffer); \
	assert_non_null(ltn); \
	if((expected) != NULL) { \
		assert_ltn_equal(ltn, (expected), __FILE__, __LINE__); \
		licence_freeTree(expected); \
	} \
	licence_freeTree(ltn); \
} while(0)

// Test license strings that evaluate to a single licence.
void test__licences_single(void **state) {
	struct LicenceClassifier *classifier = *state;

	// Test some simple good licences.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Awesome");
		testcase("Awesome", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good");
		testcase("Good", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Long name with spaces");
		testcase("Long name with spaces", expected);
	}

	// Test some simple bad licences.
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Awful");
		testcase("Awful", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad");
		testcase("Bad", expected);
	}
}

// Test licence strings that evaluate to a single-level and/or chain.
void test__licences_one_level(void **state) {
	struct LicenceClassifier *classifier = *state;

	// Test some simple "A or B" licences.
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(expected, 1, LTNT_OR, first, second);

		testcase("Good or Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Awful");
		make_ltn(expected, 1, LTNT_OR, first, second);

		testcase("Good or Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(expected, 1, LTNT_OR, first, second);

		testcase("Bad or Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 0, "Awful");
		make_ltn(expected, 0, LTNT_OR, first, second);

		testcase("Bad or Awful", expected);
	}

	// Test some simple "A and B" licences.
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(expected, 1, LTNT_AND, first, second);

		testcase("Good and Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Awful");
		make_ltn(expected, 0, LTNT_AND, first, second);

		testcase("Good and Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(expected, 0, LTNT_AND, first, second);

		testcase("Bad and Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 0, "Awful");
		make_ltn(expected, 0, LTNT_AND, first, second);

		testcase("Bad and Awful", expected);
	}

	// Test chains with more than 2 sub-licences.
	{
		struct LicenceTreeNode *first, *second, *third, *expected;
		make_ltn_simple(first, 0, "Awful");
		make_ltn_simple(second, 0, "Bad");
		make_ltn_simple(third, 0, "Unknown");
		make_ltn(expected, 0, LTNT_OR, first, second, third);

		testcase("Awful or Bad or Unknown", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *third, *fourth, *expected;
		make_ltn_simple(first, 0, "Awful");
		make_ltn_simple(second, 0, "Bad");
		make_ltn_simple(third, 0, "Unknown");
		make_ltn_simple(fourth, 1, "Good");
		make_ltn(expected, 1, LTNT_OR, first, second, third, fourth);

		testcase("Awful or Bad or Unknown or Good", expected);
	}
}

// Test licence strings that evaluate to a tree.
void test__licences_tree(void **state) {
	struct LicenceClassifier *classifier = *state;

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
		testcase("Good and (Good or Bad)", expected);
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
		testcase("Good and (Good and Bad)", expected);
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
		testcase("Bad or (Good or Bad)", expected);
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
		testcase("Bad or (Good and Bad)", expected);
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
		testcase("(Good or Bad) and (Good or Awesome)", expected);
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
		testcase("(Good or Bad) and (Good and Awesome)", expected);
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
		testcase("(Good or Bad) and (Bad or Awful)", expected);
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
		testcase("(Good or Bad) and (Bad and Awful)", expected);
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
		testcase("(Good and Awesome) or (Good or Long name with spaces)", expected);
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
		testcase("(Good and Bad) or (Good and Awesome)", expected);
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
		testcase("(Good and Bad) or (Good or Awful)", expected);
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
		testcase("(Good and Bad) or (Good and Awful)", expected);
	}
}

// Test some licence strings with spurious parentheses
void test__licences_extra_parentheses(void **state) {
	struct LicenceClassifier *classifier = *state;

	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good");
		testcase("(Good)", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad");
		testcase("((Bad))", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Awful");
		testcase("((Awful))", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *third, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 0, "Bad");
		make_ltn_simple(third, 1, "Awesome");
		make_ltn(expected, 0, LTNT_AND, first, second, third);
		testcase("Good and (Bad) and Awesome", expected);
	}
}

// Test whether joiners ("and"/"or") are case-insensitive
void test__licences_case_insensitive_joiners(void **state) {
	struct LicenceClassifier *classifier = *state;

	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(expected, 1, LTNT_AND, first, second);

		testcase("Good And Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(expected, 1, LTNT_AND, first, second);

		testcase("Good anD Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 1, "Good");
		make_ltn_simple(second, 1, "Awesome");
		make_ltn(expected, 1, LTNT_AND, first, second);

		testcase("Good AND Awesome", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 0, "Awful");
		make_ltn(expected, 0, LTNT_OR, first, second);

		testcase("Bad Or Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 0, "Awful");
		make_ltn(expected, 0, LTNT_OR, first, second);

		testcase("Bad oR Awful", expected);
	}
	{
		struct LicenceTreeNode *first, *second, *expected;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 0, "Awful");
		make_ltn(expected, 0, LTNT_OR, first, second);

		testcase("Bad OR Awful", expected);
	}
}

void test__licences_acceptable_suffixes(void **state) {
	struct LicenceClassifier *classifier = *state;

	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Good with acknowledgement");
		testcase("Good with acknowledgement", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Awesome with additional permissions");
		testcase("Awesome with additional permissions", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 1, "Long name with spaces with linking exception");
		testcase("Long name with spaces with linking exception", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad with acknowledgement");
		testcase("Bad with acknowledgement", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad with additional permissions");
		testcase("Bad with additional permissions", expected);
	}
	{
		struct LicenceTreeNode *expected;
		make_ltn_simple(expected, 0, "Bad with linking exception");
		testcase("Bad with linking exception", expected);
	}
}

// Test some licence strings with mismatched parentheses.
// We're mostly concerned about avoiding segfaults.
// Trying to make sense out of the string is a secondary concern.
void test__licences_mismatched_parentheses(void **state) {
	struct LicenceClassifier *classifier = *state;

	testcase("(Bad", NULL);
	testcase("Bad)", NULL);
	testcase("(", NULL);
	testcase(")", NULL);
	testcase("(()", NULL);
	testcase("())", NULL);

	testcase("Good and (Bad", NULL);
	testcase("Good and Bad)", NULL);

	testcase("Good and (Bad or Awesome", NULL);
	testcase("Good and Bad) or Awesome", NULL);

	testcase("(Good and (Bad or Awesome)", NULL);
	testcase("Good and (Bad or Awesome)) and Long name with spaces", NULL);

	testcase("Good and ", NULL);
	testcase("Good or ", NULL);
	testcase("and Awful", NULL);
	testcase("or Awesome", NULL);
	testcase(" and ", NULL);
	testcase(" or ", NULL);
}
