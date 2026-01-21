/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018-2023, 2025-2026 suve (a.k.a. Artur Frenszek-Iwicki)
 * Copyright (C) 2020 Jan Drögehoff
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
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "src/config.h"
#include "src/lang.h"
#include "src/options.h"

static void print_help(void);


#define OPT_COLOUR_NEVER  0
#define OPT_COLOUR_ALWAYS 1
#define OPT_COLOUR_AUTO   2

static struct Options options_default(void) {
	return (struct Options) {
		.colour = OPT_COLOUR_AUTO,
		.describe = 0,
		.evra = OPT_EVRA_AUTO,
		.grammar = DEFAULT_GRAMMAR_ENUM,
		.explain = 0,
		.format = OPT_FORMAT_TEXT,
		.image = OPT_IMAGE_NONE,
		.list = OPT_LIST_NONFREE,
		.licenceList = DEFAULT_LICENCE_LIST,
	};
}


#define ARG_NON no_argument
#define ARG_OPT optional_argument
#define ARG_REQ required_argument

enum LongOpt {
	LONGOPT_HELP = 1,
	LONGOPT_COLOUR,
	LONGOPT_DESCRIBE,
	LONGOPT_EVRA,
	LONGOPT_EXPLAIN,
	LONGOPT_FORMAT,
	LONGOPT_GRAMMAR,
	LONGOPT_IMAGE_ASCII,
	LONGOPT_IMAGE_ICAT,
	LONGOPT_LICENCELIST,
	LONGOPT_LIST,
	LONGOPT_VERSION
};

static void parseopt_colour(struct Options *opts);
static void parseopt_evra(struct Options *opts);
static void parseopt_format(struct Options *opts);
static void parseopt_grammar(struct Options *opts);
static void parseopt_list(struct Options *opts);

struct Options options_parse(int argc, char **argv) {
	struct Options opts = options_default();

	const struct option vrms_opts[] = {
		{       "ascii", ARG_NON, NULL, LONGOPT_IMAGE_ASCII },
		{       "color", ARG_REQ, NULL, LONGOPT_COLOUR },
		{      "colour", ARG_REQ, NULL, LONGOPT_COLOUR },
		{    "describe", ARG_NON, NULL, LONGOPT_DESCRIBE },
		{        "evra", ARG_REQ, NULL, LONGOPT_EVRA },
		{     "explain", ARG_NON, NULL, LONGOPT_EXPLAIN },
		{      "format", ARG_REQ, NULL, LONGOPT_FORMAT },
		{     "grammar", ARG_REQ, NULL, LONGOPT_GRAMMAR },
		{        "help", ARG_NON, NULL, LONGOPT_HELP },
		{       "image", ARG_NON, NULL, LONGOPT_IMAGE_ICAT },
		{"licence-list", ARG_REQ, NULL, LONGOPT_LICENCELIST },
		{"license-list", ARG_REQ, NULL, LONGOPT_LICENCELIST },
		{        "list", ARG_REQ, NULL, LONGOPT_LIST },
		{     "version", ARG_NON, NULL, LONGOPT_VERSION },
		{ 0, 0, 0, 0 },
	};

	opterr = 0;
	while(1) {
		int option_index = 0;
		
		int res = getopt_long(argc, argv, ":", vrms_opts, &option_index);
		if(res == -1) break;

		switch (res) {
			case LONGOPT_HELP:
				print_help();
				exit(EXIT_SUCCESS);
			
			case LONGOPT_COLOUR:
				parseopt_colour(&opts);
			break;

			case LONGOPT_DESCRIBE:
				opts.describe = 1;
			break;

			case LONGOPT_EVRA:
				parseopt_evra(&opts);
			break;

			case LONGOPT_EXPLAIN:
				opts.explain = 1;
			break;

			case LONGOPT_FORMAT:
				parseopt_format(&opts);
			break;

			case LONGOPT_GRAMMAR:
				parseopt_grammar(&opts);
			break;

			case LONGOPT_IMAGE_ASCII:
				opts.image = OPT_IMAGE_ASCII;
			break;

			case LONGOPT_IMAGE_ICAT:
				opts.image = OPT_IMAGE_ICAT;
			break;

			case LONGOPT_LICENCELIST:
				opts.licenceList = optarg;
			break;
			
			case LONGOPT_LIST:
				parseopt_list(&opts);
			break;
			
			case LONGOPT_VERSION:
				puts("vrms-rpm v2.3 by suve");
				
				const char *translator = lang_getmsg(MSG_TRANSLATION_AUTHOR);
				if(strcmp(translator, "--\n") != 0) printf("%s", translator);
				
				exit(EXIT_SUCCESS);
			
			case ':':
				lang_fprint(stderr, MSG_ERR_BADOPT_NOARG, argv[option_index]);
				exit(EXIT_FAILURE);
			
			case '?':
				lang_fprint(stderr, MSG_ERR_BADOPT_UNKNOWN, argv[option_index]);
				exit(EXIT_FAILURE);
		}
	}
	
