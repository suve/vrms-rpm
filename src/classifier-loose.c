/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2020-2024 suve (a.k.a. Artur Frenszek-Iwicki)
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
#include <stdlib.h>
#include <strings.h>

#include "src/buffers.h"
#include "src/classifiers.h"
#include "src/licences.h"
#include "src/stringutils.h"

#define LIST_COUNT(data) ((data)->list->used / sizeof(char*))

struct LooseClassifier {
	struct LicenceClassifier interface;
	const struct LicenceData *data;
	struct ReBuffer *nodeBuf;
};

// TODO: Convert the "suffix" code here to work similarly to the SPDX classifier.
//       Instead of trying all suffixes against the end of the string,
//       search for " WITH " or "-with-" first, slice the string
//       and then check if the suffix is on the list.
static char* extract_exception(char *licence) {
	static const char *suffixes[] = {
		" with acknowledgement",
		" with advertising",
		" with additional permissions",
		" with attribution",
		" with exception",
		" with exceptions",
		" with font exception",
		" with GCC exception",
		" with linking exception",
		" with plugin exception",
		"-with-acknowledgement",
		"-with-advertising",
		"-with-additional-permissions",
		"-with-attribution",
		"-with-exception",
		"-with-exceptions",
		"-with-font-exception",
		"-with-GCC-exception",
		"-with-linking-exception",
		"-with-plugin-exception",
		(const char*)NULL
	};

	for(const char **suf = suffixes; *suf != NULL; ++suf) {
		char *sufpos = str_ends_with(licence, *suf);
		if(sufpos == NULL) continue;

		// Trim the licence string where the suffix starts.
		*sufpos = '\0';

		// Return the static suffix string as the exception string.
		return *((char**)suf);
	}

	return NULL;
}

static int is_free(const struct LicenceData *data, char *licence) {
	return licences_find(data, licence) >= 0;
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

static int get_joiner_len(enum LicenceTreeNodeType type) {
	switch(type) {
		case LTNT_LICENCE: return 1; // "("
		case LTNT_AND: return 5; // " and "
		case LTNT_OR: return 4; // " or "
		default: return -1;
	}
}

static match_func_t get_joiner_func(enum LicenceTreeNodeType type) {
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
#define NODEBUFPTR(offset) ((struct LicenceTreeNode*)(((char*)self->nodeBuf->data) + (offset)))

static struct LicenceTreeNode* loose_classify(struct LicenceClassifier *class, char* licence) {
	struct LooseClassifier* self = (struct LooseClassifier*)class;

	enum LicenceTreeNodeType type;
	while((type = detect_type(licence)) == LTNT_PARENTHESISED) {
		size_t liclen = strlen(licence);
		if(licence[liclen-1] == ')') licence[liclen-1] = '\0';
		++licence;
	}

	if(type == LTNT_LICENCE) {
		struct LicenceTreeNode *node = licence_allocNode(0);
		if(node != NULL) {
			node->type = LTNT_LICENCE;
			node->exception = extract_exception(licence);
			node->licence = licence;
			node->is_free = is_free(self->data, licence);
		}
		return node;
	}

	const int joiner_len = get_joiner_len(type);
	const match_func_t patterns[] = {
		&is_opening_paren,
		get_joiner_func(type),
		NULL
	};

	const size_t bufStart = self->nodeBuf->used;
	int isFree = (type == LTNT_AND) ? 1 : 0;

	int match;
	do {
		char *needle_pos;
		match = str_match_first(licence, patterns, &needle_pos);

		struct LicenceTreeNode *child = NULL;
		if(match < 0) {
			size_t trimlen;
			licence = trim_extra(licence, &trimlen, "()");
			if(trimlen > 0) child = loose_classify(class, licence);
		} else if(match == 1) {
			*needle_pos = '\0';

			size_t partlen;
			char *part = trim(licence, &partlen);
			if(partlen > 0) child = loose_classify(class, part);

			licence = needle_pos + joiner_len;
		} else {
			char *closingparen = find_closing_paren(needle_pos);
			if(closingparen != NULL) {
				*closingparen = '\0';
				child = loose_classify(class, needle_pos + 1);
				licence = closingparen + 1;
			} else {
				licence = needle_pos + 1;
			}
		}

		if(child != NULL) {
			if(rebuf_append(self->nodeBuf, &child, sizeof(struct LicenceTreeNode*)) == NULL) {
				// Appending failed. Free any child nodes allocated so far, and bail out.
				for(size_t pos = bufStart; bufStart < self->nodeBuf->used; pos += sizeof(struct LicenceTreeNode*)) {
					licence_freeTree(NODEBUFPTR(pos));
				}
				self->nodeBuf->used = bufStart;
				return NULL;
			}
			isFree = (type == LTNT_AND) ? (isFree && child->is_free) : (isFree || child->is_free);
		}
	} while(match >= 0);

	const size_t bufDataLen = self->nodeBuf->used - bufStart;
	const unsigned int children = bufDataLen / sizeof(struct LicenceTreeNode*);

	struct LicenceTreeNode *node = licence_allocNode(children);
	if(node != NULL) {
		node->type = type;
		node->is_free = isFree;
		memcpy(node->child, NODEBUFPTR(bufStart), bufDataLen);
	}

	self->nodeBuf->used = bufStart;
	return node;
}

static void classifier_free(struct LicenceClassifier *class) {
	if(class != NULL) {
		struct LooseClassifier *self = (struct LooseClassifier*)class;
		rebuf_free(self->nodeBuf);
		free(self);
	}
}

struct LicenceClassifier* classifier_newLoose(const struct LicenceData *data) {
	struct LooseClassifier *self = malloc(sizeof(struct LooseClassifier));
	if(self == NULL) return NULL;

	struct ReBuffer *nodeBuf = rebuf_init(1024);
	if(nodeBuf == NULL) {
		free(self);
		return NULL;
	}

	self->data = data;
	self->nodeBuf = nodeBuf;

	self->interface.classify = &loose_classify;
	self->interface.free = &classifier_free;
	return &self->interface;
}
