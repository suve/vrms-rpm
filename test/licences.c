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
#include <stdio.h>
#include <stdlib.h>

#include "src/buffers.h"
#include "test/licences.h"

static void append_licence(struct LicenceData *data, const char *text) {
	char *buffer_pos = chainbuf_append(&data->buffer, text);
	assert_non_null(buffer_pos);
	char *list_pos = rebuf_append(data->list, &buffer_pos, sizeof(buffer_pos));
	assert_non_null(list_pos);
}

int test_setup__licences(void **state) {
	struct LicenceData *licences = malloc(sizeof(struct LicenceData));
	assert_non_null(licences);
	licences->list = rebuf_init(4 * sizeof(void*));
	assert_non_null(licences->list);
	licences->buffer = chainbuf_init(200);
	assert_non_null(licences->buffer);

	append_licence(licences, "Awesome");
	append_licence(licences, "Good");
	append_licence(licences, "Long name with spaces");

	struct LicenceClassifier *looseClassifier = classifier_newLoose(licences);
	assert_non_null(looseClassifier);
	struct LicenceClassifier *spdxStrictClassifier = classifier_newSPDX(licences, 0);
	assert_non_null(spdxStrictClassifier);
	struct LicenceClassifier *spdxLenientClassifier = classifier_newSPDX(licences, 1);
	assert_non_null(spdxLenientClassifier);

	struct TestState *ts = malloc(sizeof(struct TestState));
	assert_non_null(ts);

	ts->data = licences;
	ts->looseClassifier = looseClassifier;
	ts->spdxStrictClassifier = spdxStrictClassifier;
	ts->spdxLenientClassifier = spdxLenientClassifier;

	*state = ts;
	return 0;
}

int test_teardown__licences(void **state) {
	struct TestState *ts = *state;
	ts->looseClassifier->free(ts->looseClassifier);
	ts->spdxStrictClassifier->free(ts->spdxStrictClassifier);
	ts->spdxLenientClassifier->free(ts->spdxLenientClassifier);
	licences_free(ts->data);
	free(ts);
	return 0;
}

static void ltn_to_str(char *buffer, const size_t bufsize, const struct LicenceTreeNode *ltn) {
	if(ltn->type == LTNT_LICENCE) {
		if(ltn->exception != NULL) {
			snprintf(buffer, bufsize,"{type = LICENCE, is_free = %d, licence = \"%s\", exception = \"%s\"}",ltn->is_free, ltn->licence, ltn->exception);
		} else {
			snprintf(buffer, bufsize,"{type = LICENCE, is_free = %d, licence = \"%s\", exception = NULL}",ltn->is_free, ltn->licence);
		}
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
