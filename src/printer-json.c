/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2025 suve (a.k.a. Artur Frenszek-Iwicki)
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

#include "src/json.h"
#include "src/licences.h"
#include "src/memory.h"
#include "src/options.h"
#include "src/packages.h"
#include "src/printers.h"

struct JsonPrinter {
	struct Printer interface;

	FILE *file;
	int pretty;
};

static void licenceNodeCallback(struct JsonObject *obj, void *node);

static void licenceNodeMembersCallback(struct JsonArray *arr, void *node) {
	const struct LicenceTreeNode *ltn = node;

	for(unsigned int m = 0; m < ltn->members; ++m) {
		jsonArr_pushObj(arr, &licenceNodeCallback, ltn->child[m]);
	}
}

static void licenceNodeCallback(struct JsonObject *obj, void *node) {
	const struct LicenceTreeNode *ltn = node;

	const char* type[] = {
		[LTNT_LICENCE] = "licence",
		[LTNT_AND] = "and",
		[LTNT_OR] = "or",
	};

	jsonObj_pushBool(obj, "is-free", ltn->is_free);
	jsonObj_pushStr(obj, "type", type[ltn->type]);
	
	if(ltn->type == LTNT_LICENCE) {
		jsonObj_pushStr(obj, "text", ltn->licence);
	} else {
		jsonObj_pushArr(obj, "members", &licenceNodeMembersCallback, node);
	}
}

static void packageCallback(struct JsonObject *obj, void *userdata) {
	const struct PackageListItem *item = userdata;
	const struct Package *pkg = item->package;

	jsonObj_pushStr(obj, "name", pkg->name);

	if(item->duplicated) {
		if(pkg->epoch != NULL) jsonObj_pushStr(obj, "epoch", pkg->epoch);
		jsonObj_pushStr(obj, "version", pkg->version);
		jsonObj_pushStr(obj, "release", pkg->release);
		if(pkg->arch != NULL) jsonObj_pushStr(obj, "architecture", pkg->arch);
	}

	if(opt_describe) jsonObj_pushStr(obj, "summary", pkg->summary);
	if(opt_explain) jsonObj_pushObj(obj, "licence", &licenceNodeCallback, pkg->licence);
}

static void listCallback(struct JsonArray *arr, void *userdata) {
	struct PackageListIterator *iter = userdata;

	struct PackageListItem item;
	while(pkgIter_next(iter, &item)) {
		jsonArr_pushObj(arr, &packageCallback, &item);
	}
}

static void list(struct JsonObject *obj, const char *key, struct PackageData *pd, int free) {
	struct PackageListIterator *iter = pkgIter_new(pd, free);
	jsonObj_pushArr(obj, key, &listCallback, iter);
	pkgIter_free(iter);
}

static void countsCallback(struct JsonObject *obj, void *userdata) {
	struct PackageData *pd = userdata;

	jsonObj_pushInt(obj, "free", pd->count[1]);
	jsonObj_pushInt(obj, "non-free", pd->count[0]);
}

static void topLevelCallback(struct JsonObject *obj, void *userdata) {
	struct PackageData *pd = userdata;

	// This represents the JSON schema version, not the program version!
	jsonObj_pushInt(obj, "version", 0);

	jsonObj_pushObj(obj, "count", &countsCallback, userdata);
	if(opt_list & OPT_LIST_FREE) list(obj, "free", pd, 1);
	if(opt_list & OPT_LIST_NONFREE) list(obj, "non-free", pd, 0);
}

void jsonPrinter_print(
	struct Printer *self,
	struct PackageData *pd
) {
	struct JsonPrinter *printer = (void*)self;
	json_new(printer->file, printer->pretty, &topLevelCallback, pd);
}

void jsonPrinter_free(struct Printer *self) {
	mem_free((struct JsonPrinter*)self);
}

struct Printer* printer_newJSON(FILE *f, const int pretty) {
	struct JsonPrinter *printer = mem_alloc(sizeof(struct JsonPrinter));
	printer->file = f;
	printer->pretty = pretty;

	printer->interface.print = &jsonPrinter_print;
	printer->interface.free = &jsonPrinter_free;
	return &(printer->interface);
}
