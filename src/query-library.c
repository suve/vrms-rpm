/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2026 suve (a.k.a. Artur Frenszek-Iwicki)
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
#ifdef WITH_LIBRPM

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmlog.h>
#include <rpm/rpmts.h>

#include "src/memory.h"
#include "src/queries.h"

struct LibraryQuery {
	struct RPMQuery interface;
	FILE *devNull;
	rpmts transaction;
	rpmdbMatchIterator iter;
	int withSummary;
};

static int getNextRow(struct RPMQuery *query, struct QueryRow *result) {
	struct LibraryQuery *self = (struct LibraryQuery*)query;

	Header header = rpmdbNextIterator(self->iter);
	if(header == NULL) return 0;

	result->name = headerGetString(header, RPMTAG_NAME);
	result->arch = headerGetString(header, RPMTAG_ARCH);
	result->epoch = headerGetString(header, RPMTAG_EPOCH);
	result->version = headerGetString(header, RPMTAG_VERSION);
	result->release = headerGetString(header, RPMTAG_RELEASE);
	result->licence = headerGetString(header, RPMTAG_LICENSE);

	result->summary = (self->withSummary) ? headerGetString(header, RPMTAG_SUMMARY) : NULL;
	result->isPubkey = headerIsEntry(header, RPMTAG_PUBKEYS);

	return 1;
}

static void freeQuery(struct RPMQuery *query) {
	struct LibraryQuery *self = (struct LibraryQuery*)query;
	
	rpmdbFreeIterator(self->iter);
	rpmtsFree(self->transaction);

	rpmlogSetFile(NULL);
	fclose(self->devNull);

	mem_free(self);
}

struct RPMQuery* query_newLibrary(struct Options *opts) {
	struct LibraryQuery *self = mem_alloc(sizeof(struct LibraryQuery));
	self->withSummary = opts->describe;

	/*
	 * librpm does this annoying thing where, by default, it prints all
	 * errors to stderr. To prevent the library from polluting our output
	 * we need to give it a different file where to write the logs.
	 */
	self->devNull = fopen("/dev/null", "w");
	rpmlogSetFile(self->devNull);

	rpmReadConfigFiles(NULL, NULL);

	// FIXME: creating a transaction set can fail, should handle that
	self->transaction = rpmtsCreate();
	rpmtsSetFlags(self->transaction, rpmtsFlags(self->transaction) | RPMTRANS_FLAG_NOPLUGINS);

	// FIXME: creating an iterator can fail, should handle that
	self->iter = rpmtsInitIterator(self->transaction, RPMDBI_PACKAGES, NULL, 0);

	self->interface.next = &getNextRow;
	self->interface.free = &freeQuery;
	return &self->interface;
}
#endif
