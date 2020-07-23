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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FOREACH_MESSAGE(MESSAGE)     \
	MESSAGE(TRANSLATION_AUTHOR)      \
	MESSAGE(FREE_PACKAGES_COUNT)     \
	MESSAGE(NONFREE_PACKAGES_COUNT)  \
	MESSAGE(RMS_HAPPY)               \
	MESSAGE(RMS_DISAPPOINTED)        \
	MESSAGE(HELP_USAGE)              \
	MESSAGE(HELP_OPTION_ASCII)       \
	MESSAGE(HELP_OPTION_COLOUR)      \
	MESSAGE(HELP_OPTION_DESCRIBE)    \
	MESSAGE(HELP_OPTION_EXPLAIN)     \
	MESSAGE(HELP_OPTION_HELP)        \
	MESSAGE(HELP_OPTION_IMAGE)       \
	MESSAGE(HELP_OPTION_LICENCELIST) \
	MESSAGE(HELP_OPTION_LIST)        \
	MESSAGE(HELP_OPTION_VERSION)     \
	MESSAGE(ERR_PIPE_OPEN_FAILED)    \
	MESSAGE(ERR_PIPE_NOEVENTS)       \
	MESSAGE(ERR_PIPE_POLL_ERROR)     \
	MESSAGE(ERR_PIPE_POLL_HANGUP)    \
	MESSAGE(ERR_PIPE_READ_FAILED)    \
	MESSAGE(ERR_LICENCES_FAILED)     \
	MESSAGE(ERR_LICENCES_BADFILE)    \
	MESSAGE(ERR_BADOPT_COLOUR)       \
	MESSAGE(ERR_BADOPT_LIST)         \
	MESSAGE(ERR_BADOPT_NOARG)        \
	MESSAGE(ERR_BADOPT_UNKNOWN)      \


#define GENERATE_ENUM(what) MSG_ ## what,
enum MessageID {
	FOREACH_MESSAGE(GENERATE_ENUM)
};
#undef GENERATE_ENUM


extern void lang_init(void);

extern char* lang_getmsg(const enum MessageID msgid);
extern char* lang_getmsgn(const enum MessageID msgid, const int number);

extern int lang_fprint(FILE *const file, const enum MessageID msgid, ...);
extern int lang_fprint_n(FILE *const file, const enum MessageID msgid, const int number, ...);

#define lang_print(msgid, ...)    lang_fprint(stdout, (msgid), ##__VA_ARGS__)
#define lang_print_n(msgid, ...)  lang_fprint_n(stdout, (msgid), ##__VA_ARGS__)

#endif
