/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2021-2023 suve (a.k.a. Artur Frenszek-Iwicki)
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
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "buffers.h"
#include "lang.h"
#include "licences.h"
#include "options.h"
#include "packages.h"
#include "pipes.h"
#include "stringutils.h"
#include "versions.h"

struct Pipe* packages_openPipe(void) {
	char *queryformat;
	if(!opt_describe)
		queryformat = "%{NAME}\\t%{EPOCH}\\t%{VERSION}\\t%{RELEASE}\\t%{ARCH}\\t%{LICENSE}\\n";
	else
		queryformat = "%{NAME}\\t%{EPOCH}\\t%{VERSION}\\t%{RELEASE}\\t%{ARCH}\\t%{LICENSE}\\t%{SUMMARY}\\n";

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


struct Package {
	char *name, *summary;
	char *epoch, *release, *version, *arch;
	struct LicenceTreeNode *licence;
};

#define LIST_COUNT      (list->used / sizeof(struct Package))
#define LIST_ITEM(idx)  ( ((struct Package*)list->data)[(idx)] )
static struct ReBuffer *list = NULL;
static struct ChainBuffer *buffer = NULL;

static int class_count[2] = {0, 0};
static int sorted = 0;


static int init_buffers(void) {
	if(list == NULL) {
		list = rebuf_init();
		if(list == NULL) return -1;
	}
	
	if(buffer == NULL) {
		buffer = chainbuf_init();
		if(buffer == NULL) return -1;
	}
	
	return 0;
}

static int is_defined(const char *value) {
	return strcmp(value, "(none)") != 0;
}

extern int packages_read(struct Pipe *pipe, struct LicenceClassifier *classifier) {
	char *line = NULL;
	FILE *f = NULL;

	#define LINEBUF_SIZE 4096
	line = malloc(LINEBUF_SIZE);
	if(line == NULL) goto fail;
	if(init_buffers() != 0) goto fail;

	f = pipe_fopen(pipe);
	if(f == NULL) goto fail;

	const int expected = opt_describe ? 7 : 6;
	char* fields[7];

	while(fgets(line, LINEBUF_SIZE, f) != NULL) {
		replace_unicode_spaces(line);
		if(str_split(line, '\t', fields, expected) != expected) continue;

		char *name    = fields[0];
		char *epoch   = fields[1];
		char *version = fields[2];
		char *release = fields[3];
		char *arch    = fields[4];
		char *licence = fields[5];
		char *summary = fields[6];

		name = chainbuf_append(&buffer, name);
		licence = chainbuf_append(&buffer, trim(licence, NULL));
		if(opt_describe) summary = chainbuf_append(&buffer, trim(summary, NULL));

		// Epoch is typically undefined. RPM reports this using the special string "(none)".
		// Avoid storing unnecessary epoch info by comparing epoch with this special string.
		// In some very rare cases (hello, "gpg-pubkey" packages!), this can also happen to Arch.
		epoch = is_defined(epoch) ? chainbuf_append(&buffer, epoch) : NULL;
		arch = is_defined(arch) ? chainbuf_append(&buffer, arch) : NULL;

		version = chainbuf_append(&buffer, version);
		release = chainbuf_append(&buffer, release);

		struct LicenceTreeNode *classification = classifier->classify(classifier, licence);
		class_count[classification->is_free] += 1;
		
		struct Package pkg = {
			.name = name,
			.summary = summary,
			.epoch = epoch,
			.version = version,
			.release = release,
			.arch = arch,
			.licence = classification,
		};
		if(rebuf_append(list, &pkg, sizeof(struct Package)) == NULL) goto fail;
	}

	fclose(f);
	free(line);

	sorted = 0;
	return LIST_COUNT;

	fail: { // As seen in CVE-2014-1266!
		if(f != NULL) fclose(f);
		if(line != NULL) free(line);
		packages_free();
		return -1;
	}
}

void packages_free(void) {
	if(list != NULL) {
		const int count = LIST_COUNT;
		for(int i = 0; i < count; ++i) {
			struct Package *pkg = &LIST_ITEM(i);
			licence_freeTree(pkg->licence);
		}
		
		rebuf_free(list);
		list = NULL;
	}
	
	if(buffer != NULL) {
		chainbuf_free(buffer);
		buffer = NULL;
	}
	
	class_count[0] = class_count[1] = 0;
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

static void packages_sort(void) {
	qsort(list->data, LIST_COUNT, sizeof(struct Package), &pkgcompare);
	sorted = 1;
}

static void printnode(struct LicenceTreeNode *node) {
	if(node->type == LTNT_LICENCE) {
		if(opt_colour)
			printf("%s%s" ANSI_RESET, node->is_free ? ANSI_GREEN : ANSI_RED, node->licence);
		else
			printf("%s", node->licence);
			
		return;
	}

	// SPDX mandates that AND/OR operators should be matched in a case-sensitive manner.
	// Of course, we also support lenient mode, which allows for "and" (also "And" etc.)...
	// Maybe the joiner strings should also be stored in nodes?
	const char *const joiner = (opt_grammar != OPT_GRAMMAR_LOOSE)
		? ((node->type == LTNT_AND) ? " AND " : " OR ")
		: ((node->type == LTNT_AND) ? " and " : " or ");

	for(unsigned int m = 0; m < node->members;) {
		if(node->child[m]->type != LTNT_LICENCE) {
			putc('(', stdout);
			printnode(node->child[m]);
			putc(')', stdout);
		} else {
			printnode(node->child[m]);
		}
		
		++m;
		if(m < node->members) printf("%s", joiner);
	}
}

static void format_evra(char *bufptr, const size_t bufsize, const struct Package *pkg) {
	snprintf(
		bufptr, bufsize, "-%s%s%s-%s%s%s",
		(pkg->epoch != NULL) ? pkg->epoch : "",
		(pkg->epoch != NULL) ? ":" : "",
		pkg->version,
		pkg->release,
		(pkg->arch != NULL) ? "." : "",
		(pkg->arch != NULL) ? pkg->arch : ""
	);
}

static void printlist(const int which_kind) {
	int duplicate_next = 0;

	const int count = LIST_COUNT;
	for(int i = 0; i < count; ++i) {
		struct Package *pkg = &LIST_ITEM(i);
		if(pkg->licence->is_free != which_kind) continue;

		/*
		 * A buffer for formatting the epoch:version-release.arch information,
		 * to eliminate ambiguity as to which package we're describing.
		 *
		 * Subject to the --evra option:
		 * - when set to 'auto', print E:V-R.A only for duplicate packages
		 * - when set to 'always', print for all packages
		 * - when set to 'never', well, don't print it
		 */
		char evra[128] = {'\0'};
		if(opt_evra == OPT_EVRA_AUTO) {
			/*
			 * Compare with next package to determine whether this is a duplicate.
			 * Since we sorted the packages by name earlier, duplicates are guaranteed
			 * to be located next to each other in the list.
			 *
			 * If this is not a duplicate of the next package, check whether
			 * the previous package set the "next package is a duplicate" flag.
			 */
			int duplicate_this;
			if((i != (count-1)) && (strcasecmp(pkg->name, LIST_ITEM(i+1).name) == 0)) {
				duplicate_next = duplicate_this = 1;
			} else {
				duplicate_this = duplicate_next;
				duplicate_next = 0;
			}

			if(duplicate_this) format_evra(evra, sizeof(evra), pkg);
		} else if(opt_evra == OPT_EVRA_ALWAYS) {
			format_evra(evra, sizeof(evra), pkg);
		}

		if(opt_describe) {
			printf(" - %s%s: %s", pkg->name, evra, pkg->summary);
		} else {
			printf(" - %s%s", pkg->name, evra);
		}
		
		if(opt_explain) {
			printf("\n   ");
			printnode(pkg->licence);
		}
		putc('\n', stdout);
	}
}

void packages_list(void) {
	if(!sorted) packages_sort();
	
	int promil_nonfree = (1000L * class_count[0]) / (class_count[0] + class_count[1]);
	int promil_free = 1000 - promil_nonfree;
	
	char percent_nonfree[16], percent_free[16];
	snprintf(percent_nonfree, sizeof(percent_nonfree), "%d.%d%%", promil_nonfree / 10, promil_nonfree % 10);
	snprintf(percent_free, sizeof(percent_nonfree), "%d.%d%%", promil_free / 10, promil_free % 10);
	
	lang_print_n(MSG_FREE_PACKAGES_COUNT, class_count[1], class_count[1], percent_free);
	if(opt_list & OPT_LIST_FREE) printlist(1);
	
	lang_print_n(MSG_NONFREE_PACKAGES_COUNT, class_count[0], class_count[0], percent_nonfree);
	if(opt_list & OPT_LIST_NONFREE) printlist(0);
}

void packages_getcount(int *free, int *nonfree) {
	if(free != NULL) *free = class_count[1];
	if(nonfree != NULL) *nonfree = class_count[0];
}
