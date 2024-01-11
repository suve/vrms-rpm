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

// Try to find the WITH operator. The operator is matched in a case-insensitive
// manner, and can be surrounded by either spaces or hyphens.
//
// The string is traversed in reverse because otherwise input like:
//   "Licence name with spaces with extra permissions"
// would be split into "Licence name" and "spaces with extra permissions"
// instead of "Licence name with spaces" and "extra permissions".
static char* find_WITH_operator(char *licence) {
	const char matchChars[] = " HTIW ";
	const char altChars[] = "-htiw-";

	const size_t len = strlen(licence);
	if(len == 0) return NULL; // Better safe than sorry
	size_t pos = len - 1;

	int charsMatched = 0;
	while(1) {
		char c = licence[pos];
		if((c == matchChars[charsMatched]) || (c == altChars[charsMatched])) {
			++charsMatched;
			if(charsMatched == 6) { // " WITH " or "-with-" or whatever -> 6 chars
				return licence + pos;
			}
		} else {
			charsMatched = 0;
		}

		if(pos == 0) return NULL;
		--pos;
	}
}

static int is_free(const struct LicenceData *data, char *licence) {
	// TODO: Multi-word suffixes appear in the list twice: first using
	//       a space-separated form, and then a hyphen-seperated one.
	//       Devise some mechanism to get rid of this duplication.
	const char *suffixes[] = {
		"acknowledgement",
		"advertising",
		"additional permissions",
		"additional-permissions",
		"attribution",
		"exception",
		"exceptions",
		"font exception",
		"font-exception",
		"GCC exception",
		"GCC-exception",
		"linking exception",
		"linking-exception",
		"plugin exception",
		"plugin-exception",
		(const char*)NULL
	};

	char *with = find_WITH_operator(licence);
	if(with != NULL) {
		// The `with` pointer points to space/hyphen immediately before
		// the "WITH" operator. Adding 6 chars skips past "-with-".
		char *past_with = with + 6;
		// Skip past any extra spaces/hyphens, should there be any.
		while((*past_with == ' ') || (*past_with == '-')) ++past_with;

		// Check if there's a comma before the WITH operator, e.g.
		// "Licence name, with extra permissions".
		if((with > licence) && (*(with-1) == ',')) {
			--with;
		}
		// Remove any extra spaces/hyphens, should there be any.
		while(with > licence) {
			char previous = *(with - 1);
			if((previous != ' ') && (previous != '-')) break;
			--with;
		}

		for(const char **suf = suffixes; *suf != NULL; ++suf) {
			if(strcmp(past_with, *suf) != 0) continue;

			// Store the character appearing before the "WITH" operator.
			// We allow both spaces and hyphens, so we must remember which one was it.
			const char oldChar = *with;

			*with = '\0'; // Trim the licence string before lookup
			const int search = licences_find(data, licence);
			*with = oldChar; // Restore old char

			// It's not possible for a licence string to have two valid suffixes,
			// so we can return now, without looking through the rest of the suffixes.
			return search >= 0;
		}
	}

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
		struct LicenceTreeNode *node = malloc(sizeof(struct LicenceTreeNode));
		if(node != NULL) {
			node->type = LTNT_LICENCE;
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
	struct LicenceTreeNode *node = malloc(sizeof(struct LicenceTreeNode) + bufDataLen);
	if(node != NULL) {
		node->members = bufDataLen / sizeof(struct LicenceTreeNode*);
		memcpy(node->child, NODEBUFPTR(bufStart), bufDataLen);

		node->type = type;
		node->is_free = isFree;
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
