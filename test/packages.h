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
#ifndef TEST_PACKAGES_H
#define TEST_PACKAGES_H

#include "src/packages.h"

extern void add_package(
	struct PackageData *pd,
	const char *name,
	const char *version,
	const char *summary,
	int is_free,
	const char *licence
);

extern void test__packageIter(void **state);

extern int test_setup__packages(void **state);
extern int test_teardown__packages(void **state);

#endif
