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
#ifndef VRMS_RPM_LANG_H
#define VRMS_RPM_LANG_H

#include <string.h>

#define FOREACH_MESSAGE(MESSAGE)   \
	MESSAGE(TRANSLATION_AUTHOR)     \
	MESSAGE(FREE_PACKAGES_COUNT)    \
	MESSAGE(NONFREE_PACKAGES_COUNT) \
	MESSAGE(RMS_HAPPY)              \
	MESSAGE(RMS_DISAPPOINTED)       \


#define GENERATE_ENUM(what) MSG_ ## what,
enum MessageID {
	FOREACH_MESSAGE(GENERATE_ENUM)
};
#undef GENERATE_ENUM


void lang_init(void);

char* lang_getmsg(const enum MessageID msgid);
char* lang_getmsgn(const enum MessageID msgid, const int number);

int lang_printmsg(const enum MessageID msgid, ...);
int lang_printmsgn(const enum MessageID msgid, const int number, ...);

#endif
