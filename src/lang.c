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
#include <libintl.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/config.h"
#include "src/lang.h"

#define GENERATE_STRING(what) #what "\n",
static const char *const msgname[] = {
	FOREACH_MESSAGE(GENERATE_STRING)
};


void lang_init(void) {
	setlocale(LC_ALL, "");
	bindtextdomain("vrms-rpm", INSTALL_PREFIX "/share/locale");
	textdomain("vrms-rpm");
	
	// Take a look at the test string and change locale to English
	// if there isn't a translation available
	const char *teststr = lang_getmsg(MSG_TRANSLATION_AUTHOR);
	if(strcmp(teststr, msgname[MSG_TRANSLATION_AUTHOR]) == 0) {
		setenv("LANGUAGE", "en", 1);
	}
}

char* lang_getmsg(const enum MessageID msgid) {
	return gettext(msgname[msgid]);
}

char* lang_getmsgn(const enum MessageID msgid, const int number) {
	return ngettext(msgname[msgid], msgname[msgid], number);
}

int lang_fprint(FILE *const file, const enum MessageID msgid, ...) {
	char *msgstr = lang_getmsg(msgid);
	if(msgstr == NULL) return -1;
	
	va_list args;
	va_start(args, msgid);
	int bytes = vfprintf(file, msgstr, args);
	va_end(args);
	
	return bytes;
}

int lang_fprint_n(FILE *const file, const enum MessageID msgid, const int number, ...) {
	char *msgstr = lang_getmsgn(msgid, number);
	if(msgstr == NULL) return -1;
	
	va_list args;
	va_start(args, number);
	int bytes = vfprintf(file, msgstr, args);
	va_end(args);
	
	return bytes;
}
