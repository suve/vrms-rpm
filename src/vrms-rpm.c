/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2023, 2025-2026 suve (a.k.a. Artur Frenszek-Iwicki)
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

#include "src/classifiers.h"
#include "src/fileutils.h"
#include "src/lang.h"
#include "src/licences.h"
#include "src/options.h"
#include "src/packages.h"
#include "src/pipes.h"
#include "src/printers.h"

static void easteregg(struct PackageData *pd, enum OptImage opt_image) {
	const size_t nonfree = pd->count[0];
	const size_t free = pd->count[1];
	if(nonfree == 0) {
		putc('\n', stdout);
		rms_happy(opt_image);
		lang_print(MSG_RMS_HAPPY);
	} else {
		const size_t total_packages = free + nonfree;
		if(nonfree > (total_packages / 10)) {
			putc('\n', stdout);
			rms_disappointed(opt_image);
			lang_print(MSG_RMS_DISAPPOINTED);
		}
	}
}

static struct LicenceClassifier* allocClassifier(
	const struct Options *opts,
	const struct LicenceData *data
) {
	switch(opts->grammar) {
		case OPT_GRAMMAR_LOOSE:
			return classifier_newLoose(data);
		case OPT_GRAMMAR_SPDX_STRICT:
			return classifier_newSPDX(data, 0);
		case OPT_GRAMMAR_SPDX_LENIENT:
			return classifier_newSPDX(data, 1);
		default:
			return NULL; // Should Never Happen (TM)
	}
}

static struct Printer* allocPrinter(const struct Options *opts) {
	struct PrinterSettings settings = (struct PrinterSettings){
		.colour = opts->colour,
		.describe = opts->describe,
		.evra = opts->evra,
		.explain = opts->explain,
		.file = stdout,
		.list = opts->list,
		.pretty = (opts->format == OPT_FORMAT_JSON_PRETTY),
		.textAnd = (opts->grammar == OPT_GRAMMAR_LOOSE) ? "and" : "AND",
		.textOr = (opts->grammar == OPT_GRAMMAR_LOOSE) ? "or" : "OR",
	};

	switch(opts->format) {
		case OPT_FORMAT_TEXT:
			return printer_newText(settings);
		case OPT_FORMAT_JSON:
		case OPT_FORMAT_JSON_PRETTY:
			return printer_newJSON(settings);
		default:
			return NULL;
	}
}

int main(int argc, char *argv[]) {
	lang_init();
	struct Options opts = options_parse(argc, argv);
	
	struct Pipe *rpmpipe = packages_openPipe(&opts);
	if(rpmpipe == NULL) {
		lang_fprint(stderr, MSG_ERR_PIPE_OPEN_FAILED);
		exit(EXIT_FAILURE);
	}

	struct LicenceData *licenses = licences_read(opts.licenceList);
	if(licenses == NULL) {
		lang_fprint(stderr, MSG_ERR_LICENCES_FAILED);
		exit(EXIT_FAILURE);
	}
	struct LicenceClassifier *classifier = allocClassifier(&opts, licenses);
	if(classifier == NULL) {
		lang_fprint(stderr, MSG_ERR_MALLOC);
		exit(EXIT_FAILURE);
	}
	
	struct PackageData *pkgs = packages_read(rpmpipe, classifier, &opts);
	if(pkgs == NULL) {
		lang_fprint(stderr, MSG_ERR_PIPE_READ_FAILED);
		exit(EXIT_FAILURE);
	}

	struct Printer *printer = allocPrinter(&opts);
	if(printer == NULL) {
		lang_fprint(stderr, MSG_ERR_MALLOC);
		exit(EXIT_FAILURE);
	}

	printer->print(printer,	pkgs);
	printer->free(printer);

	// TODO: Would make sense to move this into the text printer
	if(opts.format == OPT_FORMAT_TEXT) easteregg(pkgs, opts.image);
	
	packages_free(pkgs);
	classifier->free(classifier);
	licences_free(licenses);
	return 0;
}
