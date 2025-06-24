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
#ifndef VRMS_RPM_JSON_H
#define VRMS_RPM_JSON_H

#include <stdio.h>

/*
 * A very limited and clunky API for outputting JSON.
 *
 * The main design goal here is to, as far as possible,
 * avoid holding data in memory and serialize everything immediately instead.
 */
struct JsonArray;
struct JsonObject;

typedef void (*JsonArrayCallback)(struct JsonArray *arr, void *userdata);
typedef void (*JsonObjectCallback)(struct JsonObject *obj, void *userdata);

extern void json_new(
	FILE *output,
	int pretty,
	JsonObjectCallback callback,
	void *userdata
);

extern void jsonObj_pushBool(
	struct JsonObject *obj,
	const char *const key,
	const int value
);
extern void jsonObj_pushInt(
	struct JsonObject *obj,
	const char *const key,
	const int value
);
extern void jsonObj_pushStr(
	struct JsonObject *obj,
	const char *const key,
	const char *const value
);
extern void jsonObj_pushArr(
	struct JsonObject *obj,
	const char *const key,
	JsonArrayCallback callback,
	void *userdata
);
extern void jsonObj_pushObj(
	struct JsonObject *obj,
	const char *const key,
	JsonObjectCallback callback,
	void *userdata
);

extern void jsonArr_pushStr(
	struct JsonArray *arr,
	const char *const value
);
extern void jsonArr_pushObj(
	struct JsonArray *arr,
	JsonObjectCallback callback,
	void *userdata
);

#endif
