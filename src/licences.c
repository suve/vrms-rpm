/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018 Artur "suve" Iwicki
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
#include <stdio.h>
#include <stdlib.h>

#include "licences.h"
#include "stringutils.h"

static int list_count = 0;
static int list_size = 0;
static char **licence_list;

#define LIST_STEP 512

static int expand_list(void) {
	const size_t bytes = (list_size + LIST_STEP) * sizeof(char*);
	
	if(licence_list == NULL) {
		licence_list = malloc(bytes);
		if(licence_list == NULL) return 1;
	} else {
		char** newptr = realloc(licence_list, bytes);
		if(newptr == NULL) return 1;
		licence_list = newptr;
	}
	
	list_size += LIST_STEP;
	return 0;
}

static size_t buffer_size = 0;
static char *licence_buffer;

#define BUFFER_STEP 2048

static int expand_buffer(void) {
	const size_t bytes = buffer_size + BUFFER_STEP;
	
	if(licence_buffer == NULL) {
		licence_buffer = malloc(bytes);
		if(licence_buffer == NULL) return 1;
	} else {
		char* newptr = realloc(licence_buffer, bytes);
		if(newptr == NULL) return 1;
		licence_buffer = newptr;
	}
	
	char *buf = licence_buffer;
	for(int i = 0; i < list_count; ++i) {
		licence_list[i] = buf;
		buf += strlen(buf) + 1;
	}
	
	buffer_size += BUFFER_STEP;
	return 0;
}

int licences_read(void) {
	licences_free();
	
	FILE *goodlicences = fopen("/usr/share/suve/vrms-rpm/good-licences.txt", "r");
	if(goodlicences == NULL) return -1;
	
	size_t buffer_pos = 0;
	
	char linebuffer[256];
	while(fgets(linebuffer, sizeof(linebuffer), goodlicences)) {
		size_t line_len;
		char *line;
		line = trim(linebuffer, &line_len);
		
		if(buffer_pos + line_len + 1 >= buffer_size) {
			if(expand_buffer() != 0) return -1;
		}
		if(list_count == list_size) {
			if(expand_list() != 0) return -1;
		}
		
		licence_list[list_count] = licence_buffer + buffer_pos;
		++list_count;
		
		memcpy(licence_buffer + buffer_pos, line, line_len+1);
		buffer_pos += line_len + 1;
	}
	
	fclose(goodlicences);
	return list_count;
}

void licences_free(void) {
	if(licence_list != NULL) {
		free(licence_list);
		licence_list = NULL;
	}
	list_count = list_size = 0;
	
	if(licence_buffer != NULL) {
		free(licence_buffer);
		licence_buffer = NULL;
	}
	buffer_size = 0;
}

static int binary_search(const char *const value, const int minpos, const int maxpos) {
	if(minpos > maxpos) return -1;
	
	const int pos = (minpos + maxpos) / 2;
	const int cmpres = strcasecmp(value, licence_list[pos]);
	
	if(cmpres < 0) return binary_search(value, minpos, pos-1);
	if(cmpres > 0) return binary_search(value, pos+1, maxpos);
	return pos;
}

static int is_free(const char *const licence) {
	return binary_search(licence, 0, list_count-1) >= 0;
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

static enum LicenceTreeNodeType detect_type(char *licence) {
	if(*licence == '(') {
		char *closingparen = find_closing_paren(licence);
		if(closingparen) return detect_type(closingparen + 1);
	}
	
	char *and_str = " and ";
	char *or_str = " or ";
	char *needles[] = {
		and_str,
		or_str
	};
	
	char *needle_str;
	str_findmultiple(licence, 2, needles, NULL, &needle_str);
	
	if(needle_str == and_str) return LTNT_AND;
	if(needle_str == or_str) return LTNT_OR;
	return LTNT_LICENCE;
}

static int count_members(char *licence, char *joiner_str) {
	int count = 1;
	const size_t joiner_len = strlen(joiner_str);
	
	char *paren_str = "(";
	char *needles[] = {
		paren_str,
		joiner_str
	};
	
	for(;;) {
		char *needle_str, *needle_pos;
		str_findmultiple(licence, 2, needles, &needle_pos, &needle_str);
		if(needle_pos == NULL) return count;
		
		if(needle_str == paren_str) {
			char *closingparen = find_closing_paren(needle_pos);
			if(closingparen != NULL) {
				licence = closingparen + 1;
				continue;
			}
		}
		
		++count;
		licence = needle_pos + joiner_len;
	}
}

static void add_child(struct LicenceTreeNode *node, struct LicenceTreeNode *child) {
	node->child[node->members++] = child;
	node->is_free = (node->type == LTNT_AND) ? (node->is_free && child->is_free) : (node->is_free || child->is_free);
}

struct LicenceTreeNode* licence_classify(char* licence) {
	enum LicenceTreeNodeType type = detect_type(licence);
	if(type == LTNT_LICENCE) {
		struct LicenceTreeNode *node = malloc(sizeof(struct LicenceTreeNode));
		node->type = LTNT_LICENCE;
		node->licence = licence;
		node->is_free = is_free(licence);
		return (struct LicenceTreeNode*)node;
	}
	
	char *joiner_str = type == LTNT_AND ? " and " : " or ";
	size_t joiner_len = strlen(joiner_str);
	
	int members = count_members(licence, joiner_str);
	
	struct LicenceTreeNode *node = malloc(sizeof(struct LicenceTreeNode) + members * sizeof(struct LicenceTreeNode*));
	node->type = type;
	node->members = 0;
	node->is_free = type == LTNT_AND ? 1 : 0;
	
	char *paren_str = "(";
	char *needles[] = {
		paren_str,
		joiner_str
	};
	
	for(;;) {
		char *needle_str, *needle_pos;
		str_findmultiple(licence, 2, needles, &needle_pos, &needle_str);
		
		if(needle_str == NULL) {
			size_t trimlen;
			licence = trim_extra(licence, &trimlen, "()");
			if(trimlen > 0) add_child(node, licence_classify(licence));
			
			return (struct LicenceTreeNode*)node;
		}
		
		struct LicenceTreeNode *child = NULL;
		if(needle_str == joiner_str) {
			*needle_pos = '\0';
			
			size_t partlen;
			char *part = trim(licence, &partlen);
			if(partlen > 0) child = licence_classify(part);
			
			licence = needle_pos + joiner_len;
		} else {
			char *closingparen = find_closing_paren(needle_pos);
			if(closingparen != NULL) {
				*closingparen = '\0';
				child = licence_classify(needle_pos + 1);
				licence = closingparen + 1;
			} else {
				licence = needle_pos + 1;
			}
		}
		
		if(child != NULL) add_child(node, child);
	}
}

void licence_freeTree(struct LicenceTreeNode *node) {
	if(node == NULL) return;
	
	if(node->type != LTNT_LICENCE) {
		for(int m = 0; m < node->members; ++m) licence_freeTree(node->child[m]);
	}
	free(node);
}
