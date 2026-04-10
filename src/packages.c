/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2021-2023, 2025-2026 suve (a.k.a. Artur Frenszek-Iwicki)
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
#include <string.h>
#include <strings.h>

#include "src/buffers.h"
#include "src/json.h"
#include "src/lang.h"
#include "src/licences.h"
#include "src/memory.h"
#include "src/options.h"
#include "src/packages.h"
#include "src/pipes.h"
#include "src/queries.h"
#include "src/stringutils.h"
#include "src/versions.h"

#define QUERY_BASE \
	"%{NAME}\\t%{EPOCH}\\t%{VERSION}\\t%{RELEASE}\\t%{ARCH}" \
	"\\t%|PUBKEYS?{1}:{0}|" \
	"\\t%{LICENSE}" \

struct Pipe* packages_openPipe(const struct Options *opts) {
	char *queryformat;
	if(!opts->describe)
		queryformat = QUERY_BASE "\\n";
	else
		queryformat = QUERY_BASE "\\t%{SUMMARY}\\n";

	char *args[] = {
		"/usr/bin/rpm",
		"--all",
		"--query",
		"--queryformat",
		queryformat,
		(char*)NULL
	};

	return pipe_create(args);
}

#define LIST_COUNT(var)      ((var)->list->used / sizeof(struct Package))
#define LIST_ITEM(var, idx)  ( ((struct Package*)(var)->list->data)[(idx)] )

static struct PackageData* packages_alloc(void) {
	struct PackageData *pd = mem_alloc(sizeof(struct PackageData));
	pd->list = rebuf_init(1024 * sizeof(void*));
	pd->buffer = chainbuf_init(16256);
	pd->count[0] = 0;
	pd->count[1] = 0;
	pd->sorted = 0;
	return pd;
}

struct PackageData* packages_read(
	struct RPMQuery *query,
	struct LicenceClassifier *classifier
) {
	struct PackageData *pd = packages_alloc();

	struct QueryRow row;
	while(query->next(query, &row)) {
		char *name = chainbuf_append(&pd->buffer, row.name);
		char *version = chainbuf_append(&pd->buffer, row.version);
		char *release = chainbuf_append(&pd->buffer, row.release);
		
		char *arch = (row.arch != NULL) ? chainbuf_append(&pd->buffer, row.arch) : NULL;
		char *summary = (row.summary != NULL) ? chainbuf_append(&pd->buffer, row.summary) : NULL;
		char *epoch = (row.epoch != NULL ) ? chainbuf_append(&pd->buffer, row.epoch) : NULL;

		char *licenceText = chainbuf_append(&pd->buffer, row.licence);
		struct LicenceTreeNode *licenceTree = row.isPubkey
			? ((struct LicenceTreeNode*)(&PubkeyLicence))
			: classifier->classify(classifier, licenceText);

		struct Package pkg = {
			.name = name,
			.summary = summary,
			.epoch = epoch,
			.version = version,
			.release = release,
			.arch = arch,
			.licence = licenceTree,
			.is_pubkey = row.isPubkey,
		};
		if(rebuf_append(pd->list, &pkg, sizeof(struct Package)) == NULL) {
			packages_free(pd);
			return NULL;
		}
		
		pd->count[licenceTree->is_free] += 1;
	}

	return pd;
}

void packages_free(struct PackageData *pd) {
	if(pd == NULL) return;

	const size_t count = LIST_COUNT(pd);
	for(size_t i = 0; i < count; ++i) {
		struct Package *pkg = &LIST_ITEM(pd, i);
		if(!pkg->is_pubkey) licence_freeTree(pkg->licence);
	}
	
	rebuf_free(pd->list);
	chainbuf_free(pd->buffer);
	mem_free(pd);
}

static int pkgcompare(const void *A, const void *B) {
	const struct Package *a = A;
	const struct Package *b = B;
	
	int compare_names = strcasecmp(a->name, b->name);
	if(compare_names) return compare_names;

	// Compare the Epoch, Version, and Release tags of the packages,
	// using the fancy librpm algorithm (or our fallback).
	const char* pairs[] = {
		a->epoch, b->epoch,
		a->version, b->version,
		a->release, b->release,
	};
	for(unsigned int p = 0; p < sizeof(pairs) / sizeof(pairs[0]) / 2; ++p) {
		const char *pair_a = pairs[p*2];
		const char *pair_b = pairs[p*2 + 1];
		
		int compare_pair = compare_versions(pair_a, pair_b);
		if(compare_pair) return compare_pair;
	}

	// If EVRs are deemed to be equal, resort to comparing Arch.
	return str_compare_with_null_check(a->arch, b->arch, &strcmp);
}

