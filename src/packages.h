/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2023, 2025-2026 suve (a.k.a. Artur Frenszek-Iwicki)
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

#ifndef VRMS_RPM_PACKAGES_H
#define VRMS_RPM_PACKAGES_H

#include "src/classifiers.h"
#include "src/pipes.h"

// FIXME: Figure out a way to make this an opaque struct,
//        while also allowing to create a custom struct for testing.
struct PackageData {
	struct ReBuffer *list;
	struct ChainBuffer *buffer;
	size_t count[2];
	int sorted;
};

struct Package {
	const char *name, *summary;
	const char *epoch, *release, *version, *arch;
	struct LicenceTreeNode *licence;
	int is_pubkey;
};

struct PackageListIterator;

struct PackageListItem {
	const struct Package *package;
	int duplicated;
};

extern struct Pipe* packages_openPipe(void);
extern struct PackageData* packages_read(struct Pipe *pipe, struct LicenceClassifier *classifier);
extern void packages_free(struct PackageData *pd);

extern struct PackageListIterator *pkgIter_new(struct PackageData *pd, int free);
extern int pkgIter_next(struct PackageListIterator *iter, struct PackageListItem *item);
extern void pkgIter_free(struct PackageListIterator *iter);

#endif
