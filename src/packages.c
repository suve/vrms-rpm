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

static int is_defined(const char *value) {
	return strcmp(value, "(none)") != 0;
}

/*
 * "gpg-pubkey-XXXXXXXX-YYYYYYYY" packages are "fake" packages
 * in which RPM stores imported GPG keys. These are a special case
 * and should be treated as such.
 */
static int is_pubkey_package(const char *name, const char *arch, const char *pubkeys, const char *licence) {
	// Check arch first before engaging in expensive strcmp() calls.
	return
		(arch == NULL) &&
		(strcmp(pubkeys, "1") == 0) &&
		(strcmp(name, "gpg-pubkey") == 0) &&
		(strcmp(licence, "pubkey") == 0);
}

struct PackageData* packages_read(
	struct Pipe *pipe,
	struct LicenceClassifier *classifier,
	const struct Options *opts
) {
	struct PackageData *pd = NULL;
	char *line = NULL;
	char *licenceBuffer = NULL;
	FILE *f = NULL;

	pd = packages_alloc();

	/*
	 * TODO: It would be preferable to use a growable buffer here,
	 *       instead of hard-coding the size. That would most likely
	 *       entail switching from fgets(3) to read(2) and performing
	 *       end-of-line detection ourselves.
	 *
	 * The current buffer size is based on Fedora's "fex-emu-rootfs-fedora"
	 * package, which contains an absolute monster of a licence string,
	 * totaling at 12490 characters. 16 KiB will be enough for now
	 * and should provide a bit of margin for the future.
	 */
	#define LINEBUF_SIZE (16 * 1024)
	line = mem_alloc(LINEBUF_SIZE);

	#define LICBUF_SIZE LINEBUF_SIZE
	licenceBuffer = mem_alloc(LICBUF_SIZE);

	f = pipe_fopen(pipe);
	if(f == NULL) goto fail;

	const int expected = (opts->describe) ? 8 : 7;
	char* fields[8];

	while(fgets(line, LINEBUF_SIZE, f) != NULL) {
		replace_unicode_spaces(line);
		str_squeeze_char(line, ' ');
		if(str_split(line, '\t', fields, expected) != expected) continue;

		char *name    = fields[0];
		char *epoch   = fields[1];
		char *version = fields[2];
		char *release = fields[3];
		char *arch    = fields[4];
		char *pubkeys = fields[5];
		char *licence = fields[6];
		char *summary = fields[7];

		// FIXME: This function can fail, should handle that somehow
		str_balance_parentheses(trim(licence, NULL), licenceBuffer, LICBUF_SIZE, NULL);
		licence = chainbuf_append(&pd->buffer, licenceBuffer);

		name = chainbuf_append(&pd->buffer, trim(name, NULL));
		if(opts->describe) summary = chainbuf_append(&pd->buffer, trim(summary, NULL));

		// Epoch is typically undefined. RPM reports this using the special string "(none)".
		// Avoid storing unnecessary epoch info by comparing epoch with this special string.
		// In some very rare cases (hello, "gpg-pubkey" packages!), this can also happen to Arch.
		epoch = is_defined(epoch) ? chainbuf_append(&pd->buffer, epoch) : NULL;
		arch = is_defined(arch) ? chainbuf_append(&pd->buffer, arch) : NULL;

		version = chainbuf_append(&pd->buffer, version);
		release = chainbuf_append(&pd->buffer, release);

		const int is_pubkey = is_pubkey_package(name, arch, pubkeys, licence);
		struct LicenceTreeNode *classification = is_pubkey
				? ((struct LicenceTreeNode*)(&PubkeyLicence))
				: classifier->classify(classifier, licence);
		pd->count[classification->is_free] += 1;

		struct Package pkg = {
			.name = name,
			.summary = summary,
			.epoch = epoch,
			.version = version,
			.release = release,
			.arch = arch,
			.licence = classification,
			.is_pubkey = is_pubkey,
		};
		if(rebuf_append(pd->list, &pkg, sizeof(struct Package)) == NULL) goto fail;
	}

	fclose(f);
	mem_free(licenceBuffer);
	mem_free(line);
	return pd;

	fail: { // As seen in CVE-2014-1266!
		if(f != NULL) fclose(f);
		if(licenceBuffer != NULL) mem_free(licenceBuffer);
		if(line != NULL) mem_free(line);
		if(pd != NULL) packages_free(pd);
		return NULL;
	}
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
