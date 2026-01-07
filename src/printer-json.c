/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2025-2026 suve (a.k.a. Artur Frenszek-Iwicki)
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
	int describe;
	int evra;
	int explain;
	int list;
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

// This one struct holds the data for (almost) all callbacks.
// Since the printer runs single-threaded, there are no race conditions.
//
// Using a separate struct for each callback would be cleaner from an
// architecture perspective, but would also result in more boilerplate code
// and copying data around, so eh, let's just do it like this.
struct CallbackData {
	struct JsonPrinter *printer;
	struct PackageData *pkgs;
	struct PackageListIterator *iter;
	struct PackageListItem item;
};

static void packageCallback(struct JsonObject *obj, void *userdata) {
	const struct CallbackData *cbdata = userdata;
	const struct Package *pkg = cbdata->item.package;

	jsonObj_pushStr(obj, "name", pkg->name);

	if(cbdata->item.duplicated) {
		if(pkg->epoch != NULL) jsonObj_pushStr(obj, "epoch", pkg->epoch);
		jsonObj_pushStr(obj, "version", pkg->version);
		jsonObj_pushStr(obj, "release", pkg->release);
		if(pkg->arch != NULL) jsonObj_pushStr(obj, "architecture", pkg->arch);
	}

	if(cbdata->printer->describe) jsonObj_pushStr(obj, "summary", pkg->summary);
	if(cbdata->printer->explain) jsonObj_pushObj(obj, "licence", &licenceNodeCallback, pkg->licence);
}

static void listCallback(struct JsonArray *arr, void *userdata) {
	struct CallbackData *cbdata = userdata;
	while(pkgIter_next(cbdata->iter, &cbdata->item)) {
		jsonArr_pushObj(arr, &packageCallback, cbdata);
	}
}

static void list(struct JsonObject *obj, const char *key, struct CallbackData *cbdata, int free) {
	struct PackageListIteratorSettings settings = (struct PackageListIteratorSettings){
		.evra = cbdata->printer->evra,
		.free = free
	};
	cbdata->iter = pkgIter_new(cbdata->pkgs, settings);
	jsonObj_pushArr(obj, key, &listCallback, cbdata);
	pkgIter_free(cbdata->iter);
}

static void countsCallback(struct JsonObject *obj, void *userdata) {
	struct PackageData *pd = userdata;

	jsonObj_pushInt(obj, "free", pd->count[1]);
	jsonObj_pushInt(obj, "non-free", pd->count[0]);
}

static void topLevelCallback(struct JsonObject *obj, void *userdata) {
	struct CallbackData *cbdata = userdata;

	// This represents the JSON schema version, not the program version!
	jsonObj_pushInt(obj, "version", 0);

	jsonObj_pushObj(obj, "count", &countsCallback, cbdata->pkgs);
	if(cbdata->printer->list & OPT_LIST_FREE) list(obj, "free", cbdata, 1);
	if(cbdata->printer->list & OPT_LIST_NONFREE) list(obj, "non-free", cbdata, 0);
}

static void jsonPrinter_print(
	struct Printer *self,
	struct PackageData *pd
) {
	struct JsonPrinter *printer = (void*)self;

	struct CallbackData cbdata;
	cbdata.printer = printer;
	cbdata.pkgs = pd;

	json_new(printer->file, printer->pretty, &topLevelCallback, &cbdata);
	if(printer->pretty) putc('\n', printer->file);
}

static void jsonPrinter_free(struct Printer *self) {
	mem_free((struct JsonPrinter*)self);
}

struct Printer* printer_newJSON(struct PrinterSettings settings) {
	struct JsonPrinter *printer = mem_alloc(sizeof(struct JsonPrinter));
	*printer = (struct JsonPrinter){
		.interface = (struct Printer){
			.print = &jsonPrinter_print,
			.free = &jsonPrinter_free,	
		},
		.describe = settings.describe,
		.evra = settings.evra,
		.explain = settings.explain,
		.file = settings.file,
		.list = settings.list,
		.pretty = settings.pretty,
	};
	return &(printer->interface);
}
