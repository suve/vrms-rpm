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
#include <string.h>

#include "src/stringutils.h"


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

size_t str_squeeze_char(char *str, const char needle) {
	size_t len = strlen(str);
	size_t pos = 0;
	while(pos < len) {
		if(str[pos] != needle) {
			++pos;
			continue;
		}

		size_t match_len = 1;
		while(str[pos + match_len] == needle) ++match_len;

		if(match_len > 1) {
			memmove(str + pos + 1, str + pos + match_len, len - (pos + match_len) + 1);
			len -= (match_len - 1);
		}
		++pos;
	}
	return len;
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

int str_balance_parentheses(const char *input, char *buffer, const size_t bufSize, size_t *outputLen) {
	// Holds the result, allows easily bailing out via goto
	int ok = 0;

	// Keep track of number of bytes written
	size_t written = 0;
	// Convenience macro. Subtracts one extra byte, so we have space for the NUL terminator.
	#define REMAINING (bufSize - written - 1)

	size_t depth = 0;
	size_t inputLen; // Avoid the strlen() call since we're iterating over the string anyway
	{
		const char *s;
		for(s = input; *s != '\0'; ++s) {
			if(*s == ')') {
				if(depth == 0) {
					// Mismatched parentheses: a ')' that doesn't have a preceding '('.
					if(REMAINING == 0) goto terminateString;
					buffer[written++] = '(';
				} else {
					--depth;
				}
			} else if(*s == '(') {
				++depth;
			}
		}
		inputLen = s - input;
	}

	if(inputLen > REMAINING) goto terminateString;
	memcpy(buffer + written, input, inputLen);
	written += inputLen;

	// If depth is non-zero, that means we had '(' without matching ')'.
	if(depth > 0) {
		if(depth > REMAINING) goto terminateString;
		while(depth-- > 0) buffer[written++] = ')';
	}

	ok = 1;
	terminateString: {
		buffer[written] = '\0';
		if(outputLen != NULL) *outputLen = written;
		return ok;
	}
}

char* find_closing_paren(const char *str) {
	if(*str != '(') return NULL;

	unsigned int depth = 1;
	while(depth > 0) {
		switch(*(++str)) {
			case '(': ++depth; break;
			case ')': --depth; break;
			case '\0': return NULL;
		}
	}

	return (char*)str;
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
