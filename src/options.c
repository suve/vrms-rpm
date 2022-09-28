/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018-2022 suve (a.k.a. Artur Frenszek-Iwicki)
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

#include "config.h"
#include "lang.h"
#include "options.h"

static void print_help(void);


#define OPT_COLOUR_NEVER  0
#define OPT_COLOUR_ALWAYS 1
#define OPT_COLOUR_AUTO   2

int opt_colour = OPT_COLOUR_AUTO;
int opt_describe = 0;
int opt_evra = OPT_EVRA_AUTO;
int opt_explain = 0;
int opt_image = OPT_IMAGE_NONE;
int opt_list = OPT_LIST_NONFREE;
char* opt_licencelist = DEFAULT_LICENCE_LIST;


#define ARG_NON no_argument
#define ARG_OPT optional_argument
#define ARG_REQ required_argument

enum LongOpt {
	LONGOPT_HELP = 1,
	LONGOPT_COLOUR,
	LONGOPT_EVRA,
	LONGOPT_LICENCELIST,
	LONGOPT_LIST,
	LONGOPT_VERSION
};

static void parseopt_colour(void);
static void parseopt_evra(void);
static void parseopt_list(void);

void options_parse(int argc, char **argv) {
	const struct option vrms_opts[] = {
		{       "ascii", ARG_NON, &opt_image, OPT_IMAGE_ASCII },
		{       "color", ARG_REQ, NULL, LONGOPT_COLOUR },
		{      "colour", ARG_REQ, NULL, LONGOPT_COLOUR },
		{    "describe", ARG_NON, &opt_describe, 1 },
		{        "evra", ARG_REQ, NULL, LONGOPT_EVRA },
		{     "explain", ARG_NON, &opt_explain, 1 },
		{        "help", ARG_NON, NULL, LONGOPT_HELP },
		{       "image", ARG_NON, &opt_image, OPT_IMAGE_ICAT },
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
				parseopt_colour();
			break;

			case LONGOPT_EVRA:
				parseopt_evra();
			break;
			
			case LONGOPT_LICENCELIST:
				opt_licencelist = optarg;
			break;
			
			case LONGOPT_LIST:
				parseopt_list();
			break;
			
			case LONGOPT_VERSION:
				puts("vrms-rpm v.2.2 by suve");
				
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
	
	if(opt_colour == OPT_COLOUR_AUTO) {
		if (getenv("NO_COLOR") != NULL) {
			opt_colour = OPT_COLOUR_NEVER;
		} else {
			opt_colour = isatty(fileno(stdout));
		}
	}
}

#define arg_eq(str)  (strcmp((str), optarg) == 0)

// In the help text, we state that the only allowed values are "auto", "always" and "never".
// However, previous versions of the program used "yes" instead of "always", and "no" instead of "never".
// Keep support for these in the name of backwards-compatibility.
static void parseopt_colour(void) {
	if(arg_eq("auto")) {
		opt_colour = OPT_COLOUR_AUTO;
	} else if(arg_eq("never") || (arg_eq("no"))) {
		opt_colour = OPT_COLOUR_NEVER;
	} else if(arg_eq("always") || arg_eq("yes")) {
		opt_colour = OPT_COLOUR_ALWAYS;
	} else {
		lang_fprint(stderr, MSG_ERR_BADOPT_COLOUR);
		exit(EXIT_FAILURE);
	}
}

static void parseopt_evra(void) {
	if(arg_eq("auto")) {
		opt_evra = OPT_EVRA_AUTO;
	} else if(arg_eq("never")) {
		opt_evra = OPT_EVRA_NEVER;
	} else if(arg_eq("always")) {
		opt_evra = OPT_EVRA_ALWAYS;
	} else {
		lang_fprint(stderr, MSG_ERR_BADOPT_EVRA);
		exit(EXIT_FAILURE);
	}
}

static void parseopt_list(void) {
	if(arg_eq("all")) {
		opt_list = OPT_LIST_FREE | OPT_LIST_NONFREE;
	} else if(arg_eq("free")) {
		opt_list = OPT_LIST_FREE;
	} else if(arg_eq("nonfree") || arg_eq("non-free")) {
		opt_list = OPT_LIST_NONFREE;
	} else if(arg_eq("none")) {
		opt_list = 0;
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
