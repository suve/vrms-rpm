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
#include <sys/wait.h>
#include <unistd.h>

#include "licences.h"
#include "packages.h"
#include "stringutils.h"

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
	buffer = trim_extra(buffer, &trimmed_length, "()");
	if(trimmed_length) {
		printf("%s%s" ANSI_RESET "\n", licence_is_free(buffer) ? ANSI_GREEN : ANSI_RED, buffer);
	}
}

int main(void) {
	FILE* pkgs = packages_read();
	if(pkgs == NULL) exit(EXIT_FAILURE);
	if(licences_read() != 0) exit(EXIT_FAILURE);
	
	// Check if the child process that should exec() into /usr/bin/rpm
	// has exited or not. If it has exited, it most likely failed.
	//
	// Of course, there's the risk that the child process has been
	// stalled and has not made it to exec() yet.
	//
	// Hopefully, reading the licence file took us enough time.
	// If someone has a better idea on how to solve this,
	// I encourage them to submit a Pull Request.
	int wstatus;
	waitpid(-1, &wstatus, WNOHANG);
	if(WEXITSTATUS(wstatus) != 0) exit(EXIT_FAILURE);
	
	char buffer[1024];
	char *name, *licence;
	
	while(fgets(buffer, sizeof(buffer), pkgs)) {
		char *tab = strchr(buffer, '\t');
		if(!tab) continue;
		
		*tab = '\0';
		name = buffer;
		licence = trim(tab+1, NULL);
		
		puts(licence);
		list_licences(licence);
		puts("");
	}
	
	licences_free();
	fclose(pkgs);
}
