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
#include <string.h>
#include <unistd.h>

#include "stringutils.h"

int get_packages(void) {
	int pipefd[2];
	if(pipe(pipefd) != 0) exit(EXIT_FAILURE);
	
	pid_t pid = fork();
	if(pid == -1) exit(EXIT_FAILURE);
	
	if(pid == 0) {
		close(1);
		dup2(pipefd[1], 1);
		
		char *args[] = {
			"/usr/bin/rpm",
			"--all",
			"--query",
			"--queryformat",
			"%{NAME}\\t%{LICENSE}\\n",
			(char*)NULL
		};
		execv(args[0], args);
		
		perror("-- execv() failed"); 
	} else {
		close(0);
		dup2(pipefd[0], 0);
		close(pipefd[1]);
	}
}

char *licence_list[512];
int licence_count = 0;

char licence_buffer[4096];

int get_licences(void) {
	FILE *goodlicences = fopen("/usr/share/suve/vrms-rpm/good-licences.txt", "r");
	if(goodlicences == NULL) exit(EXIT_FAILURE);
	
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
}

int binary_search(const char *const value, const int minpos, const int maxpos) {
	if(minpos > maxpos) return -1;
	
	const int pos = (minpos + maxpos) / 2;
	const int cmpres = strcasecmp(value, licence_list[pos]);
	
	if(cmpres == 0) return pos;
	if(cmpres < 0) return binary_search(value, minpos, pos-1);
	if(cmpres > 0) return binary_search(value, pos+1, maxpos);
}

int is_good_licence(const char *const licence) {
	return binary_search(licence, 0, licence_count-1) >= 0;
}

void list_licences(char *buffer) {
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
	
	size_t trimmed_length;
	buffer = trim(buffer, &trimmed_length);
	if(trimmed_length) {
		printf("%s%s" ANSI_RESET "\n", is_good_licence(buffer) ? ANSI_GREEN : ANSI_RED, buffer);
	}
}

int main(void) {
	get_packages();
	get_licences();
	
	char buffer[1024];
	char *name, *licence;
	
	while(fgets(buffer, sizeof(buffer), stdin)) {
		char *tab = strchr(buffer, '\t');
		if(!tab) continue;
		
		*tab = '\0';
		name = buffer;
		licence = trim(tab+1, NULL);
		
		puts(licence);
		list_licences(licence);
		puts("");
	}
}
