/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2025-2026 suve (a.k.a. Artur Frenszek-Iwicki)
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
#ifndef VRMS_RPM_PRINTERS_H
#define VRMS_RPM_PRINTERS_H

#include <stdio.h>

#include "src/packages.h"

struct PrinterSettings {
	FILE *file;
	// TODO: Add separate enums for each option
	int colour;
	int describe;
	int evra;
	int explain;
	int list;
	int pretty;
	const char *textAnd;
	const char *textOr;
};

struct Printer {
	void (*print)(
		struct Printer *self,
		struct PackageData *pd
	);

	void (*free)(struct Printer *self);
};

extern struct Printer* printer_newText(struct PrinterSettings settings);
extern struct Printer* printer_newJSON(struct PrinterSettings settings);

#endif
