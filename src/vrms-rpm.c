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
#include <stdlib.h>
#include <stdio.h>

#include "classifiers.h"
#include "fileutils.h"
#include "lang.h"
#include "licences.h"
#include "options.h"
#include "packages.h"
#include "pipes.h"

static void easteregg(void) {
	int free, nonfree;
	packages_getcount(&free, &nonfree);
	
	if(nonfree == 0) {
		putc('\n', stdout);
		rms_happy();
		lang_print(MSG_RMS_HAPPY);
	} else {
		const int total_packages = free + nonfree;
		if(nonfree > (total_packages / 10)) {
			putc('\n', stdout);
			rms_disappointed();
			lang_print(MSG_RMS_DISAPPOINTED);
		}
	}
}

static struct LicenceClassifier* allocClassifier(const struct LicenceData *data) {
	switch(opt_grammar) {
		case OPT_GRAMMAR_LOOSE:
			return classifier_newLoose(data);
		case OPT_GRAMMAR_SPDX:
			return classifier_newSPDX(data);
		default:
			return NULL; // Should Never Happen (TM)
	}
}

int main(int argc, char *argv[]) {
	lang_init();
	options_parse(argc, argv);
	
	struct Pipe *rpmpipe = packages_openPipe();
	if(rpmpipe == NULL) {
		lang_fprint(stderr, MSG_ERR_PIPE_OPEN_FAILED);
		exit(EXIT_FAILURE);
	}

	struct LicenceData *licenses = licences_read();
	if(licenses == NULL) {
		lang_fprint(stderr, MSG_ERR_LICENCES_FAILED);
		exit(EXIT_FAILURE);
	}
	struct LicenceClassifier *classifier = allocClassifier(licenses);
	if(classifier == NULL) {
		// TODO: This needs its own error message
		lang_fprint(stderr, MSG_ERR_LICENCES_FAILED);
		exit(EXIT_FAILURE);
	}
	
	if(packages_read(rpmpipe, classifier) < 0) {
		lang_fprint(stderr, MSG_ERR_PIPE_READ_FAILED);
		exit(EXIT_FAILURE);
	}
	
	packages_list();
	easteregg();
	
	packages_free();
	classifier->free(classifier);
	licences_free(licenses);
	return 0;
}
