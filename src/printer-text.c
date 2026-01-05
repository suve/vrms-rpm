/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018-2026 suve (a.k.a. Artur Frenszek-Iwicki)
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

struct TextPrinter {
	struct Printer interface;

	FILE *file;
};

static void printEvra(FILE *file, const struct Package *pkg) {
	fprintf(
		file,
		"-%s%s%s-%s%s%s",
		(pkg->epoch != NULL) ? pkg->epoch : "",
		(pkg->epoch != NULL) ? ":" : "",
		pkg->version,
		pkg->release,
		(pkg->arch != NULL) ? "." : "",
		(pkg->arch != NULL) ? pkg->arch : ""
	);
}

static void printLicenceNode(FILE *file, const struct LicenceTreeNode *node) {
	if(node->type == LTNT_LICENCE) {
		if(opt_colour)
			fprintf(file, "%s%s" ANSI_RESET, node->is_free ? ANSI_GREEN : ANSI_RED, node->licence);
		else
			fprintf(file, "%s", node->licence);

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
			putc('(', file);
			printLicenceNode(file, node->child[m]);
			putc(')', file);
		} else {
			printLicenceNode(file, node->child[m]);
		}

		++m;
		if(m < node->members) fprintf(file, "%s", joiner);
	}
}

static void printList(FILE *file, struct PackageData *pd, int free) {
	struct PackageListIterator *iter = pkgIter_new(pd, free);

	struct PackageListItem item;
	while(pkgIter_next(iter, &item)) {
		fprintf(file, " - %s", item.package->name);
		if(item.duplicated) printEvra(file, item.package);
		if(opt_describe) fprintf(file, ": %s", item.package->summary);

		if(opt_explain) {
			fprintf(file, "\n   ");
			printLicenceNode(file, item.package->licence);
		}
		putc('\n', file);
	}

	pkgIter_free(iter);
}

void textPrinter_print(
	struct Printer *self,
	struct PackageData *pd
) {
	FILE *file = ((struct TextPrinter*)self)->file;

	const size_t count_nonfree = pd->count[0];
	const size_t count_free = pd->count[1];

	int promille_nonfree = (1000L * count_nonfree) / (count_nonfree + count_free);
	int promille_free = 1000 - promille_nonfree;
	
	char percent_nonfree[16], percent_free[16];
	snprintf(percent_nonfree, sizeof(percent_nonfree), "%d.%d%%", promille_nonfree / 10, promille_nonfree % 10);
	snprintf(percent_free, sizeof(percent_nonfree), "%d.%d%%", promille_free / 10, promille_free % 10);

	lang_fprint_n(file, MSG_FREE_PACKAGES_COUNT, count_free, count_free, percent_free);
	if(opt_list & OPT_LIST_FREE) printList(file, pd, 1);
	
	lang_fprint_n(file, MSG_NONFREE_PACKAGES_COUNT, count_nonfree, count_nonfree, percent_nonfree);
	if(opt_list & OPT_LIST_NONFREE) printList(file, pd, 0);
}

void textPrinter_free(struct Printer *self) {
	mem_free(self);
}

struct Printer* printer_newText(FILE *f) {
	struct TextPrinter *printer = mem_alloc(sizeof(struct TextPrinter));
	printer->file = f;

	printer->interface.print = &textPrinter_print;
	printer->interface.free = &textPrinter_free;
	return &(printer->interface);
}
