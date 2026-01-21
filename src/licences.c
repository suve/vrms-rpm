/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2020-2023, 2025-2026 suve (a.k.a. Artur Frenszek-Iwicki)
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
#include <strings.h>

#include "src/buffers.h"
#include "src/config.h"
#include "src/lang.h"
#include "src/licences.h"
#include "src/memory.h"
#include "src/stringutils.h"

const struct LicenceTreeNode PubkeyLicence = (struct LicenceTreeNode) {
	.type = LTNT_LICENCE,
	.is_free = 1,
	.licence = "pubkey",
};

#define LIST_COUNT(data) ((data)->list->used / sizeof(char*))

static FILE* openfile(const char *name) {
	char* buffer = NULL;
	FILE *f = NULL;

	const int skip_builtin_check = str_starts_with(name, "/") || str_starts_with(name, "./") || str_starts_with(name, "../");
	if(!skip_builtin_check) {
		const size_t bufsize = strlen(name) + strlen(INSTALL_DIR "/licences/.txt") + 1;
		buffer = mem_alloc(bufsize);

		snprintf(buffer, bufsize, INSTALL_DIR "/licences/%s.txt", name);
		f = fopen(buffer, "r");
	}

	// There's no built-in file with said name, or we skipped the built-in check - try again, passing "name" as-is
	if(f == NULL) {
		f = fopen(name, "r");
		// Both attempts failed, print error message
		if(f == NULL) {
			const char *errstr = strerror(errno);
			lang_fprint(stderr, MSG_ERR_LICENCES_BADFILE, name, errstr);
		}
	}

	if(buffer != NULL) mem_free(buffer);
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
	struct LicenceData *data = mem_alloc(sizeof(struct LicenceData));
	data->list = rebuf_init(500 * sizeof(void*));
	data->buffer = chainbuf_init(8000);

	return data;
}

struct LicenceData* licences_read(const char *listName) {
	FILE *goodlicences = NULL;
	struct LicenceData *result = NULL;

	goodlicences = openfile(listName);
	if(goodlicences == NULL) goto fail;
	
	result = licensedata_init();

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
		mem_free(data);
	}
}

static int binary_search(const struct LicenceData *data, const char *const value, const int minpos, const int maxpos) {
	if(minpos > maxpos) return -1;

	const int pos = (minpos + maxpos) / 2;
	const int cmpres = strcasecmp(value, ((char**)data->list->data)[pos]);

	if(cmpres < 0) return binary_search(data, value, minpos, pos-1);
	if(cmpres > 0) return binary_search(data, value, pos+1, maxpos);
	return pos;
}

int licences_find(const struct LicenceData *data, const char *licence) {
	return binary_search(data, licence, 0, LIST_COUNT(data)-1);
}

void licence_freeTree(struct LicenceTreeNode *node) {
	if(node == NULL) return;

	if(node->type != LTNT_LICENCE) {
		for(unsigned int m = 0; m < node->members; ++m) licence_freeTree(node->child[m]);
	}
	mem_free(node);
}
