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
#include "test/packages.h"
#include "test/printers.h"
#include "test/test.h"

static struct PackageData* setup_package_data(void) {
	struct PackageData *pd = malloc(sizeof(struct PackageData));	
	pd->list = rebuf_init(256);
	pd->count[0] = pd->count[1] = 0;

	// NOTE: This list should be sorted by name.
	add_package(pd, "evil", "6.6.6", "Package full of nasty stuff", 0, "All rights reserved");
	add_package(pd, "first", "1.0.0", "Come and served", 1, "GPL-2.0-or-later");
	add_package(pd, "second", "2.4.8", "Too bad, so sad", 1, "MPL-2.0");
	add_package(pd, "third", "3.0.3", "Still on the podium", 1, "BSD-3-Clause");

	pd->buffer = NULL;
	pd->sorted = 1;

	return pd;
}

int test_setup__printers(void **state) {
	struct PrinterTestState *ts = malloc(sizeof(struct PrinterTestState));
	ts->pkgs = setup_package_data();
	ts->buffer = malloc(PRINTER_TEST_BUFFER_SIZE);

	*state = ts;
	return 0;
}

int test_teardown__printers(void **state) {
	struct PrinterTestState *ts = *state;
	packages_free(ts->pkgs);
	free(ts->buffer);
	return 0;
}
