/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2021 Artur "suve" Iwicki
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

#include "versions.h"

#ifdef WITH_LIBRPM
	#include <rpm/rpmio.h>
	#include <rpm/rpmlib.h>
#else
	#include <string.h>
#endif

int compare_versions(const char *a, const char *b) {
	if(a == NULL) {
		if(b == NULL) {
			return 0;
		}
		return -1;
	} else {
		if(b == NULL) {
			return +1;
		}
	}

	#ifdef WITH_LIBRPM
		return rpmvercmp(a, b);
	#else
		// TODO: Add a better fallback mechanism
		return strcmp(a, b);
	#endif
}
