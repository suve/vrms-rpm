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

#ifndef VRMS_RPM_OPTIONS_H
#define VRMS_RPM_OPTIONS_H

#define OPT_IMAGE_NONE  0
#define OPT_IMAGE_ASCII 1
#define OPT_IMAGE_ICAT  2

#define OPT_LIST_FREE    (1<<0)
#define OPT_LIST_NONFREE (1<<1)

extern int opt_colour;
extern int opt_describe;
extern int opt_explain;
extern int opt_image;
extern int opt_list;
extern char* opt_licencelist;

void options_parse(int argc, char **argv);

#endif
