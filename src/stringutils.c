/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2020-2021 Artur "suve" Iwicki
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

	char *end = buffer + len - 1;
	while((len > 0) && (IS_WHITESPACE(*end) || IS_EXTRA(*end))) {
		*end = '\0';
		--end;
		--len;
	}

	while((len > 0) && (IS_WHITESPACE(*buffer) || IS_EXTRA(*buffer))) {
		*buffer = '\0';
		++buffer;
		--len;
	}
	
	if(length != NULL) *length = len;
	return buffer;
}

char* trim(char *buffer, size_t *const length) {
	return trim_extra(buffer, length, "");
}

int str_split(char *const str, const char separator, char* *const fields, const int max_fields) {
	fields[0] = str;
	for(int i = 1; i < max_fields; ++i) fields[i] = NULL;

	char *search_pos = str;

	int count = 1;
	while(count < max_fields) {
		char* sep = strchr(search_pos, separator);
		if(sep == NULL) break;

		*sep = '\0';
		fields[count] = search_pos = (sep + 1);
		count += 1;
	}

	return count;
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
	const char *const *const needle,
	char * *const result_ptr,
	const char * *const result_needle
) {
	char *best_ptr = NULL;
	const char *best_needle = NULL;
	
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

int str_match_first(
	const char *haystack,
	const match_func_t match_func[],
	char **result_ptr
) {
	for(; *haystack != '\0'; ++haystack) {
		for(int f = 0; match_func[f] != NULL; ++f) {
			if(match_func[f](haystack)) {
				if(result_ptr != NULL) *result_ptr = (char*)haystack;
				return f;
			}
		}
	}

	if(result_ptr != NULL) *result_ptr = NULL;
	return -1;
}

const char* str_starts_with(const char *const haystack, const char *const needle) {
	const size_t stacklen = strlen(haystack);
	const size_t ndllen = strlen(needle);
	if(stacklen < ndllen) return NULL;

	return (memcmp(haystack, needle, ndllen) == 0) ? haystack : NULL;
}

char* str_ends_with(const char *const haystack, const char *const needle) {
	const size_t stacklen = strlen(haystack);
	const size_t ndllen = strlen(needle);
	if(stacklen < ndllen) return NULL;

	char *cmppos = (char*)(haystack + (stacklen - ndllen));
	return (memcmp(cmppos, needle, ndllen) == 0) ? cmppos : NULL;
}

int str_compare_with_null_check(const char *first, const char *second, int(*compare_func)(const char*, const char*)) {
	if(first != NULL) {
		if(second != NULL) {
			const int result = compare_func(first, second);
			// Normalize the result from the function to one of [-1, 0, +1].
			if(result > 0) return +1;
			if(result < 0) return -1;
			return 0;
		} else {
			return +1;
		}
	} else {
		if(second != NULL) {
			return -1;
		} else {
			return 0;
		}
	}
}

size_t replace_unicode_spaces(char *text) {
	size_t textlen = strlen(text);
	
	char* spaces[] = {
		"\u00A0", // no-break space
		"\u2000", // en quad
		"\u2001", // em quad
		"\u2002", // en space
		"\u2003", // em space
		"\u2004", // thrree-per-em space
		"\u2005", // four-per-em space
		"\u2006", // six-per-em space
		"\u2007", // figure space
		"\u2008", // punctuation space
		"\u2009", // thin space
		"\u200A", // hair space
		"\u202F", // narrow no-break space
		(char*)NULL
	};
	
	int n = 0;
	char *needle;
	while((needle = spaces[n++]) != NULL) {
		char *pos = strstr(text, needle);
		if(pos == NULL) continue;
		
		const size_t ndllen = strlen(needle);
		do {
			*pos = ' ';
			
			char *const move_to = pos + 1;
			char *const move_from = pos + ndllen;
			const size_t move_size = textlen - (move_from - text) + 1; // Plus one for trailing NUL byte
			memmove(move_to, move_from, move_size);
			
			// Plus one because the needle is replaced by ' ', not removed totally
			textlen = textlen - ndllen + 1;
		} while((pos = strstr(text, needle)) != NULL);
	}
	
	return textlen;
}
