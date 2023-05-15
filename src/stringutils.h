/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2020-2021, 2023 suve (a.k.a. Artur Frenszek-Iwicki)
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

extern size_t str_squeeze_char(char *str, const char needle);

extern int str_split(char *const str, const char separator, char* *const fields, const int max_fields);

typedef int(*match_func_t)(const char*);
extern int str_match_first(
	const char *haystack,
	const match_func_t match_func[],
	char **result_ptr
);

extern const char* str_starts_with(const char *const haystack, const char *const needle);
extern char* str_ends_with(const char *const haystack, const char *const needle);

extern int str_compare_with_null_check(const char *first, const char *second, int(*compare_func)(const char*, const char*));

extern int str_balance_parentheses(const char *input, char *buffer, const size_t bufSize, size_t *written);
extern char* find_closing_paren(const char *str);

extern size_t replace_unicode_spaces(char *str);

#endif
