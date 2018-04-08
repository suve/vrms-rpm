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


#define IS_WHITESPACE_OR_PAREN(chr)  \
	((chr) > '\0' && ((chr) <= ' ' || (chr) == '(' || (chr) == ')'))

char* trim(char *buffer, size_t *length) {
	size_t len = strlen(buffer);
	if(len > 0) {
		char *end = buffer + len - 1;
		while(IS_WHITESPACE_OR_PAREN(*end)) {
			*end = '\0';
			--end;
			--len;
		}
		while(IS_WHITESPACE_OR_PAREN(*buffer)) {
			*buffer = '\0';
			++buffer;
			--len;
		}
	}
	
	if(length != NULL) *length = len;
	return buffer;
}
