/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2024-2025 suve (a.k.a. Artur Frenszek-Iwicki)
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

static void simpleObject(struct JsonObject *obj, void *userdata) {
	UNUSED(userdata);

	jsonObj_pushInt(obj, "i", 0);
	jsonObj_pushInt(obj, "ii", 22);
	jsonObj_pushInt(obj, "iii", 333);
	jsonObj_pushStr(obj, "string", "this is a \"string\"");
}

static void nestedObject(struct JsonObject *obj, void *userdata) {
	int local = 0;
	int *ptr = (userdata != NULL) ? userdata : &local;
	if(*ptr > 2) return;

	jsonObj_pushInt(obj, "depth", *ptr);
	*ptr += 1;

	jsonObj_pushObj(obj, "child", &nestedObject, ptr);
}

static void arrayOfStrings(struct JsonArray *arr, void *userdata) {
	UNUSED(userdata);

	jsonArr_pushStr(arr, "one");
	jsonArr_pushStr(arr, "two");
	jsonArr_pushStr(arr, "three");
	jsonArr_pushStr(arr, "four");
}

static void dictionaryObject(struct JsonObject *obj, void *userdata) {
	const char **data = userdata;

	jsonObj_pushStr(obj, "true", data[0]);
	jsonObj_pushStr(obj, "false", data[1]);
}

static void arrayOfObjects(struct JsonArray *arr, void *userdata) {
	UNUSED(userdata);

	static const char* polish[] = {"prawda", "fałsz"};
	static const char* german[] = {"richtig", "falsch"};

	jsonArr_pushObj(arr, &dictionaryObject, polish);
	jsonArr_pushObj(arr, &dictionaryObject, german);
}

static void objectOfArrays(struct JsonObject *obj, void *userdata) {
	UNUSED(userdata);

	jsonObj_pushArr(obj, "strings", &arrayOfStrings, NULL);
	jsonObj_pushArr(obj, "objects", &arrayOfObjects, NULL);
}

#define testcase(func, ptr, pretty, expected) \
	do { \
		char *buffer = *state; \
		FILE *f = fmemopen(buffer, JSON_BUFSIZ, "w"); \
		json_new(f, (pretty), &(func), (ptr)); \
		putc('\0', f); \
		fflush(f); \
		fclose(f); \
		assert_string_equal(buffer, (expected)); \
	} while(0)

void test__json(void **state) {
	testcase(
		simpleObject, NULL, 0,
		"{\"i\":0,\"ii\":22,\"iii\":333,\"string\":\"this is a \\\"string\\\"\"}"
	);
	testcase(
		simpleObject, NULL, 1,
		"{\n"
		"\t\"i\": 0,\n"
		"\t\"ii\": 22,\n"
		"\t\"iii\": 333,\n"
		"\t\"string\": \"this is a \\\"string\\\"\"\n"
		"}"
	);

	testcase(
		nestedObject, NULL, 0,
		"{\"depth\":0,\"child\":{\"depth\":1,\"child\":{\"depth\":2,\"child\":{}}}}"
	);
	testcase(
		nestedObject, NULL, 1,
		"{\n"
		"\t\"depth\": 0,\n"
		"\t\"child\": {\n"
		"\t\t\"depth\": 1,\n"
		"\t\t\"child\": {\n"
		"\t\t\t\"depth\": 2,\n"
		"\t\t\t\"child\": {}\n"
		"\t\t}\n"
		"\t}\n"
		"}"
	);

	testcase(
		objectOfArrays, NULL, 0,
		"{\"strings\":[\"one\",\"two\",\"three\",\"four\"],\"objects\":[{\"true\":\"prawda\",\"false\":\"fałsz\"},{\"true\":\"richtig\",\"false\":\"falsch\"}]}"
	);
	testcase(
		objectOfArrays, NULL, 1,
		"{\n"
		"\t\"strings\": [\n"
		"\t\t\"one\",\n"
		"\t\t\"two\",\n"
		"\t\t\"three\",\n"
		"\t\t\"four\"\n"
		"\t],\n"
		"\t\"objects\": [\n"
		"\t\t{\n"
		"\t\t\t\"true\": \"prawda\",\n"
		"\t\t\t\"false\": \"fałsz\"\n"
		"\t\t},\n"
		"\t\t{\n"
		"\t\t\t\"true\": \"richtig\",\n"
		"\t\t\t\"false\": \"falsch\"\n"
		"\t\t}\n"
		"\t]\n"
		"}"
	);
}
