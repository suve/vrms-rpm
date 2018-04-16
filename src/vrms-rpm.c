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
#include <stdlib.h>
#include <stdio.h>

#include "licences.h"
#include "packages.h"
#include "pipes.h"

int main(void) {
	struct Pipe *rpmpipe = packages_openPipe();
	if(rpmpipe == NULL) {
		fprintf(stderr, "vrms-rpm: failed to open pipe to rpm\n");
		exit(EXIT_FAILURE);
	}
	
	if(licences_read() < 0) {
		fprintf(stderr, "vrms-rpm: failed to read list of good licences");
		exit(EXIT_FAILURE);
	}
	
	if(packages_read(rpmpipe) < 0) {
		fprintf(stderr, "vrms-rpm: failed to read from rpm-pipe\n");
		exit(EXIT_FAILURE);
	}
	
	packages_free();
	licences_free();
	return 0;
}
