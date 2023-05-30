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
#ifndef TEST_LICENCES_H
#define TEST_LICENCES_H

// The arg/def/jmp includes are required by cmocka.
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>
#include <string.h>

#include "src/classifiers.h"
#include "src/licences.h"

struct TestState {
	struct LicenceData *data;
	struct LicenceClassifier *looseClassifier;
	struct LicenceClassifier *spdxStrictClassifier;
	struct LicenceClassifier *spdxLenientClassifier;
};

extern int test_setup__licences(void **state);
extern int test_teardown__licences(void **state);

extern void test__looseClassifier_single(void **state);
extern void test__looseClassifier_one_level(void **state);
extern void test__looseClassifier_tree(void **state);
extern void test__looseClassifier_extra_parentheses(void **state);
extern void test__looseClassifier_case_insensitive_joiners(void **state);
extern void test__looseClassifier_acceptable_suffixes(void **state);
extern void test__looseClassifier_mismatched_parentheses(void **state);

extern void test__spdxStrict_single(void **state);
extern void test__spdxStrict_suffixes(void **state);
extern void test__spdxStrict_one_level(void **state);
extern void test__spdxStrict_tree(void **state);
extern void test__spdxStrict_precedence(void **state);
extern void test__spdxStrict_whitespace(void **state);
extern void test__spdxStrict_caseSensitivity(void **state);
extern void test__spdxStrict_mangledStrings(void **state);

extern void test__spdxLenient(void **state);

extern void assert_ltn_equal(const struct LicenceTreeNode *actual, const struct LicenceTreeNode *expected, const char *const file, const int line);

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
#define test_licence(text, expected) do{ \
	char buffer[] = (text); \
	struct LicenceTreeNode *ltn = classifier->classify(classifier, buffer); \
	assert_non_null(ltn); \
	if((expected) != NULL) { \
		assert_ltn_equal(ltn, (expected), __FILE__, __LINE__); \
		licence_freeTree(expected); \
	} \
	licence_freeTree(ltn); \
} while(0)

#endif
