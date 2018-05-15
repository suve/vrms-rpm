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
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "fileutils.h"
#include "options.h"


#define FD_STDIN  0
#define FD_STDOUT 1
#define FD_STDERR 2

// Equal to standard filesystem block size; should be ok for IO.
static char buffer[4096];

void echo_file_contents(const char *const filename) {
	int fd = open(filename, O_RDONLY);
	if(fd == -1) return;
	
	// We're about to perform syscall IO, without going through libc.
	// As such, we should flush the stdout libc FILE stream to make sure
	// that whatever was left in libc buffers gets printed
	// before we make any write() calls.
	fflush(stdout);
	
	ssize_t bytes;
	while((bytes = read(fd, buffer, sizeof(buffer))) > 0) {
		write(FD_STDOUT, buffer, bytes);
	}
	
	close(fd);
}

void rms_disappointed() {
	switch(opt_image) {
		case OPT_IMAGE_ASCII:
			echo_file_contents(INSTALL_DIR "/images/rms-disappointed-ascii");
		break;

		case OPT_IMAGE_ICAT:
			echo_file_contents(INSTALL_DIR "/images/rms-disappointed-icat");
		break;
	}
}

void rms_happy() {
	switch(opt_image) {
		case OPT_IMAGE_ASCII:
			echo_file_contents(INSTALL_DIR "/images/rms-happy-ascii");
		break;

		case OPT_IMAGE_ICAT:
			echo_file_contents(INSTALL_DIR "/images/rms-happy-icat");
		break;
	}
}
