/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018 Artur "suve" Iwicki
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

#include "buffers.h"
#include "licences.h"
#include "packages.h"
#include "pipes.h"
#include "stringutils.h"

struct Pipe* packages_openPipe(void) {
	#define ARGNUM 5
	char *args[ARGNUM] = {
		"/usr/bin/rpm",
		"--all",
		"--query",
		"--queryformat",
		"%{NAME}\\t%{LICENSE}\\n",
	};
	
	return pipe_create(ARGNUM, args);
}

struct Package {
	char *name;
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

int packages_read(struct Pipe *pipe) {
	if(init_buffers() != 0) return -1;
	sorted = 0;
	
	FILE *f = pipe_fopen(pipe);
	if(f == NULL) return -1;
	
	char line[256];
	char *name, *licence;
	
	while(fgets(line, sizeof(line), f) != NULL) {
		char *tab = strchr(line, '\t');
		if(!tab) continue;
		
		*tab = '\0';
		name = chainbuf_append(&buffer, line);
		
		licence = trim(tab+1, NULL);
		licence = chainbuf_append(&buffer, licence);
		
		struct LicenceTreeNode *classification = licence_classify(licence);
		class_count[classification->is_free] += 1;
		
		struct Package pkg = {
			.name = name,
			.licence = classification
		};
		rebuf_append(list, &pkg, sizeof(struct Package));
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
	
	return strcasecmp(a->name, b->name);
}

static void packages_sort(void) {
	qsort(list->data, LIST_COUNT, sizeof(struct Package), &pkgcompare);
	sorted = 1;
}

static void printnode(struct LicenceTreeNode *node) {
	if(node->type == LTNT_LICENCE) {
		printf("%s%s" ANSI_RESET, node->is_free ? ANSI_GREEN : ANSI_RED, node->licence);
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

void packages_list(void) {
	if(!sorted) packages_sort();
	
	const int count = LIST_COUNT;
	for(int c = 1; c >= 0; --c) {
		printf("%d %s packages\n", class_count[c], c ? "free" : "non-free");
		for(int i = 0; i < count; ++i) {
			struct Package *pkg = &LIST_ITEM(i);
			if(pkg->licence->is_free != c) continue;
			
			printf(" - %s: ", pkg->name);
			printnode(pkg->licence);
			putc('\n', stdout);
		}
	}
}

void packages_getcount(int *free, int *nonfree) {
	if(free != NULL) *free = class_count[1];
	if(nonfree != NULL) *nonfree = class_count[0];
}
