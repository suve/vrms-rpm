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
#include <stdio.h>
#include <string.h>

#include "src/memory.h"
#include "src/queries.h"
#include "src/stringutils.h"

#define LINE_BUF_SIZE    (16 * 1024)
#define LICENCE_BUF_SIZE LINE_BUF_SIZE

struct BinaryQuery {
	struct RPMQuery interface;
	FILE *file;
	char *lineBuf;
	char *licenceBuf;
	int withSummary;
};

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


static int getNextRow(struct RPMQuery *query, struct QueryRow *result) {
	struct BinaryQuery *self = (struct BinaryQuery*)query;

	const int expected = (self->withSummary) ? 8 : 7;
	char* fields[8];

	int readSuccessful = 0;
	while(fgets(self->lineBuf, LINE_BUF_SIZE, self->file) != NULL) {
		replace_unicode_spaces(self->lineBuf);
		str_squeeze_char(self->lineBuf, ' ');
		if(str_split(self->lineBuf, '\t', fields, expected) != expected) continue;

		readSuccessful = 1;
		break;
	}
	if(!readSuccessful) return 0;

	char *name    = trim(fields[0], NULL);
	char *epoch   = trim(fields[1], NULL);
	char *version = trim(fields[2], NULL);
	char *release = trim(fields[3], NULL);
	char *arch    = trim(fields[4], NULL);
	char *pubkeys = trim(fields[5], NULL);
	char *licence = trim(fields[6], NULL);
	char *summary = trim(fields[7], NULL);

	// FIXME: This function can fail, should handle that somehow
	str_balance_parentheses(licence, self->licenceBuf, LICENCE_BUF_SIZE, NULL);

	// Epoch is typically undefined. RPM reports this using the special string "(none)".
	// Avoid storing unnecessary epoch info by comparing epoch with this special string.
	// In some very rare cases (hello, "gpg-pubkey" packages!), this can also happen to Arch.
	epoch = is_defined(epoch) ? epoch : NULL;
	arch = is_defined(arch) ? arch : NULL;

	// Put values into result struct
	memset(result, 0, sizeof(struct QueryRow));
	result->name = name;
	result->arch = arch;
	result->summary = summary;
	result->licence = self->licenceBuf;
	result->epoch = epoch;
	result->version = version;
	result->release = release;
	result->isPubkey = is_pubkey_package(name, arch, pubkeys, licence);

	return 1;
}

static void freeQuery(struct RPMQuery *query) {
	struct BinaryQuery *self = (struct BinaryQuery*)query;
	fclose(self->file);
	mem_free(self->lineBuf);
	mem_free(self->licenceBuf);
	mem_free(self);
}

struct RPMQuery* query_newBinary(struct Options *opts, struct Pipe *pipe) {
	struct BinaryQuery *self = mem_alloc(sizeof(struct BinaryQuery));
	self->lineBuf = mem_alloc(LINE_BUF_SIZE);
	self->licenceBuf = mem_alloc(LICENCE_BUF_SIZE);

	self->file = pipe_fopen(pipe);
	self->withSummary = opts->describe;

	self->interface.next = &getNextRow;
	self->interface.free = &freeQuery;
	return &self->interface;
}
