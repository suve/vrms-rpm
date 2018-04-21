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

#define OPT_COLOUR_NO   0
#define OPT_COLOUR_YES  1
#define OPT_COLOUR_AUTO 2

int opt_ascii = 0;
int opt_colour = OPT_COLOUR_AUTO;
int opt_explain = 0;
int opt_list = OPT_LIST_NONFREE;


#define ARG_NON no_argument
#define ARG_OPT optional_argument
#define ARG_REQ required_argument

enum LongOpt {
	LONGOPT_HELP = 1,
	LONGOPT_COLOUR,
	LONGOPT_LIST,
	LONGOPT_VERSION
};

#define streq(a, b)  (strcmp((a), (b)) == 0)

void options_parse(int argc, char **argv) {
	struct option vrms_opts[] = {
		{  "ascii", ARG_NON, &opt_ascii, 1 },
		{  "color", ARG_REQ, NULL, LONGOPT_COLOUR },
		{ "colour", ARG_REQ, NULL, LONGOPT_COLOUR },
		{"explain", ARG_NON, &opt_explain, 1 },
		{   "help", ARG_NON, NULL, LONGOPT_HELP },
		{   "list", ARG_REQ, NULL, LONGOPT_LIST },
		{"version", ARG_NON, NULL, LONGOPT_VERSION },
		{         0,       0, 0, 0 },
	};

	while(1) {
		int option_index = 0;
		
		int res = getopt_long(argc, argv, "", vrms_opts, &option_index);
		if(res == -1) break;

		switch (res) {
			case LONGOPT_HELP:
				// print_help();
				exit(EXIT_SUCCESS);
			
			case LONGOPT_COLOUR:
				if(streq(optarg, "auto")) {
					opt_colour = OPT_COLOUR_AUTO;
				} else if(streq(optarg, "no")) {
					opt_colour = OPT_COLOUR_NO;
				} else if(streq(optarg, "yes")) {
					opt_colour = OPT_COLOUR_YES;
				} else {
					// TODO: print error message here
					exit(EXIT_FAILURE);
				}
			break;
			
			case LONGOPT_LIST:
				if(streq(optarg, "all")) {
					opt_list = OPT_LIST_FREE | OPT_LIST_NONFREE;
				} else if(streq(optarg, "free")) {
					opt_list = OPT_LIST_FREE;
				} else if(streq(optarg, "nonfree") || streq(optarg, "non-free")) {
					opt_list = OPT_LIST_NONFREE;
				} else if(streq(optarg, "none")) {
					opt_list = 0;
				} else {
					// TODO: print error message here
					exit(EXIT_FAILURE);
				}
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
