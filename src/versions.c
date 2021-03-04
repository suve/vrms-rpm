/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2021 Artur "suve" Iwicki
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

#include "stringutils.h"
#include "versions.h"

#ifdef WITH_LIBRPM
#include <rpm/rpmio.h>
#include <rpm/rpmlib.h>

#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void next_segment(const char **verstr, char *buffer, const size_t bufsize) {
	int segment_len;
	int jump_len;

	char *dot = strchr(*verstr, '.');
	if(dot != NULL) {
		segment_len = (dot - *verstr);
		jump_len = segment_len + 1;
	} else {
		jump_len = segment_len = strlen(*verstr);
	}

	snprintf(buffer, bufsize, "%.*s", segment_len, *verstr);
	*verstr += jump_len;
}

static int str_to_int(const char *str, long *result) {
	char *badchar;
	*result = strtol(str, &badchar, 10);
	return (badchar != NULL) && (*badchar == '\0');
}

static int fallback_compare(const char *a, const char *b) {
	char buffer_a[64];
	char buffer_b[64];

	for(;;) {
		// If we exhausted one of the version strings,
		// but the other one still has some segments,
		// consider the longer one to be greater.
		// If we exhausted both, consider them equal.
		if(*a == '\0') {
			if(*b == '\0') {
				return 0;
			}
			return -1;
		} else {
			if(*b == '\0') {
				return +1;
			}
		}

		// Retrieve the next segment of each version string.
		next_segment(&a, buffer_a, sizeof(buffer_a));
		next_segment(&b, buffer_b, sizeof(buffer_b));

		// Attempt to convert the segments to integers.
		// If this succeeds, compare their numerical values.
		long int a_int, b_int;
		const int a_is_int = str_to_int(buffer_a, &a_int);
		const int b_is_int = str_to_int(buffer_b, &b_int);
		if(a_is_int && b_is_int) {
			if(a_int > b_int) return +1;
			if(a_int < b_int) return -1;
			continue;
		}

		// If segments are not ints, compare them as strings.
		const int compare_strings = strcmp(buffer_a, buffer_b);
		if(compare_strings != 0) return compare_strings;
	}
}
#endif

int compare_versions(const char *a, const char *b) {
	#ifdef WITH_LIBRPM
		return str_compare_with_null_check(a, b, &rpmvercmp);
	#else
		return str_compare_with_null_check(a, b, &fallback_compare);
	#endif
}
