/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018-2025 suve (a.k.a. Artur Frenszek-Iwicki)
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

#include "src/lang.h"
#include "src/licences.h"
#include "src/memory.h"
#include "src/options.h"
#include "src/packages.h"
#include "src/printers.h"
#include "src/stringutils.h"

static void printEvra(const struct Package *pkg) {
	printf(
		"-%s%s%s-%s%s%s",
		(pkg->epoch != NULL) ? pkg->epoch : "",
		(pkg->epoch != NULL) ? ":" : "",
		pkg->version,
		pkg->release,
		(pkg->arch != NULL) ? "." : "",
		(pkg->arch != NULL) ? pkg->arch : ""
	);
}

static void printLicenceNode(const struct LicenceTreeNode *node) {
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
			printLicenceNode(node->child[m]);
			putc(')', stdout);
		} else {
			printLicenceNode(node->child[m]);
		}

		++m;
		if(m < node->members) printf("%s", joiner);
	}
}

static void printList(struct PackageListIterator *iter) {
	struct PackageListItem item;
	while(pkgIter_next(iter, &item)) {
		printf(" - %s", item.package->name);
		if(item.duplicated) printEvra(item.package);
		if(opt_describe) printf(": %s", item.package->summary);

		if(opt_explain) {
			printf("\n   ");
			printLicenceNode(item.package->licence);
		}
		putc('\n', stdout);
	}
}

void textPrinter_print(
	struct Printer *self,
	struct PackageListIterator *freeIter,
	struct PackageListIterator *nonfreeIter
) {
	((void)self); // unused

	const size_t count_nonfree = pkgIter_getCount(nonfreeIter);
	const size_t count_free = pkgIter_getCount(freeIter);

	int promille_nonfree = (1000L * count_nonfree) / (count_nonfree + count_free);
	int promille_free = 1000 - promille_nonfree;
	
	char percent_nonfree[16], percent_free[16];
	snprintf(percent_nonfree, sizeof(percent_nonfree), "%d.%d%%", promille_nonfree / 10, promille_nonfree % 10);
	snprintf(percent_free, sizeof(percent_nonfree), "%d.%d%%", promille_free / 10, promille_free % 10);

	lang_print_n(MSG_FREE_PACKAGES_COUNT, count_free, count_free, percent_free);
	if(opt_list & OPT_LIST_FREE) printList(freeIter);
	
	lang_print_n(MSG_NONFREE_PACKAGES_COUNT, count_nonfree, count_nonfree, percent_nonfree);
	if(opt_list & OPT_LIST_NONFREE) printList(nonfreeIter);
}

void textPrinter_free(struct Printer *self) {
	mem_free(self);
}

struct Printer* printer_newText(void) {
	struct Printer *printer = mem_alloc(sizeof(struct Printer));
	printer->print = &textPrinter_print;
	printer->free = &textPrinter_free;
	return printer;
}
