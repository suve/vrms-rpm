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
#include "src/licences.h"
#include "src/options.h"
#include "test/packages.h"
#include "test/test.h"

void add_package(
	struct PackageData *pd,
	const char *name,
	const char *version,
	const char *summary,
	int is_free,
	const char *licence
) {
	is_free = !!is_free;

	struct LicenceTreeNode *ltn = malloc(sizeof(struct LicenceTreeNode));
	ltn->type = LTNT_LICENCE;
	ltn->is_free = is_free;
	ltn->licence = licence;

	struct Package pkg = (struct Package){
		.name = name,
		.summary = summary,
		.epoch = NULL,
		.version = version,
		.release = "1",
		.arch = "noarch",
		.licence = ltn,
		.is_pubkey = 0
	};
	rebuf_append(pd->list, &pkg, sizeof(struct Package));
	pd->count[is_free] += 1;
}

void test__packageIter(void **state) {
	struct PackageData *pd = *state;	
	struct PackageListItem item;

	// TODO: Global vars stink donkey ass
	opt_evra = OPT_EVRA_AUTO;

	// Non-Free iter
	{
		struct PackageListIterator *iter = pkgIter_new(pd, 0);

		assert_true(pkgIter_next(iter, &item));
		assert_string_equal(item.package->name, "relicensed");
		assert_string_equal(item.package->version, "1.0");
		assert_true(item.duplicated);

		assert_true(pkgIter_next(iter, &item));
		assert_string_equal(item.package->name, "relicensed");
		assert_string_equal(item.package->version, "2.0");
		assert_true(item.duplicated);

		assert_true(pkgIter_next(iter, &item));
		assert_string_equal(item.package->name, "relinquished");
		assert_false(item.duplicated);

		assert_false(pkgIter_next(iter, &item));

		pkgIter_free(iter);
	}
	// Free iter
	{
		struct PackageListIterator *iter = pkgIter_new(pd, 1);

		assert_true(pkgIter_next(iter, &item));
		assert_string_equal(item.package->name, "relicensed");
		assert_string_equal(item.package->version, "3.0");
		assert_true(item.duplicated);

		assert_false(pkgIter_next(iter, &item));

		pkgIter_free(iter);
	}
}

int test_setup__packages(void **state) {
	struct PackageData *pd = malloc(sizeof(struct PackageData));	
	pd->list = rebuf_init(256);
	pd->count[0] = pd->count[1] = 0;

	add_package(pd, "relicensed", "1.0", "", 0, "Absolutely proprietary");
	add_package(pd, "relicensed", "2.0", "", 0, "Still proprietary");
	add_package(pd, "relicensed", "3.0", "", 1, "Free for all");
	add_package(pd, "relinquished", "4.2", "", 0, "Freeware");

	pd->buffer = NULL;
	pd->sorted = 0;

	*state = pd;
	return 0;
}

int test_teardown__packages(void **state) {
	struct PackageData *pd = *state;
	packages_free(pd);
	return 0;
}
