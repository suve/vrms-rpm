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

const size_t CHUNK_SIZE = 4096;

struct buffer {
	char* data;
	size_t capacity;
	size_t size;
};

void buffer_cleanup(struct buffer* buf) {
	free(buf->data);
	memset(buf, 0, sizeof(*buf));
}

int buffer_append(struct buffer* buf, const char* data, size_t size) {
	if (buf->size + size > buf->capacity) {
		size_t new_capacity = buf->size + size;
		if (new_capacity % CHUNK_SIZE) {
			new_capacity = ((new_capacity / CHUNK_SIZE) + 1) * CHUNK_SIZE;
		}
		char* new_data = realloc(buf->data, new_capacity);
		if (!new_data) {
			return -1;
		}
		buf->data = new_data;
		buf->capacity = new_capacity;
	}

	memcpy(&buf->data[buf->size], data, size);
	buf->size += size;
	return 0;
}

char* read_stdin() {
	struct buffer buf = {0};

	while (!feof(stdin)) {
		char chunk[CHUNK_SIZE];
		size_t read = fread(chunk, 1, sizeof(chunk), stdin);
		if (buffer_append(&buf, chunk, read)) {
			buffer_cleanup(&buf);
			return NULL;
		}
	}
	buffer_append(&buf, "", 1);
	return buf.data;
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