static void packages_sort(struct PackageData *pd) {
	if(pd->sorted) return;

	qsort(pd->list->data, LIST_COUNT(pd), sizeof(struct Package), &pkgcompare);
	pd->sorted = 1;
}

enum DuplicateCheckStatus {
	// Can be duplicate of [i-1] or [i+1]
	DCS_UNKNOWN,
	// Not a duplicate of [i-1], but can be of [i+1]
	DCS_POSSIBLY,
	// Is a duplicate of [i-1]
	DCS_DEFINITELY
};

struct PackageListIterator {
	struct PackageData *pd;
	struct PackageListIteratorSettings settings;
	enum DuplicateCheckStatus next_dup;
	size_t next_index;
};

struct PackageListIterator *pkgIter_new(
	struct PackageData *pd,
	struct PackageListIteratorSettings settings
) {
	struct PackageListIterator *iter = mem_alloc(sizeof(struct PackageListIterator));
	iter->settings = settings;
	iter->settings.free = !!iter->settings.free;

	iter->next_index = 0;
	iter->next_dup = DCS_POSSIBLY;

	packages_sort(pd);
	iter->pd = pd;

	return iter;
}

/*
 * Decide whether to print the epoch:version-release.arch information,
 * to eliminate ambiguity as to which package we're describing.
 *
 * Subject to the --evra option:
 * - when set to 'always', print for all packages
 * - when set to 'auto', print E:V-R.A only for duplicate packages
 * - when set to 'never', well, don't print it
 *
 * An exception to the rules above are "gpg-pubkey-XXXXXXXX-YYYYYYYY" packages.
 * For all of them, the name is just "gpg-pubkey"; the XXXXXXXX and YYYYYYYY parts
 * are stored inside the Version and Release fields.
 *
 * For example, the package holding the Fedora 38 signing key looks like this:
 * - Name: "gpg-pubkey"
 * - Version: "eb10b464"
 * - Release: "6202d9c6"
 *
 * Since printing just "gpg-pubkey" is rather unhelpful, we want to ALWAYS
 * print EVRA information for these packages, even if the user specified "--evra never".
 */
static int should_print_evra(
	struct PackageListIterator *iter,
	const size_t i,
	const size_t count
) {
	if(iter->settings.evra == OPT_EVRA_ALWAYS) {
		return 1;
	}

	struct Package *this = &LIST_ITEM(iter->pd, i);
	if(this->is_pubkey) {
		iter->next_dup = DCS_UNKNOWN;
		return 1;
	}

	if(iter->settings.evra == OPT_EVRA_NEVER) {
		return 0;
	}

	/*
	 * If control reaches this point, then:
	 * - settings.evra must be OPT_EVRA_AUTO
	 * - package is not pubkey
	 * Perform the duplicate check logic.
	 */

	// Bail out early if "next package is duplicate" flag is set.
	if(iter->next_dup == DCS_DEFINITELY) {
		iter->next_dup = DCS_UNKNOWN;
		return 1;
	}

	/*
	 * Compare with next package to determine whether this is a duplicate.
	 * Since we sorted the packages by name earlier, duplicates are guaranteed
	 * to be located next to each other in the list.
	 */
	if(i != count-1) {
		struct Package *next = &LIST_ITEM(iter->pd, i+1);
		if(strcasecmp(this->name, next->name) == 0) {
			iter->next_dup = DCS_DEFINITELY;
			return 1;
		}
	}

	// If the previous package did not perform the comparison, do it now.
	int duplicated = 0;
	if((iter->next_dup == DCS_UNKNOWN) && (i > 0)) {
		struct Package *previous = &LIST_ITEM(iter->pd, i-1);
		duplicated = (strcasecmp(this->name, previous->name) == 0);
	}

	iter->next_dup = DCS_POSSIBLY;
	return duplicated;
}

int pkgIter_next(struct PackageListIterator *iter, struct PackageListItem *item) {
	const size_t count = LIST_COUNT(iter->pd);
	if(iter->next_index >= count) return 0;

	struct Package *pkg;
	size_t current_index = iter->next_index;
	while(1) {
		pkg = &LIST_ITEM(iter->pd, current_index);
		if(pkg->licence->is_free == iter->settings.free) break;

		current_index += 1;
		if(current_index >= count) {
			iter->next_index = count;
			return 0;
		}

		iter->next_dup = DCS_UNKNOWN;
	}

	item->package = pkg;
	item->duplicated = should_print_evra(iter, current_index, count);

	iter->next_index = current_index + 1;
	return 1;
}

void pkgIter_free(struct PackageListIterator *iter) {
	mem_free(iter);
}
