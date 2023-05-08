/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2023 suve (a.k.a. Artur Frenszek-Iwicki)
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

#ifndef VRMS_RPM_PACKAGES_H
#define VRMS_RPM_PACKAGES_H

#include "classifiers.h"
#include "pipes.h"

extern struct Pipe* packages_openPipe(void);
extern int packages_read(struct Pipe *pipe, struct LicenceClassifier *classifier);

extern void packages_getcount(int *free, int *nonfree);
extern void packages_list(void);

extern void packages_free(void);

#endif
