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
	int colour;
	int describe;
	int evra;
	int explain;
	int list;
	const char *textAnd;
	const char *textOr;
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

static void printLicenceNode(struct TextPrinter *self, const struct LicenceTreeNode *node) {
	if(node->type == LTNT_LICENCE) {
		if(self->colour)
			fprintf(self->file, "%s%s" ANSI_RESET, node->is_free ? ANSI_GREEN : ANSI_RED, node->licence);
		else
			fprintf(self->file, "%s", node->licence);

		return;
	}

	const char *const joiner =
		(node->type == LTNT_AND) ? self->textAnd : self->textOr;

	for(unsigned int m = 0; m < node->members;) {
		if(node->child[m]->type != LTNT_LICENCE) {
			putc('(', self->file);
			printLicenceNode(self, node->child[m]);
			putc(')', self->file);
		} else {
			printLicenceNode(self, node->child[m]);
		}

		++m;
		if(m < node->members) fprintf(self->file, " %s ", joiner);
	}
}

static void printList(struct TextPrinter *self, struct PackageData *pd, int free) {
	struct PackageListIteratorSettings settings = (struct PackageListIteratorSettings){
		.evra = self->evra,
		.free = free
	};
	struct PackageListIterator *iter = pkgIter_new(pd, settings);

	struct PackageListItem item;
	while(pkgIter_next(iter, &item)) {
		fprintf(self->file, " - %s", item.package->name);
		if(item.duplicated) printEvra(self->file, item.package);
		if(self->describe) fprintf(self->file, ": %s", item.package->summary);

		if(self->explain) {
			fprintf(self->file, "\n   ");
			printLicenceNode(self, item.package->licence);
		}
		putc('\n', self->file);
	}

	pkgIter_free(iter);
}

static void textPrinter_print(
	struct Printer *self,
	struct PackageData *pd
) {
	struct TextPrinter *printer = (void*)self;

	const size_t count_nonfree = pd->count[0];
	const size_t count_free = pd->count[1];

	int promille_nonfree = (1000L * count_nonfree) / (count_nonfree + count_free);
	int promille_free = 1000 - promille_nonfree;
	
	char percent_nonfree[16], percent_free[16];
	snprintf(percent_nonfree, sizeof(percent_nonfree), "%d.%d%%", promille_nonfree / 10, promille_nonfree % 10);
	snprintf(percent_free, sizeof(percent_nonfree), "%d.%d%%", promille_free / 10, promille_free % 10);

	lang_fprint_n(printer->file, MSG_FREE_PACKAGES_COUNT, count_free, count_free, percent_free);
	if(printer->list & OPT_LIST_FREE) printList(printer, pd, 1);
	
	lang_fprint_n(printer->file, MSG_NONFREE_PACKAGES_COUNT, count_nonfree, count_nonfree, percent_nonfree);
	if(printer->list & OPT_LIST_NONFREE) printList(printer, pd, 0);
}

static void textPrinter_free(struct Printer *self) {
	mem_free(self);
}

struct Printer* printer_newText(struct PrinterSettings settings) {
	struct TextPrinter *printer = mem_alloc(sizeof(struct TextPrinter));
	*printer = (struct TextPrinter){
		.interface = (struct Printer){
			.print = &textPrinter_print,
			.free = &textPrinter_free,	
		},
		.colour = settings.colour,
		.describe = settings.describe,
		.evra = settings.evra,
		.explain = settings.explain,
		.file = settings.file,
		.list = settings.list,
		.textAnd = settings.textAnd,
		.textOr = settings.textOr,
	};
	return &(printer->interface);
}
