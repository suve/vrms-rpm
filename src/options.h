/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2020-2021, 2023, 2025-2026 suve (a.k.a. Artur Frenszek-Iwicki)
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
#ifndef VRMS_RPM_OPTIONS_H
#define VRMS_RPM_OPTIONS_H

enum OptEvra {
	OPT_EVRA_NEVER = -1,
	OPT_EVRA_AUTO,
	OPT_EVRA_ALWAYS
};

enum OptFormat {
	OPT_FORMAT_TEXT,
	OPT_FORMAT_JSON,
	OPT_FORMAT_JSON_PRETTY
};

enum OptGrammar {
	OPT_GRAMMAR_LOOSE,
	OPT_GRAMMAR_SPDX_STRICT,
	OPT_GRAMMAR_SPDX_LENIENT
};

enum OptImage {
	OPT_IMAGE_NONE,
	OPT_IMAGE_ASCII,
	OPT_IMAGE_ICAT
};

enum OptList {
	OPT_LIST_NONE = 0,
	OPT_LIST_FREE = (1<<0),
	OPT_LIST_NONFREE = (1<<1),
	OPT_LIST_BOTH = (OPT_LIST_FREE | OPT_LIST_NONFREE)
};

struct Options {
	int colour;
	int describe;
	enum OptEvra evra;
	int explain;
	enum OptFormat format;
	enum OptGrammar grammar;
	enum OptImage image;
	enum OptList list;
	const char *licenceList;
};

extern struct Options options_parse(int argc, char **argv);

#endif
