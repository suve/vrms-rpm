/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2023 Marcin "dextero" Radomski
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

#include "../licences.h"

#include <stdio.h>
#include <stdlib.h>

#include "buffers.h"

char* read_stdin() {
	const size_t CHUNK_SIZE = 4096;
	struct ReBuffer* buf = rebuf_init();

	while (!feof(stdin)) {
		char chunk[CHUNK_SIZE];
		size_t read = fread(chunk, 1, sizeof(chunk), stdin);
		if (rebuf_append(buf, chunk, read)) {
			rebuf_free(buf);
			return NULL;
		}
	}
	rebuf_append(buf, "", 1);

	char* result = buf.data;
	buf.data = NULL;
	rebuf_free(buf);
	return result;
}

int main(int argc, char* argv[]) {
	(void)argc;

	extern char* argv_zero;
	argv_zero = argv[0];

	struct TestState* state = &(struct TestState){0};
	if (test_setup__licences((void**)&state)) {
		return -1;
	}
	struct LicenceClassifier* strict = state->spdxStrictClassifier;
	struct LicenceClassifier* lenient = state->spdxLenientClassifier;
	struct LicenceTreeNode* ltn = NULL;
	char* input = read_stdin();

	ltn = strict->classify(strict, input);
	licence_freeTree(ltn);

	ltn = lenient->classify(lenient, input);
	licence_freeTree(ltn);

	free(input);
	test_teardown__licences((void**)&state);
}
