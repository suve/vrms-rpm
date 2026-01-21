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
#include "src/stringutils.h"

#include "test/printers.h"
#include "test/test.h"

// (!) FIXME: The text printer needs access to localized strings.

void test__textPrinter_none(void **state) {
	struct PrinterSettings settings = (struct PrinterSettings) {
		.colour = 0,
		.describe = 0,
		.evra = OPT_EVRA_NEVER,
		.explain = 0,
		.list = 0,
		.pretty = 0,
		.textAnd = "AND",
		.textOr = "OR",
	};

	printerTestCase(
		settings,
		printer_newText,
		pkgs_simple,
		"FREE_PACKAGES_COUNT\n"
		"NONFREE_PACKAGES_COUNT\n"
	);
}

void test__textPrinter_basic(void **state) {
	struct PrinterSettings settings = (struct PrinterSettings) {
		.colour = 0,
		.describe = 0,
		.evra = OPT_EVRA_NEVER,
		.explain = 0,
		.list = (OPT_LIST_FREE | OPT_LIST_NONFREE),
		.pretty = 0,
		.textAnd = "AND",
		.textOr = "OR",
	};

	printerTestCase(
		settings,
		printer_newText,
		pkgs_simple,
		"FREE_PACKAGES_COUNT\n"
		" - first\n"
		" - second\n"
		" - third\n"
		"NONFREE_PACKAGES_COUNT\n"
		" - evil\n"
	);
}

void test__textPrinter_everything(void **state) {
	struct PrinterSettings settings = (struct PrinterSettings) {
		.colour = 1,
		.describe = 1,
		.evra = OPT_EVRA_ALWAYS,
		.explain = 1,
		.list = (OPT_LIST_FREE | OPT_LIST_NONFREE),
		.pretty = 0,
		.textAnd = "AND",
		.textOr = "OR",
	};

	printerTestCase(
		settings,
		printer_newText,
		pkgs_simple,
		"FREE_PACKAGES_COUNT\n"
		" - first-1.0.0-1.noarch: Come and served\n"
		"   " ANSI_GREEN "GPL-2.0-or-later" ANSI_RESET "\n"
		" - second-2.4.8-1.noarch: Too bad, so sad\n"
		"   " ANSI_GREEN "MPL-2.0" ANSI_RESET "\n"
		" - third-3.0.3-1.noarch: Still on the podium\n"
		"   " ANSI_GREEN "BSD-3-Clause" ANSI_RESET "\n"
		"NONFREE_PACKAGES_COUNT\n"
		" - evil-6.6.6-1.noarch: Package full of nasty stuff\n"
		"   " ANSI_RED "All rights reserved" ANSI_RESET "\n"
	);
}

void test__textPrinter_complex(void **state) {
	struct PrinterSettings settings = (struct PrinterSettings) {
		.colour = 1,
		.describe = 1,
		.evra = OPT_EVRA_AUTO,
		.explain = 1,
		.list = (OPT_LIST_FREE | OPT_LIST_NONFREE),
		.pretty = 0,
		.textAnd = "ANDrzej",
		.textOr = "ORpheus",
	};

	printerTestCase(
		settings,
		printer_newText,
		pkgs_complex,
		"FREE_PACKAGES_COUNT\n"
		" - pabog: Pas all the bogs\n"
		"   " ANSI_GREEN "Permissive" ANSI_RESET " ANDrzej (" ANSI_RED "Bad" ANSI_RESET " ORpheus " ANSI_GREEN "Good" ANSI_RESET ")\n"
		"NONFREE_PACKAGES_COUNT\n"
		" - bagor: Bags all the ors\n"
		"   (" ANSI_RED "Bad" ANSI_RESET " ANDrzej " ANSI_GREEN "Good" ANSI_RESET ") ORpheus " ANSI_RED "Restrictive" ANSI_RESET "\n"
	);
}