	if(opts.colour == OPT_COLOUR_AUTO) {
		if (getenv("NO_COLOR") != NULL) {
			opts.colour = OPT_COLOUR_NEVER;
		} else {
			opts.colour = isatty(fileno(stdout));
		}
	}

	return opts;
}

#define arg_eq(str)  (strcmp((str), optarg) == 0)

// In the help text, we state that the only allowed values are "auto", "always" and "never".
// However, previous versions of the program used "yes" instead of "always", and "no" instead of "never".
// Keep support for these in the name of backwards-compatibility.
static void parseopt_colour(struct Options *opts) {
	if(arg_eq("auto")) {
		opts->colour = OPT_COLOUR_AUTO;
	} else if(arg_eq("never") || (arg_eq("no"))) {
		opts->colour = OPT_COLOUR_NEVER;
	} else if(arg_eq("always") || arg_eq("yes")) {
		opts->colour = OPT_COLOUR_ALWAYS;
	} else {
		lang_fprint(stderr, MSG_ERR_BADOPT_COLOUR);
		exit(EXIT_FAILURE);
	}
}

static void parseopt_evra(struct Options *opts) {
	if(arg_eq("auto")) {
		opts->evra = OPT_EVRA_AUTO;
	} else if(arg_eq("never")) {
		opts->evra = OPT_EVRA_NEVER;
	} else if(arg_eq("always")) {
		opts->evra = OPT_EVRA_ALWAYS;
	} else {
		lang_fprint(stderr, MSG_ERR_BADOPT_EVRA);
		exit(EXIT_FAILURE);
	}
}

static void parseopt_grammar(struct Options *opts) {
	if(arg_eq("loose")) {
		opts->grammar = OPT_GRAMMAR_LOOSE;
	} else if(arg_eq("spdx-strict")) {
		opts->grammar = OPT_GRAMMAR_SPDX_STRICT;
	} else if(arg_eq("spdx-lenient")) {
		opts->grammar = OPT_GRAMMAR_SPDX_LENIENT;
	} else {
		lang_fprint(stderr, MSG_ERR_BADOPT_GRAMMAR);
		exit(EXIT_FAILURE);
	}
}

static void parseopt_format(struct Options *opts) {
	if(arg_eq("text")) {
		opts->format = OPT_FORMAT_TEXT;
	} else if(arg_eq("json")) {
		opts->format = OPT_FORMAT_JSON;
	} else if(arg_eq("json-pretty") || arg_eq("pretty-json")) {
		opts->format = OPT_FORMAT_JSON_PRETTY;
	} else {
		lang_fprint(stderr, MSG_ERR_BADOPT_FORMAT);
		exit(EXIT_FAILURE);
	}
}

static void parseopt_list(struct Options *opts) {
	if(arg_eq("all")) {
		opts->list = OPT_LIST_FREE | OPT_LIST_NONFREE;
	} else if(arg_eq("free")) {
		opts->list = OPT_LIST_FREE;
	} else if(arg_eq("nonfree") || arg_eq("non-free")) {
		opts->list = OPT_LIST_NONFREE;
	} else if(arg_eq("none")) {
		opts->list = 0;
	} else {
		lang_fprint(stderr, MSG_ERR_BADOPT_LIST);
		exit(EXIT_FAILURE);
	}
}

static void print_help(void) {
	lang_print(MSG_HELP_USAGE);
	
	puts("  --ascii");
	lang_print(MSG_HELP_OPTION_ASCII);
	
	puts("  --colour <auto, never, always>");
	lang_print(MSG_HELP_OPTION_COLOUR);
	
	puts("  --describe");
	lang_print(MSG_HELP_OPTION_DESCRIBE);
	
	puts("  --evra <auto, never, always>");
	lang_print(MSG_HELP_OPTION_EVRA);

	puts("  --explain");
	lang_print(MSG_HELP_OPTION_EXPLAIN);

	puts("  --format <text, json, json-pretty>");
	lang_print(MSG_HELP_OPTION_FORMAT);

	puts("  --grammar <loose, spdx-strict, spdx-lenient>");
	lang_print(MSG_HELP_OPTION_GRAMMAR, DEFAULT_GRAMMAR_NAME);

	puts("  --help");
	lang_print(MSG_HELP_OPTION_HELP);
	
	puts("  --image");
	lang_print(MSG_HELP_OPTION_IMAGE);
	
	puts("  --licence-list <FILE>");
	lang_print(MSG_HELP_OPTION_LICENCELIST, ALL_LICENCE_LISTS, DEFAULT_LICENCE_LIST);
	
	puts("  --list <none, free, nonfree, all>");
	lang_print(MSG_HELP_OPTION_LIST);
	
	puts("  --version");
	lang_print(MSG_HELP_OPTION_VERSION);
}
