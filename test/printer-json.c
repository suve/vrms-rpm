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
#include "src/version.h"
#include "test/printers.h"
#include "test/test.h"

#define SCHEMA_VERSION STRINGIFY_EXPANSION(JSON_SCHEMA_VERSION)

void test__jsonPrinter_none(void **state) {
	struct PrinterSettings settings = (struct PrinterSettings) {
		.colour = 0,
		.describe = 0,
		.evra = OPT_EVRA_NEVER,
		.explain = 0,
		.list = 0,
		.pretty = 0,
		.textAnd = "and",
		.textOr = "or",
	};

	printerTestCase(
		settings,
		printer_newJSON,
		pkgs_simple,
		"{\"version\":{\"schema\":" SCHEMA_VERSION ",\"program\":\"" PROGRAM_VERSION "\"},\"count\":{\"free\":3,\"non-free\":1}}"
	);
}


void test__jsonPrinter_basic(void **state) {
	struct PrinterSettings settings = (struct PrinterSettings) {
		.colour = 0,
		.describe = 0,
		.evra = OPT_EVRA_NEVER,
		.explain = 0,
		.list = (OPT_LIST_FREE | OPT_LIST_NONFREE),
		.pretty = 0,
		.textAnd = "and",
		.textOr = "or",
	};

	printerTestCase(
		settings,
		printer_newJSON,
		pkgs_simple,
		"{"
		"\"version\":{\"schema\":" SCHEMA_VERSION ",\"program\":\"" PROGRAM_VERSION "\"},"
		"\"count\":{\"free\":3,\"non-free\":1},"
		"\"free\":[{\"name\":\"first\"},{\"name\":\"second\"},{\"name\":\"third\"}],"
		"\"non-free\":[{\"name\":\"evil\"}]"
		"}"
	);
}

void test__jsonPrinter_everything(void **state) {
	struct PrinterSettings settings = (struct PrinterSettings) {
		.colour = 0,
		.describe = 1,
		.evra = OPT_EVRA_ALWAYS,
		.explain = 1,
		.list = (OPT_LIST_FREE | OPT_LIST_NONFREE),
		.pretty = 1,
		.textAnd = "and",
		.textOr = "or",
	};

	printerTestCase(
		settings,
		printer_newJSON,
		pkgs_simple,
		"{\n"
		"\t\"version\": {\n"
		"\t\t\"schema\": " SCHEMA_VERSION ",\n"
		"\t\t\"program\": \"" PROGRAM_VERSION "\"\n"
		"\t},\n"
		"\t\"count\": {\n"
		"\t\t\"free\": 3,\n"
		"\t\t\"non-free\": 1\n"
		"\t},\n"
		"\t\"free\": [\n"
		"\t\t{\n"
		"\t\t\t\"name\": \"first\",\n"
		"\t\t\t\"version\": \"1.0.0\",\n"
		"\t\t\t\"release\": \"1\",\n"
		"\t\t\t\"architecture\": \"noarch\",\n"
		"\t\t\t\"summary\": \"Come and served\",\n"
		"\t\t\t\"licence\": {\n"
		"\t\t\t\t\"is-free\": true,\n"
		"\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\"text\": \"GPL-2.0-or-later\"\n"
		"\t\t\t}\n"
		"\t\t},\n"
		"\t\t{\n"
		"\t\t\t\"name\": \"second\",\n"
		"\t\t\t\"version\": \"2.4.8\",\n"
		"\t\t\t\"release\": \"1\",\n"
		"\t\t\t\"architecture\": \"noarch\",\n"
		"\t\t\t\"summary\": \"Too bad, so sad\",\n"
		"\t\t\t\"licence\": {\n"
		"\t\t\t\t\"is-free\": true,\n"
		"\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\"text\": \"MPL-2.0\"\n"
		"\t\t\t}\n"
		"\t\t},\n"
		"\t\t{\n"
		"\t\t\t\"name\": \"third\",\n"
		"\t\t\t\"version\": \"3.0.3\",\n"
		"\t\t\t\"release\": \"1\",\n"
		"\t\t\t\"architecture\": \"noarch\",\n"
		"\t\t\t\"summary\": \"Still on the podium\",\n"
		"\t\t\t\"licence\": {\n"
		"\t\t\t\t\"is-free\": true,\n"
		"\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\"text\": \"BSD-3-Clause\"\n"
		"\t\t\t}\n"
		"\t\t}\n"
		"\t],\n"
		"\t\"non-free\": [\n"
		"\t\t{\n"
		"\t\t\t\"name\": \"evil\",\n"
		"\t\t\t\"version\": \"6.6.6\",\n"
		"\t\t\t\"release\": \"1\",\n"
		"\t\t\t\"architecture\": \"noarch\",\n"
		"\t\t\t\"summary\": \"Package full of nasty stuff\",\n"
		"\t\t\t\"licence\": {\n"
		"\t\t\t\t\"is-free\": false,\n"
		"\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\"text\": \"All rights reserved\"\n"
		"\t\t\t}\n"
		"\t\t}\n"
		"\t]\n"
		"}\n"
	);
}

