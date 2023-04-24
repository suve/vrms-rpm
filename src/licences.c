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
	data->nodeBuf = rebuf_init();

	if((data->list == NULL) || (data->buffer == NULL) || (data->nodeBuf == NULL)) {
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
		rebuf_free(data->nodeBuf);
		free(data);
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

static int is_free(const struct LicenceData *data, char *licence) {
	const char *suffixes[] = {
		" with acknowledgement",
		" with advertising",
		" with additional permissions",
		" with attribution",
		" with exception",
		" with exceptions",
		" with font exception",
		" with linking exception",
		" with plugin exception",
		"-with-acknowledgement",
		"-with-advertising",
		"-with-additional-permissions",
		"-with-attribution",
		"-with-exception",
		"-with-exceptions",
		"-with-font-exception",
		"-with-linking-exception",
		"-with-plugin-exception",
		(const char*)NULL
	};
	
	int bs = binary_search(data, licence, 0, LIST_COUNT(data)-1);
	if(bs >= 0) return 1;
	
	// See if the licence ends with an acceptable suffix.
	// This allows us some flexibility when it comes to classifying licences.
	for(const char **suf = suffixes; *suf != NULL; ++suf) {
		char *sufpos = str_ends_with(licence, *suf);
		if(sufpos == NULL) continue;
		
		const char oldchar = *sufpos;
		*sufpos = '\0';
		
		bs = binary_search(data, licence, 0, LIST_COUNT(data)-1);
		*sufpos = oldchar;
		
		// It's not possible for a licence string to have two valid suffixes,
		// so we can return now, without looking through the rest of the suffixes.
		return bs >= 0;
	}
	
	return 0;
}

static char* find_closing_paren(char *start) {
	int depth = 1;
	while(depth > 0) {
		switch(*(++start)) {
			case '(': ++depth; break;
			case ')': --depth; break;
			case '\0': return NULL;
		}
	}
	return start;
}

static int is_opening_paren(const char *str) {
	return *str == '(';
}

static int is_and_joiner(const char *str) {
	if(str[0] != ' ') return 0;
	if((str[1] != 'a') && (str[1] != 'A')) return 0;
	if((str[2] != 'n') && (str[2] != 'N')) return 0;
	if((str[3] != 'd') && (str[3] != 'D')) return 0;
	if(str[4] != ' ') return 0;

	return 1;
}

static int is_or_joiner(const char *str) {
	if(str[0] != ' ') return 0;
	if((str[1] != 'o') && (str[1] != 'O')) return 0;
	if((str[2] != 'r') && (str[2] != 'R')) return 0;
	if(str[3] != ' ') return 0;

	return 1;
}

int get_joiner_len(enum LicenceTreeNodeType type) {
	switch(type) {
		case LTNT_LICENCE: return 1; // "("
		case LTNT_AND: return 5; // " and "
		case LTNT_OR: return 4; // " or "
		default: return -1;
	}
}

match_func_t get_joiner_func(enum LicenceTreeNodeType type) {
	switch(type) {
		case LTNT_LICENCE: return &is_opening_paren;
		case LTNT_AND: return &is_and_joiner;
		case LTNT_OR: return &is_or_joiner;
		default: return NULL;
	}
}

#define LTNT_PARENTHESISED 0xFF

static enum LicenceTreeNodeType detect_type(char *licence) {
	if(*licence == '(') {
		char *closingparen = find_closing_paren(licence);
		if(closingparen) {
			if(*(closingparen + 1) == '\0') return LTNT_PARENTHESISED;
			
			return detect_type(closingparen + 1);
		}
	}

	const match_func_t patterns[] = {
		&is_and_joiner,
		&is_or_joiner,
		NULL
	};
	const int joiner = str_match_first(licence, patterns, NULL);

	if(joiner == 0) return LTNT_AND;
	if(joiner == 1) return LTNT_OR;
	return LTNT_LICENCE;
}

// Helper macro: make a pointer to a LicenceTreeNode from the value located in the nodeBuf at given offset
#define NODEBUFPTR(offset) ((struct LicenceTreeNode*)(((char*)data->nodeBuf->data) + (offset)))

struct LicenceTreeNode* licence_classify(struct LicenceData *data, char* licence) {
	enum LicenceTreeNodeType type;
	while((type = detect_type(licence)) == LTNT_PARENTHESISED) {
		size_t liclen = strlen(licence);
		if(licence[liclen-1] == ')') licence[liclen-1] = '\0';
		++licence;
	}

	if(type == LTNT_LICENCE) {
		struct LicenceTreeNode *node = malloc(sizeof(struct LicenceTreeNode));
		if(node != NULL) {
			node->type = LTNT_LICENCE;
			node->licence = licence;
			node->is_free = is_free(data, licence);
		}
		return node;
	}

	const int joiner_len = get_joiner_len(type);
	const match_func_t patterns[] = {
		&is_opening_paren,
		get_joiner_func(type),
		NULL
	};

	const size_t bufStart = data->nodeBuf->used;
	int isFree = (type == LTNT_AND) ? 1 : 0;

	int match;
	do {
		char *needle_pos;
		match = str_match_first(licence, patterns, &needle_pos);

		struct LicenceTreeNode *child = NULL;
		if(match < 0) {
			size_t trimlen;
			licence = trim_extra(licence, &trimlen, "()");
			if(trimlen > 0) child = licence_classify(data, licence);
		} else if(match == 1) {
			*needle_pos = '\0';

			size_t partlen;
			char *part = trim(licence, &partlen);
			if(partlen > 0) child = licence_classify(data, part);

			licence = needle_pos + joiner_len;
		} else {
			char *closingparen = find_closing_paren(needle_pos);
			if(closingparen != NULL) {
				*closingparen = '\0';
				child = licence_classify(data, needle_pos + 1);
				licence = closingparen + 1;
			} else {
				licence = needle_pos + 1;
			}
		}

		if(child != NULL) {
			if(rebuf_append(data->nodeBuf, &child, sizeof(struct LicenceTreeNode*)) == NULL) {
				// Appending failed. Free any child nodes allocated so far, and bail out.
				for(size_t pos = bufStart; bufStart < data->nodeBuf->used; pos += sizeof(struct LicenceTreeNode*)) {
					licence_freeTree(NODEBUFPTR(pos));
				}
				data->nodeBuf->used = bufStart;
				return NULL;
			}
			isFree = (type == LTNT_AND) ? (isFree && child->is_free) : (isFree || child->is_free);
		}
	} while(match >= 0);

	const size_t bufDataLen = data->nodeBuf->used - bufStart;
	struct LicenceTreeNode *node = malloc(sizeof(struct LicenceTreeNode) + bufDataLen);
	if(node != NULL) {
		node->members = bufDataLen / sizeof(struct LicenceTreeNode*);
		memcpy(node->child, NODEBUFPTR(bufStart), bufDataLen);

		node->type = type;
		node->is_free = isFree;
	}

	data->nodeBuf->used = bufStart;
	return node;
}

void licence_freeTree(struct LicenceTreeNode *node) {
	if(node == NULL) return;

	if(node->type != LTNT_LICENCE) {
		for(unsigned int m = 0; m < node->members; ++m) licence_freeTree(node->child[m]);
	}
	free(node);
}
