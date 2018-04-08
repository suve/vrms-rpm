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
#include <stdlib.h>

#include "licences.h"
#include "stringutils.h"

static char *licence_list[512];
static int licence_count = 0;

static char licence_buffer[4096];

int licences_read(void) {
	FILE *goodlicences = fopen("/usr/share/suve/vrms-rpm/good-licences.txt", "r");
	if(goodlicences == NULL) return 1;
	
	size_t buffer_pos = 0;
	
	char linebuffer[256];
	while(fgets(linebuffer, sizeof(linebuffer), goodlicences)) {
		size_t line_len;
		char *line;
		line = trim(linebuffer, &line_len);
		
		licence_list[licence_count] = licence_buffer + buffer_pos;
		++licence_count;
		
		memcpy(licence_buffer + buffer_pos, line, line_len+1);
		buffer_pos += line_len + 1;
	}
	
	fclose(goodlicences);
	return 0;
}

void licences_free(void) {
	memset(licence_buffer, 0, sizeof(licence_buffer));
	memset(licence_list, 0, sizeof(licence_list));
	licence_count = 0;
}

static int binary_search(const char *const value, const int minpos, const int maxpos) {
	if(minpos > maxpos) return -1;
	
	const int pos = (minpos + maxpos) / 2;
	const int cmpres = strcasecmp(value, licence_list[pos]);
	
	if(cmpres == 0) return pos;
	if(cmpres < 0) return binary_search(value, minpos, pos-1);
	if(cmpres > 0) return binary_search(value, pos+1, maxpos);
}

int licence_is_free(const char *const licence) {
	return binary_search(licence, 0, licence_count-1) >= 0;
}

int licence_is_nonfree(const char *const licence) {
	return binary_search(licence, 0, licence_count-1) < 0;
}
