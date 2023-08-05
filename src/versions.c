/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2021, 2023 suve (a.k.a. Artur Frenszek-Iwicki)
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

#include "src/stringutils.h"
#include "src/versions.h"

#ifdef WITH_LIBRPM
#include <rpm/rpmio.h>
#include <rpm/rpmlib.h>

#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * The ordering here may seem rather weird, but it follows the RPM version logic.
 * - Version strings are composed of dot-separated components.
 * - Tildes and carets may be used to provide additional version information.
 *   In such case, the part before the tilde/caret is treated as the "base version".
 * - If both base versions are the same, consider the extra version info.
 * - Tilde extra version sorts before any non-tilde info (including lack thereof).
 * - Caret sorts after any non-caret info.
 * - Multiple tilde/caret components can be tacked onto the string.
 *
 * Now, we could split the version strings into sub-strings on '~' and '^',
 * but that's not needed; we can just go through them looking for any of '.', '~', '^',
 * or end-of-string. When we do that, the logic becomes:
 * - '~' vs '~' -> Reached end of base version. Base versions equal. Compare tilde-versions.
 * - '~' vs EOF -> Reached end of base version. Base versions equal. Tilde extra version comes before non-tilde. Return result.
 * - '~' vs '^' -> Same as above.
 * - '~' vs '.' -> Same as above.
 * - EOF vs EOF -> Exhausted both strings. Consider them equal. Return result.
 * - EOF vs '^' -> Reached end of base version. Base version equal. Caret extra version comes after non-caret. Return result.
 * - EOF vs '.' -> One of the strings has more components than the other. Longer string is greater. Return result.
 * - '^' vs '^' -> Reached end of base version. Base versions equal. Compare caret-versions.
 * - '^' vs '.' -> One of the strings has a longer base-version the the other. Longer base-version is greater. Return result.
 * - '.' vs '.' -> Both strings still have components. Compare them.
 *
 * When you combine the rules above, the weird ordering we have in this enum
 * allows for implementing said rules in a simple "if(a>b) else if(a<b)" manner,
 * as seen later in the code in fallback_compare().
 */
enum VersionComponentType {
	VCT_TILDE,  // Component preceded by '~'
	VCT_EXHAUSTED, // Reached end of string
	VCT_CARET,  // Component preceded by '^'
	VCT_NORMAL, // Component preceded by '.'
};

#define COMPBUF_LEN 64

struct VersionParser {
	const char *current;
	char previous;
	enum VersionComponentType type;
	char data[COMPBUF_LEN];
};

static void parser_init(const char *source, struct VersionParser *out) {
	out->current = source;
	out->previous = '.';
}

static void parser_advance(struct VersionParser *vp) {
	if(vp->current[0] == '\0') {
		vp->type = VCT_EXHAUSTED;
		return;
	}

	size_t component_len = 0;
	size_t jump_len = 0;

	char c;
	while(1) {
		c = vp->current[component_len];
		if((c == '.') || (c == '~') || (c == '^')) {
			jump_len = component_len + 1;
			break;
		} else if(c == '\0') {
			jump_len = component_len;
			break;
		}
		++component_len;
	}

	if(component_len >= (COMPBUF_LEN - 1)) component_len = (COMPBUF_LEN - 1);
	memcpy(vp->data, vp->current, component_len);
	vp->data[component_len] = '\0';

	if(vp->previous == '^')
		vp->type = VCT_CARET;
	else if(vp->previous == '~')
		vp->type = VCT_TILDE;
	else
		vp->type = VCT_NORMAL;

	vp->current += jump_len;
	vp->previous = c;
}

static int str_to_int(const char *str, long *result) {
	char *badchar;
	*result = strtol(str, &badchar, 10);
	return (badchar != NULL) && (*badchar == '\0');
}

static int fallback_compare(const char *a, const char *b) {
	struct VersionParser comp_a, comp_b;
	parser_init(a, &comp_a);
	parser_init(b, &comp_b);

	for(;;) {
		parser_advance(&comp_a);
		parser_advance(&comp_b);

		if(comp_a.type < comp_b.type) return -1;
		if(comp_a.type > comp_b.type) return +1;
		if(comp_a.type == VCT_EXHAUSTED) return 0;

		// Attempt to convert the segments to integers.
		// If this succeeds, compare their numerical values.
		long int a_int, b_int;
		const int a_is_int = str_to_int(comp_a.data, &a_int);
		const int b_is_int = str_to_int(comp_b.data, &b_int);
		if(a_is_int && b_is_int) {
			if(a_int > b_int) return +1;
			if(a_int < b_int) return -1;
			continue;
		}

		// If segments are not ints, compare them as strings.
		const int compare_strings = strcmp(comp_a.data, comp_b.data);
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
