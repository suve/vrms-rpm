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
#include <string.h>

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

static void printpkg(char *name, char *licence) {
	printf("%s: %s\n", name, licence);
	
	struct LicenceTreeNode *tree = licence_classify(licence);
	printnode(tree);
	
	licence_freeTree(tree);
	putc('\n', stdout);
}

int packages_read(struct Pipe *pipe) {
	FILE *f = pipe_fopen(pipe);
	if(f == NULL) return -1;
	
	char buffer[256];
	char *name, *licence;
	
	int count = 0;
	while(fgets(buffer, sizeof(buffer), f) != NULL) {
		char *tab = strchr(buffer, '\t');
		if(!tab) continue;
		
		*tab = '\0';
		name = buffer;
		licence = trim(tab+1, NULL);
		printpkg(name, licence);
		
		++count;
	}
	
	return count;
}
