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
#ifndef TEST_PRINTERS_H

#include <stdio.h>

#include "src/options.h"
#include "src/packages.h"
#include "src/printers.h"

#define PRINTER_TEST_BUFFER_SIZE (8 * 1024)

struct PrinterTestState {
	struct PackageData *pkgs;
	char *buffer;
};

extern int test_setup__printers(void **state);
extern int test_teardown__printers(void **state);

#define printerTestCase(settings, constructor, expected) do { \
	struct PrinterTestState *ts = *state; \
	FILE *bf = fmemopen(ts->buffer, PRINTER_TEST_BUFFER_SIZE, "w"); \
	(settings).file = bf; \
\
	struct Printer *printer = (constructor)(settings); \
	printer->print(printer, ts->pkgs); \
	printer->free(printer); \
\
	putc('\0', bf); \
	fflush(bf); \
	fclose(bf); \
\
	assert_string_equal(ts->buffer, (expected)); \
} while(0)

extern void test__textPrinter_none(void **state);
extern void test__textPrinter_basic(void **state);
extern void test__textPrinter_everything(void **state);
extern void test__jsonPrinter_none(void **state);
extern void test__jsonPrinter_basic(void **state);
extern void test__jsonPrinter_everything(void **state);

#endif
