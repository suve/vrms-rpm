/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2020 Artur "suve" Iwicki
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

#ifndef VRMS_RPM_STRINGUTILS_H
#define VRMS_RPM_STRINGUTILS_H

#include <string.h>


#define ANSI_COLOUR(colour)  "\x1B" "[" #colour "m"

#define ANSI_RED     ANSI_COLOUR(31)
#define ANSI_GREEN   ANSI_COLOUR(32)
#define ANSI_RESET   ANSI_COLOUR(0)


extern char* trim(char *buffer, size_t *const length);
extern char* trim_extra(char *buffer, size_t *const length, const char *const extrachars);

extern void str_findmultiple(
	const char *const haystack,
	const int num_needles,
	char * *const needle,
	char * *const result_ptr,
	char * *const result_needle
);

extern const char* str_starts_with(const char *const haystack, const char *const needle);
extern char* str_ends_with(const char *const haystack, const char *const needle);

extern size_t replace_unicode_spaces(char *str);

#endif
