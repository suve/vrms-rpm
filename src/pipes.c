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
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "pipes.h"

#define FD_STDIN  0
#define FD_STDOUT 1
#define FD_STDERR 2

struct Pipe {
	int readfd;
	int writefd;
	pid_t child_pid;
};

static void child(struct Pipe *const pipe, const int argc, char **argv) {
	// Close our copy of the stdout file descriptor (inherited from parent)
	// and replace it with the write-descriptor for the pipe.
	close(FD_STDOUT);
	dup2(pipe->readfd, FD_STDOUT);
	
	char *args[argc+1];
	for(int a = 0; a < argc; ++a) args[a] = argv[a];
	args[argc] = (char*)NULL;
	
	execv(args[0], args);
	exit(EXIT_FAILURE);
}

static void parent(struct Pipe *const pipe) {
	// Close our copy of the write-descriptor for the pipe.
	// If we leave it open, we won't be able to ever reach EOF
	// on the pipe's read-descriptor.
	close(pipe->writefd);
}

struct Pipe* pipe_create(const int argc, char **argv) {
	struct Pipe *res = malloc(sizeof(struct Pipe));
	if(res == NULL) return NULL;
	
	int pipefd[2];
	if(pipe(pipefd) != 0) {
		free(res);
		return NULL;
	}
	
	pid_t pid = fork();
	if(pid == -1) {
		close(pipefd[0]);
		close(pipefd[1]);
		free(res);
		return NULL;
	}
	
	if(pid == 0)
		child(res, argc, argv);
	else
		parent(res);
	
	return res;
}

FILE* pipe_fopen(struct Pipe *pipe) {
	const int fd = pipe->readfd;
	free(pipe);
	
	struct pollfd pfd = {
		.fd = fd, 
		.events = POLLIN
	};
	
	int events = poll(&pfd, 1, -1);
	if(events < 0) return NULL;
	if(pfd.revents == POLLERR) return NULL;
	
	// Interpret "other end of pipe closed" as error
	// only when this event is not accompanied by "data ready to be read".
	if((pfd.revents & POLLHUP) && !(pfd.revents & POLLIN)) return NULL;
	
	return fdopen(fd, "r");
}

void pipe_destroy(struct Pipe *pipe) {
	close(pipe->readfd);
	close(pipe->writefd);
	free(pipe);
}
