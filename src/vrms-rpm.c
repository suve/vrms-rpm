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

#include <stdio.h>
#include <string.h>

char* trim(char *buffer) {
	size_t len = strlen(buffer);
	if(len == 0) return buffer;
	
	char *end = buffer + len - 1;
	while(*end > '\0' && *end <= ' ') *end-- = '\0';
	
	while(*buffer > '\0' && *buffer <= ' ') *buffer++ = '\0';
	
	return buffer;
}

void list_licences(char *buffer) {
	char *openParen = strchr(buffer, '(');
	if(openParen) {
		char *closeParen = strchr(buffer, ')');
		
		*openParen = '\0';
		*closeParen = '\0';
		
		if(openParen != buffer) list_licences(buffer);
		list_licences(openParen + 1);
		list_licences(closeParen + 1);
		return;
	}
	
	char* separators[4] = {
		" and ", " or ", " And ", " Or "
	};
	for(int s = 0; s < 4; ++s) {
		char *sep = strstr(buffer, separators[s]);
		if(!sep) continue;
		
		char *after = sep + strlen(separators[s]);
		
		*sep = '\0';
		*(after - 1) = '\0';
		
		list_licences(buffer);
		list_licences(after);
		return;
	}
	
	buffer = trim(buffer);
	if(strlen(buffer)) puts(buffer);
}

int main(void) {
	char buffer[1024];
	char *name, *licence;
	
	while(fgets(buffer, sizeof(buffer), stdin)) {
		char *tab = strchr(buffer, '\t');
		if(!tab) continue;
		
		*tab = '\0';
		name = buffer;
		licence = trim(tab+1);
		
		puts(licence);
		list_licences(licence);
		puts("");
	}
}
