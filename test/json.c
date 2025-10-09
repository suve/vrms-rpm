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
#include <stdio.h>
#include <stdlib.h>

#include "src/json.h"
#include "test/test.h"

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

	jsonObj_pushInt(obj, "pos", 333);
	jsonObj_pushInt(obj, "neg", -42);
	jsonObj_pushStr(obj, "string", "this is a string");
	jsonObj_pushBool(obj, "T", 1);
	jsonObj_pushBool(obj, "F", 0);
}

static void stringyObject(struct JsonObject *obj, void *userdata) {
	UNUSED(userdata);

	jsonObj_pushStr(obj, "empty", "");
	jsonObj_pushStr(obj, "underEight", "\001\002\003\004\005\006\007");
	jsonObj_pushStr(obj, "underSixteen", "\010\011\012\013\014\015\016\017");
	jsonObj_pushStr(obj, "underTwentyFour", "\020\021\022\023\024\025\026\027");
	jsonObj_pushStr(obj, "underThirtyTwo", "\030\031\032\033\034\035\036\037");
	jsonObj_pushStr(obj, "quoted", "And then I said, \"Defend yourself, blackguard!\"");
	jsonObj_pushStr(obj, "regex", "/\\s+$//g");
	jsonObj_pushStr(obj, "utf-eight", "Pchnąć w tę łódź jeża i ośm skrzyń fig.");
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

void test__jsonTypes(void **state) {
	testcase(
		simpleObject, NULL, 0,
		"{\"pos\":333,\"neg\":-42,\"string\":\"this is a string\",\"T\":true,\"F\":false}"
	);
	testcase(
		simpleObject, NULL, 1,
		"{\n"
		"\t\"pos\": 333,\n"
		"\t\"neg\": -42,\n"
		"\t\"string\": \"this is a string\",\n"
		"\t\"T\": true,\n"
		"\t\"F\": false\n"
		"}"
	);
}

void test__jsonWeirdStrings(void **state) {
	testcase(
		stringyObject, NULL, 0,
		"{\"empty\":\"\",\"underEight\":\"\\u0001\\u0002\\u0003\\u0004\\u0005\\u0006\\u0007\",\"underSixteen\":\"\\u0008\\t\\n\\u000b\\u000c\\r\\u000e\\u000f\",\"underTwentyFour\":\"\\u0010\\u0011\\u0012\\u0013\\u0014\\u0015\\u0016\\u0017\",\"underThirtyTwo\":\"\\u0018\\u0019\\u001a\\u001b\\u001c\\u001d\\u001e\\u001f\",\"quoted\":\"And then I said, \\\"Defend yourself, blackguard!\\\"\",\"regex\":\"/\\\\s+$//g\",\"utf-eight\":\"Pchnąć w tę łódź jeża i ośm skrzyń fig.\"}"
	);

	testcase(
		stringyObject, NULL, 1,
		"{\n"
		"\t\"empty\": \"\",\n"
		"\t\"underEight\": \"\\u0001\\u0002\\u0003\\u0004\\u0005\\u0006\\u0007\",\n"
		"\t\"underSixteen\": \"\\u0008\\t\\n\\u000b\\u000c\\r\\u000e\\u000f\",\n"
		"\t\"underTwentyFour\": \"\\u0010\\u0011\\u0012\\u0013\\u0014\\u0015\\u0016\\u0017\",\n"
		"\t\"underThirtyTwo\": \"\\u0018\\u0019\\u001a\\u001b\\u001c\\u001d\\u001e\\u001f\",\n"
		"\t\"quoted\": \"And then I said, \\\"Defend yourself, blackguard!\\\"\",\n"
		"\t\"regex\": \"/\\\\s+$//g\",\n"
		"\t\"utf-eight\": \"Pchnąć w tę łódź jeża i ośm skrzyń fig.\"\n"
		"}"
	);
}

void test__jsonNestedObjects(void **state) {
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
}

void test__jsonArrays(void **state) {
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
