/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2020-2023 suve (a.k.a. Artur Frenszek-Iwicki)
 * Copyright (C) 2018 Marcin "dextero" Radomski
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "buffers.h"
#include "config.h"
#include "lang.h"
#include "licences.h"
#include "options.h"
#include "stringutils.h"

#define LIST_COUNT(data) ((data)->list->used / sizeof(char*))

static FILE* openfile(char *name) {
	char buffer[512];
	
	const int non_built_in = str_starts_with(name, "/") || str_starts_with(name, "./") || str_starts_with(name, "../");
	if(!non_built_in) {
		snprintf(buffer, sizeof(buffer), INSTALL_DIR "/licences/%s.txt", name);
		name = buffer;
	}
	
	FILE *f = fopen(name, "r");
	if(f == NULL) {
		char *errstr = strerror(errno);
		lang_fprint(stderr, MSG_ERR_LICENCES_BADFILE, name, errstr);
	}
	
	return f;
}

static int comparelicences(const void *A, const void *B) {
	const char *const *a = A;
	const char *const *b = B;
	
	return strcasecmp(*a, *b);
}

static void licences_sort(struct LicenceData *data) {
	qsort(data->list->data, LIST_COUNT(data), sizeof(char*), &comparelicences);
}

static struct LicenceData* licensedata_init(void) {
	struct LicenceData *data = malloc(sizeof(struct LicenceData));
	if(data == NULL) return NULL;

	data->list = rebuf_init();
	data->buffer = chainbuf_init();

	if((data->list == NULL) || (data->buffer == NULL)) {
		licences_free(data);
		return NULL;
	}

	return data;
}

struct LicenceData* licences_read(void) {
	struct LicenceData *result = licensedata_init();
	if(result == NULL) return NULL;
	
	FILE *goodlicences = openfile(opt_licencelist);
	if(goodlicences == NULL) goto fail;
	
	char linebuffer[256];
	while(fgets(linebuffer, sizeof(linebuffer), goodlicences)) {
		size_t line_len;
		char *line;
		line = trim(linebuffer, &line_len);
		
		char *insert_pos = chainbuf_append(&result->buffer, line);
		if(insert_pos == NULL) goto fail;
		
		if(rebuf_append(result->list, &insert_pos, sizeof(char*)) == NULL) goto fail;
	}
	fclose(goodlicences);

	licences_sort(result);
	return result;

	fail: { // As seen in CVE-2014-1266!
		if(goodlicences != NULL) fclose(goodlicences);
		licences_free(result);
		return NULL;
	}
}

void licences_free(struct LicenceData *data) {
	if(data != NULL) {
		rebuf_free(data->list);
		chainbuf_free(data->buffer);
		free(data);
	}
}

void licence_freeTree(struct LicenceTreeNode *node) {
	if(node == NULL) return;

	if(node->type != LTNT_LICENCE) {
		for(unsigned int m = 0; m < node->members; ++m) licence_freeTree(node->child[m]);
	}
	free(node);
}
