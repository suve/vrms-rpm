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
#include "src/packages.h"
#include "test/licences.h"
#include "test/packages.h"
#include "test/printers.h"
#include "test/test.h"

static struct PackageData* setup_simple_packages(void) {
	struct PackageData *pd = malloc(sizeof(struct PackageData));	
	pd->list = rebuf_init(256);
	pd->count[0] = pd->count[1] = 0;

	// NOTE: This list should be sorted by name.
	add_simple_package(pd, "evil", "6.6.6", "Package full of nasty stuff", 0, "All rights reserved");
	add_simple_package(pd, "first", "1.0.0", "Come and served", 1, "GPL-2.0-or-later");
	add_simple_package(pd, "second", "2.4.8", "Too bad, so sad", 1, "MPL-2.0");
	add_simple_package(pd, "third", "3.0.3", "Still on the podium", 1, "BSD-3-Clause");

	pd->buffer = NULL;
	pd->sorted = 1;

	return pd;
}

static struct PackageData* setup_complex_packages(void) {
	struct PackageData *pd = malloc(sizeof(struct PackageData));	
	pd->list = rebuf_init(256);
	pd->count[0] = pd->count[1] = 0;

	{
		struct LicenceTreeNode *left;
		make_ltn_simple(left, 1, "Permissive");

		struct LicenceTreeNode *first, *second, *right;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 1, "Good");
		make_ltn(right, 1, LTNT_OR, first, second);

		struct LicenceTreeNode *combined;
		make_ltn(combined, 1, LTNT_AND, left, right);

		struct Package pkg = (struct Package) {
			.name = "pabog",
			.summary = "Pas all the bogs",
			.epoch = NULL,
			.version = "1.1.0",
			.release = "4",
			.arch = "noarch",
			.licence = combined,
			.is_pubkey = 0
		};
		add_package(pd, &pkg);
	}
	{
		struct LicenceTreeNode *first, *second, *left;
		make_ltn_simple(first, 0, "Bad");
		make_ltn_simple(second, 1, "Good");
		make_ltn(left, 0, LTNT_AND, first, second);

		struct LicenceTreeNode *right;
		make_ltn_simple(right, 0, "Restrictive");

		struct LicenceTreeNode *combined;
		make_ltn(combined, 0, LTNT_OR, left, right);

		struct Package pkg = (struct Package) {
			.name = "bagor",
			.summary = "Bags all the ors",
			.epoch = NULL,
			.version = "0.1.1",
			.release = "4",
			.arch = "noarch",
			.licence = combined,
			.is_pubkey = 0
		};
		add_package(pd, &pkg);
	}

	pd->buffer = NULL;
	pd->sorted = 0;

	return pd;
}

int test_setup__printers(void **state) {
	struct PrinterTestState *ts = malloc(sizeof(struct PrinterTestState));
	ts->pkgs_simple = setup_simple_packages();
	ts->pkgs_complex = setup_complex_packages();
	ts->buffer = malloc(PRINTER_TEST_BUFFER_SIZE);

	*state = ts;
	return 0;
}

int test_teardown__printers(void **state) {
	struct PrinterTestState *ts = *state;
	packages_free(ts->pkgs_simple);
	packages_free(ts->pkgs_complex);
	free(ts->buffer);
	return 0;
}
