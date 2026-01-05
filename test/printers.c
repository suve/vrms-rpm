/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2026 suve (a.k.a. Artur Frenszek-Iwicki)
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
#include <stdlib.h>

#include "src/buffers.h"
#include "src/lang.h"
#include "src/options.h"
#include "src/packages.h"
#include "src/printers.h"
#include "src/licences.h"
#include "test/test.h"

#define BUFSIZE (8 * 1024)

struct TestState {
	struct PackageData *pkgs;
	char *buffer;
};

static void add_package(
	struct PackageData *pd,
	const char *name,
	const char *summary,
	int is_free,
	const char *licence
) {
	struct LicenceTreeNode *ltn = malloc(sizeof(struct LicenceTreeNode));
	ltn->type = LTNT_LICENCE;
	ltn->is_free = is_free;
	ltn->licence = licence;

	struct Package pkg = (struct Package){
		.name = name,
		.summary = summary,
		.epoch = NULL,
		.version = "1.0.0",
		.release = "1",
		.arch = "noarch",
		.licence = ltn,
		.is_pubkey = 0
	};
	rebuf_append(pd->list, &pkg, sizeof(struct Package));
	pd->count[is_free] += 1;
}

static struct PackageData* setup_package_data(void) {
	struct PackageData *pd = malloc(sizeof(struct PackageData));	
	pd->list = rebuf_init(256);
	pd->count[0] = pd->count[1] = 0;

	// NOTE: This list should be sorted by name.
	add_package(pd, "evil", "Package full of nasty stuff", 0, "All rights reserved");
	add_package(pd, "first", "Come and served", 1, "GPL-2.0-or-later");
	add_package(pd, "second", "Too bad, so sad", 1, "MPL-2.0");
	add_package(pd, "third", "Still on the podium", 1, "BSD-3-Clause");

	pd->buffer = NULL;
	pd->sorted = 1;

	return pd;
}

int test_setup__printers(void **state) {
	struct TestState *ts = malloc(sizeof(struct TestState));
	ts->pkgs = setup_package_data();
	ts->buffer = malloc(BUFSIZE);

	// TODO: This sucks donkey ass. Testing would be easier if printers were
	//       configured via constructor arguments, instead of grabbing values
	//       from global options whenever they feel like.
	opt_colour = 0;
	opt_describe = 1;
	opt_evra = OPT_EVRA_ALWAYS;
	opt_explain = 1;
	opt_list = OPT_LIST_FREE | OPT_LIST_NONFREE;

	*state = ts;
	return 0;
}

int test_teardown__printers(void **state) {
	struct TestState *ts = *state;
	packages_free(ts->pkgs);
	free(ts->buffer);
	return 0;
}

#define testcase(format, expected) do { \
	struct TestState *ts = *state; \
	FILE *bf = fmemopen(ts->buffer, BUFSIZE, "w"); \
\
	struct Printer *printer; \
	if(format == OPT_FORMAT_TEXT) { \
		printer = printer_newText(bf); \
	} else { \
		printer = printer_newJSON(bf, (format == OPT_FORMAT_PRETTYJSON)); \
	} \
	printer->print(printer, ts->pkgs); \
	printer->free(printer); \
\
	putc('\0', bf); \
	fflush(bf); \
	fclose(bf); \
\
	assert_string_equal(ts->buffer, (expected)); \
} while(0)

// FIXME: The text printer needs access to localized strings.
void test__textPrinter(void **state) {
	testcase(
		OPT_FORMAT_TEXT,
		"FREE_PACKAGES_COUNT\n"
		" - first-1.0.0-1.noarch: Come and served\n"
		"   GPL-2.0-or-later\n"
		" - second-1.0.0-1.noarch: Too bad, so sad\n"
		"   MPL-2.0\n"
		" - third-1.0.0-1.noarch: Still on the podium\n"
		"   BSD-3-Clause\n"
		"NONFREE_PACKAGES_COUNT\n"
		" - evil-1.0.0-1.noarch: Package full of nasty stuff\n"
		"   All rights reserved\n"
	);
}

void test__jsonPrinter(void **state) {
	testcase(
		OPT_FORMAT_PRETTYJSON,
		"{\n"
		"\t\"version\": 0,\n"
		"\t\"count\": {\n"
		"\t\t\"free\": 3,\n"
		"\t\t\"non-free\": 1\n"
		"\t},\n"
		"\t\"free\": [\n"
		"\t\t{\n"
		"\t\t\t\"name\": \"first\",\n"
		"\t\t\t\"version\": \"1.0.0\",\n"
		"\t\t\t\"release\": \"1\",\n"
		"\t\t\t\"architecture\": \"noarch\",\n"
		"\t\t\t\"summary\": \"Come and served\",\n"
		"\t\t\t\"licence\": {\n"
		"\t\t\t\t\"is-free\": true,\n"
		"\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\"text\": \"GPL-2.0-or-later\"\n"
		"\t\t\t}\n"
		"\t\t},\n"
		"\t\t{\n"
		"\t\t\t\"name\": \"second\",\n"
		"\t\t\t\"version\": \"1.0.0\",\n"
		"\t\t\t\"release\": \"1\",\n"
		"\t\t\t\"architecture\": \"noarch\",\n"
		"\t\t\t\"summary\": \"Too bad, so sad\",\n"
		"\t\t\t\"licence\": {\n"
		"\t\t\t\t\"is-free\": true,\n"
		"\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\"text\": \"MPL-2.0\"\n"
		"\t\t\t}\n"
		"\t\t},\n"
		"\t\t{\n"
		"\t\t\t\"name\": \"third\",\n"
		"\t\t\t\"version\": \"1.0.0\",\n"
		"\t\t\t\"release\": \"1\",\n"
		"\t\t\t\"architecture\": \"noarch\",\n"
		"\t\t\t\"summary\": \"Still on the podium\",\n"
		"\t\t\t\"licence\": {\n"
		"\t\t\t\t\"is-free\": true,\n"
		"\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\"text\": \"BSD-3-Clause\"\n"
		"\t\t\t}\n"
		"\t\t}\n"
		"\t],\n"
		"\t\"non-free\": [\n"
		"\t\t{\n"
		"\t\t\t\"name\": \"evil\",\n"
		"\t\t\t\"version\": \"1.0.0\",\n"
		"\t\t\t\"release\": \"1\",\n"
		"\t\t\t\"architecture\": \"noarch\",\n"
		"\t\t\t\"summary\": \"Package full of nasty stuff\",\n"
		"\t\t\t\"licence\": {\n"
		"\t\t\t\t\"is-free\": false,\n"
		"\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\"text\": \"All rights reserved\"\n"
		"\t\t\t}\n"
		"\t\t}\n"
		"\t]\n"
		"}\n"
	);
}
