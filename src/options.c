/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018 Artur "suve" Iwicki
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

#include "options.h"

static void print_help(void);


#define OPT_COLOUR_NO   0
#define OPT_COLOUR_YES  1
#define OPT_COLOUR_AUTO 2

int opt_ascii = 0;
int opt_colour = OPT_COLOUR_AUTO;
int opt_describe = 0;
int opt_explain = 0;
int opt_list = OPT_LIST_NONFREE;
char* opt_licencelist = DEFAULT_LICENCE_LIST;


#define ARG_NON no_argument
#define ARG_OPT optional_argument
#define ARG_REQ required_argument

enum LongOpt {
	LONGOPT_HELP = 1,
	LONGOPT_COLOUR,
	LONGOPT_LICENCELIST,
	LONGOPT_LIST,
	LONGOPT_VERSION
};


#define arg_eq(str)  (strcmp((str), optarg) == 0)

void parseopt_colour(void);
void parseopt_list(void);

void options_parse(int argc, char **argv) {
	const struct option vrms_opts[] = {
		{       "ascii", ARG_NON, &opt_ascii, 1 },
		{       "color", ARG_REQ, NULL, LONGOPT_COLOUR },
		{      "colour", ARG_REQ, NULL, LONGOPT_COLOUR },
		{    "describe", ARG_NON, &opt_describe, 1 },
		{     "explain", ARG_NON, &opt_explain, 1 },
		{        "help", ARG_NON, NULL, LONGOPT_HELP },
		{"licence-list", ARG_REQ, NULL, LONGOPT_LICENCELIST},
		{        "list", ARG_REQ, NULL, LONGOPT_LIST },
		{     "version", ARG_NON, NULL, LONGOPT_VERSION },
		{ 0, 0, 0, 0 },
	};

	while(1) {
		int option_index = 0;
		
		int res = getopt_long(argc, argv, "", vrms_opts, &option_index);
		if(res == -1) break;

		switch (res) {
			case LONGOPT_HELP:
				print_help();
				exit(EXIT_SUCCESS);
			
			case LONGOPT_COLOUR:
				parseopt_colour();
			break;
			
			case LONGOPT_LICENCELIST:
				opt_licencelist = optarg;
			break;
			
			case LONGOPT_LIST:
				parseopt_list();
			break;
			
			case LONGOPT_VERSION:
				printf("vrms-rpm v.2.0 by suve\n");
				exit(EXIT_SUCCESS);
		}
	}
	
	if(opt_colour == OPT_COLOUR_AUTO) {
		opt_colour = isatty(fileno(stdout));
	}
}

void parseopt_colour(void) {
	if(arg_eq("auto")) {
		opt_colour = OPT_COLOUR_AUTO;
	} else if(arg_eq("no")) {
		opt_colour = OPT_COLOUR_NO;
	} else if(arg_eq("yes")) {
		opt_colour = OPT_COLOUR_YES;
	} else {
		fputs(
			"vrms-rpm: argument to --colour option must be one of: "
			"\"no\", \"yes\", or \"auto\"\n",
			stderr
		);
		exit(EXIT_FAILURE);
	}
}

void parseopt_list(void) {
	if(arg_eq("all")) {
		opt_list = OPT_LIST_FREE | OPT_LIST_NONFREE;
	} else if(arg_eq("free")) {
		opt_list = OPT_LIST_FREE;
	} else if(arg_eq("nonfree") || arg_eq("non-free")) {
		opt_list = OPT_LIST_NONFREE;
	} else if(arg_eq("none")) {
		opt_list = 0;
	} else {
		fputs(
			"vrms-rpm: argument to --list option must be one of: "
			"\"none\", \"non-free\", \"free\", \"all\"\n",
			stderr
		);
		exit(EXIT_FAILURE);
	}
}

static void print_help(void) {
	printf(
		"Usage: vrms-rpm [options]\n"
		"  --ascii\n"
		"    Display rms ASCII-art when no non-free packages are found,\n"
		"    or when non-free packages are 10%% or more of the total.\n"
		"  --colour <no,yes,auto>\n"
		"    Controls whether terminal escape sequences should be used.\n"
		"    Default is 'auto', which uses colour output when writing to a terminal,\n"
		"    but not when writing to a file or a pipe.\n"
		"  --describe\n"
		"    When listing packages, include the package summaries (short descriptions).\n"
		"  --explain\n"
		"    When listing packages, display licences as to justify\n"
		"    the free / non-free classification.\n"
		"  --help\n"
		"    Display this help and exit.\n"
		"  --licence-list\n"
		"    Specifies the list of good licences to use.\n"
		"  --list <none,free,nonfree,all>\n"
		"    Apart from displaying a summary number of free & non-free packages,\n"
		"    print them by name. The default value is 'nonfree'.\n"
		"  --version\n"
		"    Display version information and exit.\n"
	);
}
