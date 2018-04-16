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
#include <string.h>

#include "stringutils.h"


#define IS_WHITESPACE(chr)  ((chr) > '\0' && (chr) <= ' ')
#define IS_EXTRA(chr)  (strchr(extrachars, (chr)) != NULL)

char* trim_extra(char *buffer, size_t *const length, const char *const extrachars) {
	size_t len = strlen(buffer);
	if(len > 0) {
		char *end = buffer + len - 1;
		while(IS_WHITESPACE(*end) || IS_EXTRA(*end)) {
			*end = '\0';
			--end;
			--len;
		}
		while(IS_WHITESPACE(*buffer) || IS_EXTRA(*buffer)) {
			*buffer = '\0';
			++buffer;
			--len;
		}
	}
	
	if(length != NULL) *length = len;
	return buffer;
}

char* trim(char *buffer, size_t *const length) {
	return trim_extra(buffer, length, "");
}

/*
 * TODO: Maybe improve the implementation so instead of calling ststr()
 *       multiple times and looking which needle appears earlier,
 *       we go through the haystack char-after-char, counting matching
 *       characters for each needle.
 */
void str_findmultiple(
	const char *const haystack,
	const int num_needles,
	char * *const needle,
	char * *const result_ptr,
	char * *const result_needle
) {
	char *best_ptr = NULL;
	char *best_needle = NULL;
	
	for(int n = 0; n < num_needles; ++n) {
		char *current_ptr = strstr(haystack, needle[n]);
		if(current_ptr == NULL) continue;
		
		if((best_ptr == NULL) || (current_ptr < best_ptr)) {
			best_ptr = current_ptr;
			best_needle = needle[n];
			
			// Bail out early if needle is right at start of haystack.
			// Can't get any better than that.
			if(best_ptr == haystack) break;
		}
	}
	
	if(result_ptr != NULL) *result_ptr = best_ptr;
	if(result_needle != NULL) *result_needle = best_needle;
}