void test__jsonPrinter_complex(void **state) {
	struct PrinterSettings settings = (struct PrinterSettings) {
		.colour = 0,
		.describe = 0,
		.evra = OPT_EVRA_AUTO,
		.explain = 1,
		.list = (OPT_LIST_FREE | OPT_LIST_NONFREE),
		.pretty = 1,
		.textAnd = "and",
		.textOr = "or",
	};

	printerTestCase(
		settings,
		printer_newJSON,
		pkgs_complex,
		"{\n"
		"\t\"version\": {\n"
		"\t\t\"schema\": " SCHEMA_VERSION ",\n"
		"\t\t\"program\": \"" PROGRAM_VERSION "\"\n"
		"\t},\n"
		"\t\"count\": {\n"
		"\t\t\"free\": 1,\n"
		"\t\t\"non-free\": 1\n"
		"\t},\n"
		"\t\"free\": [\n"
		"\t\t{\n"
		"\t\t\t\"name\": \"pabog\",\n"
		"\t\t\t\"licence\": {\n"
		"\t\t\t\t\"is-free\": true,\n"
		"\t\t\t\t\"type\": \"and\",\n"
		"\t\t\t\t\"members\": [\n"
		"\t\t\t\t\t{\n"
		"\t\t\t\t\t\t\"is-free\": true,\n"
		"\t\t\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\t\t\"text\": \"Permissive\"\n"
		"\t\t\t\t\t},\n"
		"\t\t\t\t\t{\n"
		"\t\t\t\t\t\t\"is-free\": true,\n"
		"\t\t\t\t\t\t\"type\": \"or\",\n"
		"\t\t\t\t\t\t\"members\": [\n"
		"\t\t\t\t\t\t\t{\n"
		"\t\t\t\t\t\t\t\t\"is-free\": false,\n"
		"\t\t\t\t\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\t\t\t\t\"text\": \"Bad\"\n"
		"\t\t\t\t\t\t\t},\n"
		"\t\t\t\t\t\t\t{\n"
		"\t\t\t\t\t\t\t\t\"is-free\": true,\n"
		"\t\t\t\t\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\t\t\t\t\"text\": \"Good\"\n"
		"\t\t\t\t\t\t\t}\n"
		"\t\t\t\t\t\t]\n"
		"\t\t\t\t\t}\n"
		"\t\t\t\t]\n"
		"\t\t\t}\n"
		"\t\t}\n"
		"\t],\n"
		"\t\"non-free\": [\n"
		"\t\t{\n"
		"\t\t\t\"name\": \"bagor\",\n"
		"\t\t\t\"licence\": {\n"
		"\t\t\t\t\"is-free\": false,\n"
		"\t\t\t\t\"type\": \"or\",\n"
		"\t\t\t\t\"members\": [\n"
		"\t\t\t\t\t{\n"
		"\t\t\t\t\t\t\"is-free\": false,\n"
		"\t\t\t\t\t\t\"type\": \"and\",\n"
		"\t\t\t\t\t\t\"members\": [\n"
		"\t\t\t\t\t\t\t{\n"
		"\t\t\t\t\t\t\t\t\"is-free\": false,\n"
		"\t\t\t\t\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\t\t\t\t\"text\": \"Bad\"\n"
		"\t\t\t\t\t\t\t},\n"
		"\t\t\t\t\t\t\t{\n"
		"\t\t\t\t\t\t\t\t\"is-free\": true,\n"
		"\t\t\t\t\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\t\t\t\t\"text\": \"Good\"\n"
		"\t\t\t\t\t\t\t}\n"
		"\t\t\t\t\t\t]\n"
		"\t\t\t\t\t},\n"
		"\t\t\t\t\t{\n"
		"\t\t\t\t\t\t\"is-free\": false,\n"
		"\t\t\t\t\t\t\"type\": \"licence\",\n"
		"\t\t\t\t\t\t\"text\": \"Restrictive\"\n"
		"\t\t\t\t\t}\n"
		"\t\t\t\t]\n"
		"\t\t\t}\n"
		"\t\t}\n"
		"\t]\n"
		"}\n"
	);
}
