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
#include <stdio.h>
#include <stdlib.h>

#include "licences.h"
#include "../src/options.h"

char *argv_zero;

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

	struct TestState *ts = malloc(sizeof(struct TestState));
	ts->data = licences;
	ts->class = classifier;

	*state = ts;
	return 0;
}

int test_teardown__licences(void **state) {
	struct TestState *ts = *state;

	ts->class->free(ts->class);
	licences_free(ts->data);

	opt_licencelist = NULL;
	return 0;
}

void ltn_to_str(char *buffer, const size_t bufsize, const struct LicenceTreeNode *ltn) {
	if(ltn->type == LTNT_LICENCE) {
		snprintf(buffer, bufsize, "{type = LICENCE, is_free = %d, licence = \"%s\"}", ltn->is_free, ltn->licence);
	} else {
		snprintf(buffer, bufsize, "{type = %s, is_free = %d, members = %d}", (ltn->type == LTNT_AND) ? "AND" : (ltn->type == LTNT_OR) ? "OR": "???", ltn->is_free, ltn->members);
	}
}

void assert_ltn_equal(const struct LicenceTreeNode *actual, const struct LicenceTreeNode *expected, const char *const file, const int line) {
	char bufAct[128], bufExp[128];
	ltn_to_str(bufAct, sizeof(bufAct), actual);
	ltn_to_str(bufExp, sizeof(bufExp), expected);
	_assert_string_equal(bufAct, bufExp, file, line);

	if(expected->type != LTNT_LICENCE) {
		for(unsigned int i = 0; i < expected->members; ++i) {
			assert_ltn_equal(actual->child[i], expected->child[i], file, line);
		}
	}
}
