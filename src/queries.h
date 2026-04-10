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
#ifndef VRMS_RPM_QUERIES_H
#define VRMS_RPM_QUERIES_H

#include "src/options.h"
#include "src/pipes.h"

struct QueryRow {
	char *name;
	char *summary;
	char *epoch;
	char *version;
	char *release;
	char *arch;
	char *licence;
	int isPubkey;
};

struct RPMQuery {
	int (*next)(struct RPMQuery *self, struct QueryRow *result);
	void (*free)(struct RPMQuery *self);
};

extern struct RPMQuery* query_newBinary(struct Options *opts, struct Pipe *pipe);
// extern struct RPMQuery* query_newLibrary(struct Options *opts);

#endif
