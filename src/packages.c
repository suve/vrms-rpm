/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2021 Artur "suve" Iwicki
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

static int split_line(char *line, char ***fields, int max_fields) {
	*(fields[0]) = line;
	for(int i = 1; i < max_fields; ++i) *(fields[i]) = NULL;

	char *search_pos = line;

	int count = 1;
	while(count < max_fields) {
		char* tab = strchr(search_pos, '\t');
		if(tab == NULL) break;

		*tab = '\0';
		*(fields[count]) = search_pos = (tab + 1);
		count += 1;
	}
	
	return count;
}

extern int packages_read(struct Pipe *pipe) {
	if(init_buffers() != 0) return -1;
	sorted = 0;

	FILE *f = pipe_fopen(pipe);
	if(f == NULL) return -1;

	char *name, *licence, *summary;
	char *epoch, *version, *release, *arch;

	const int expected = opt_describe ? 7 : 6;
	char **fields[] = {
		&name, &epoch, &version, &release, &arch, &licence, &summary
	};

	char line[512];
	while(fgets(line, sizeof(line), f) != NULL) {
		replace_unicode_spaces(line);
		if(split_line(line, fields, expected) != expected) continue;

		name = chainbuf_append(&buffer, name);
		licence = chainbuf_append(&buffer, trim(licence, NULL));
		if(opt_describe) summary = chainbuf_append(&buffer, trim(summary, NULL));

		// Epoch is typically undefined. RPM reports this using the special string "(none)".
		// Avoid storing unnecessary epoch info by comparing epoch with this special string.
		if(strcmp(epoch, "(none)") != 0) {
			epoch = chainbuf_append(&buffer, epoch);
		} else {
			epoch = NULL;
		}

		version = chainbuf_append(&buffer, version);
		release = chainbuf_append(&buffer, release);
		arch = chainbuf_append(&buffer, arch);

		struct LicenceTreeNode *classification = licence_classify(licence);
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
		if(rebuf_append(list, &pkg, sizeof(struct Package)) == NULL) return -1;
	}
	
	return LIST_COUNT;
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

	const char* pairs[] = {
		a->epoch, b->epoch,
		a->version, b->version,
		a->release, b->release,
		a->arch, b->arch,
	};
	for(unsigned int p = 0; p < sizeof(pairs) / sizeof(pairs[0]) / 2; ++p) {
		const char *pair_a = pairs[p*2];
		const char *pair_b = pairs[p*2 + 1];

		// If the value is undefined for both packages, skip comparing it.
		// If it's defined only for one package, assume that any_value > no_value.
		if(pair_a == NULL) {
			if(pair_b == NULL) {
				continue;
			}
			return -1;
		} else {
			if(pair_b == NULL) {
				return +1;
			}
		}

		// Temporary logic before we switch to using librpm
		int compare_pair = strcasecmp(pair_a, pair_b);
		if(compare_pair) return compare_pair;
	}
	return 0;
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
	
	const char *const joiner = (node->type == LTNT_AND) ? " and " : " or ";
	for(int m = 0; m < node->members;) {
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

static void format_evr(char *evrbuf, const size_t bufsize, const struct Package *pkg) {
	if(pkg->epoch != NULL)
		snprintf(evrbuf, bufsize, "-%s:%s-%s.%s", pkg->epoch, pkg->version, pkg->release, pkg->arch);
	else
		snprintf(evrbuf, bufsize, "-%s-%s.%s", pkg->version, pkg->release, pkg->arch);
}

static void printlist(const int which_kind) {
	int duplicate_next = 0;

	const int count = LIST_COUNT;
	for(int i = 0; i < count; ++i) {
		struct Package *pkg = &LIST_ITEM(i);
		if(pkg->licence->is_free != which_kind) continue;

		/*
		 * Compare with next package to determine whether this is a duplicate.
		 * Since we sorted the packages by name earlier, duplicates are guaranteed
		 * to be located next to each other in the list.
		 *
		 * If this is not a duplicate of the next package, check whether the previous
		 * package set the "next package is a duplicate" flag.
		 */
		int duplicate_this;
		if((i != (count-1)) && (strcasecmp(pkg->name, LIST_ITEM(i+1).name) == 0)) {
			duplicate_next = duplicate_this = 1;
		} else {
			duplicate_this = duplicate_next;
			duplicate_next = 0;
		}

		// If this is a duplicate, print also the epoch-version-release numbers
		// to reduce ambiguity as to which package we're describing.
		char evr[128] = {'\0'};
		if(duplicate_this) format_evr(evr, sizeof(evr), pkg);

		if(opt_describe) {
			printf(" - %s%s: %s", pkg->name, evr, pkg->summary);
		} else {
			printf(" - %s%s", pkg->name, evr);
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
