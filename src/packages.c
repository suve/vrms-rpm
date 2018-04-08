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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "packages.h"

static FILE* child(int pipefd[]) {
	// Close our copy of the stdout file descriptor (inherited from parent)
	// and replace it with the write-descriptor for the pipe.
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
	
	exit(EXIT_FAILURE);
	return NULL;
}

static FILE* parent(const pid_t child_pid, int pipefd[]) {
	// Close our copy of the write-descriptor for the pipe.
	// If we leave it open, we won't be able to ever reach EOF
	// on the pipe's read-descriptor.
	close(pipefd[1]);
	
	return fdopen(pipefd[0], "r");
}

FILE* packages_read(void) {
	int pipefd[2];
	if(pipe(pipefd) != 0) return NULL;
	
	pid_t pid = fork();
	if(pid == -1) return NULL;
	
	if(pid == 0)
		return child(pipefd);
	else
		return parent(pid, pipefd);
}
