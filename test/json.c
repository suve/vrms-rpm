/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2024 suve (a.k.a. Artur Frenszek-Iwicki)
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

#include "src/json.h"

#include <stdio.h>
#include <stdlib.h>

#define UNUSED(x) ((void)(x))

#define JSON_BUFSIZ (16 * 1024)

int test_setup__json(void **state) {
	char* buffer = malloc(JSON_BUFSIZ);
	*state = buffer;
	return 0;
}

int test_teardown__json(void **state) {
	char *buffer = *state;
	free(buffer);
	return 0;
}

void test__json(void **state) {
	char *buffer = *state;
	FILE *f = fmemopen(buffer, JSON_BUFSIZ, "w");

	struct JsonObject *obj = jsonObj_open(f, 0);
	jsonObj_pushInt(obj, "i", 0);
	jsonObj_pushInt(obj, "ii", 22);
	jsonObj_pushInt(obj, "iii", 333);
	jsonObj_pushStr(obj, "string", "this is a \"string\"");
	jsonObj_close(obj);

	putc('\0', f);
	fflush(f);
	fclose(f);

	assert_string_equal(buffer,
		"{\"i\":0,\"ii\":22,\"iii\":333,\"string\":\"this is a \\\"string\\\"\"}"
	);
}
