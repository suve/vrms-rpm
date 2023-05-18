/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2023 suve (a.k.a. Artur Frenszek-Iwicki)
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
#include <string.h>

#include "buffers.h"
#include "classifiers.h"
#include "licences.h"
#include "stringutils.h"

static int is_free(const struct LicenceData *data, char *licence) {
	if(licences_find(data, licence) >= 0) return 1;

	// SPDX allows specifying additional rights ("licensing exceptions")
	// through the use of the "WITH" operator.
	// TODO: Allow this to be case-insensitive.
	//       The spec mandates case-sensitivity, but we want to be lenient.
	char* with = strstr(licence, " WITH ");
	if(with != NULL) *with = '\0';

	// SPDX allows specifying "or later version" by tacking a "+" to the licence name.
	const size_t len = strlen(licence);
	size_t plusPos = len;
	char plusChar;
	if((len > 0) && (licence[len - 1] == '+')) {
		plusPos = len - 1;
		plusChar = '+';
		// The SPDX spec mandates no whitespace between LicenceName and '+',
		// but we want to be lenient.
		while((plusPos > 0) && (licence[plusPos - 1] == ' ')) {
			plusPos -= 1;
			plusChar = ' ';
		}
	}

	// If we didn't find the "WITH" operator nor the "+" operator, bail out early.
	if((plusPos >= len) && (with == NULL)) return 0;

	// Try matching the licence name, stripped from the +/WITH parts, again.
	int found = licences_find(data, licence);

	// Restore the licence string to its original shape.
	if(plusPos < len) licence[plusPos] = plusChar;
	if(with != NULL) *with = ' ';

	return found >= 0;
}

enum DetectionState {
	DT_SEARCHING,
	DT_MATCH_START,
	DT_FOUND_AND_A,
	DT_FOUND_AND_N,
	DT_FOUND_AND_D,
	DT_FOUND_OR_O,
	DT_FOUND_OR_R,
};

static enum LicenceTreeNodeType detect_type(const char *licence) {
	int found_and = 0;
	int found_or = 0;

	enum DetectionState state = DT_SEARCHING;
	for(char c = *licence; c != '\0'; c = *(++licence)) {
		if(c == '(') {
			// Need to check this here, since this if() will get executed before the switch()
			if(state == DT_FOUND_AND_D)
				found_and = 1;
			else if(state == DT_FOUND_OR_R)
				found_or = 1;

			const char *closingParen = find_closing_paren(licence);
			if(closingParen != NULL) {
				licence = closingParen;
				state = DT_MATCH_START;
			} else {
				state = DT_SEARCHING;
			}
			continue;
		}

		switch(state) {
			case DT_SEARCHING:
				if(c == ' ') state = DT_MATCH_START;
			break;

			case DT_MATCH_START:
				if((c == 'a') || (c == 'A'))
					state = DT_FOUND_AND_A;
				else if((c == 'o') || (c == 'O'))
					state = DT_FOUND_OR_O;
				else if(c != ' ')
					state = DT_SEARCHING;
			break;

			case DT_FOUND_AND_A:
				state = ((c == 'n') || (c == 'N')) ? DT_FOUND_AND_N : DT_SEARCHING;
			break;

			case DT_FOUND_AND_N:
				state = ((c == 'd') || (c == 'D')) ? DT_FOUND_AND_D : DT_SEARCHING;
			break;

			case DT_FOUND_AND_D:
				if(c == ' ') found_and = 1;
				state = DT_SEARCHING;
			break;

			case DT_FOUND_OR_O:
				state = ((c == 'r') || (c == 'R')) ? DT_FOUND_OR_R : DT_SEARCHING;
			break;

			case DT_FOUND_OR_R:
				if(c == ' ') found_or = 1;
				state = DT_SEARCHING;
			break;
		}
	}

	if(found_or) return LTNT_OR;
	if(found_and) return LTNT_AND;
	return LTNT_LICENCE;
}

static struct LicenceTreeNode* append(struct LicenceClassifier *class, char *licence, enum LicenceTreeNodeType rootType, int *rootIsFree);

// Helper macro: make a pointer to a LicenceTreeNode from the value located in the nodeBuf at given offset
#define NODEBUFPTR(offset) ((struct LicenceTreeNode*)(((char*)nodeBuf->data) + (offset)))

static struct LicenceTreeNode* spdx_classify(struct LicenceClassifier *class, char *licence) {
	enum LicenceTreeNodeType type = detect_type(licence);
	if(type == LTNT_LICENCE) {
		// If the detected type is LICENCE, but we've got an opening parenthesis,
		// then that means the whole string is parenthesised (e.g. "(text)" instead of "text").
		if(*licence == '(') {
			// Advance one char to skip the opening paren
			++licence;
			// Check the end of the string for the closing paren
			const size_t length = strlen(licence);
			if(licence[length - 1] == ')') licence[length - 1] = '\0';
			// Both parentheses have been stripped - try again.
			return spdx_classify(class, licence);
		}

		struct LicenceTreeNode *node = malloc(sizeof(struct LicenceTreeNode));
		if(node != NULL) {
			node->type = LTNT_LICENCE;
			node->licence = licence;
			node->is_free = is_free(class->data, licence);
		}
		return node;
	}

	struct ReBuffer *nodeBuf = class->private;
	const size_t bufStart = nodeBuf->used;
	int isFree = (type == LTNT_AND) ? 1 : 0;

	char *s = licence;
	while(1) {
		while((*s != '\0') && (*s != '(') && (*s != ')') && (*s != ' ')) ++s;
		if(*s == '\0') {
			// TODO: Handle errors here
			if(s > licence) append(class, licence, type, &isFree);
			break;
		}
		if(*s == '(') {
			char* closingParen = find_closing_paren(s);
			s = (closingParen != NULL) ? closingParen : (s + 1);
			continue;
		}

		// Not a NUL byte and not an opening paren - must be closing paren or space.
		// Try matching for AND/OR.
		int match = 1;
		if(type == LTNT_AND) {
			match = match && ((s[1] == 'A') || (s[1] == 'a'));
			match = match && ((s[2] == 'N') || (s[2] == 'n'));
			match = match && ((s[3] == 'D') || (s[3] == 'd'));
			match = match && ((s[4] == ' ') || (s[4] == '('));
		} else {
			match = match && ((s[1] == 'O') || (s[1] == 'o'));
			match = match && ((s[2] == 'R') || (s[2] == 'r'));
			match = match && ((s[3] == ' ') || (s[3] == '('));
		}
		if(!match) {
			++s; // TODO: Skip more than one char, based on failed match length
			continue;
		}

		if(*s == ')') ++s;
		*s = '\0';

		// TODO: Handle errors here
		append(class, licence, type, &isFree);

		while((*s != ' ') && (*s != '(')) {
			*s = '\0';
			++s;
		}
		licence = s;
	}

	const size_t bufDataLen = nodeBuf->used - bufStart;
	struct LicenceTreeNode *node = malloc(sizeof(struct LicenceTreeNode) + bufDataLen);
	if(node != NULL) {
		node->members = bufDataLen / sizeof(struct LicenceTreeNode*);
		memcpy(node->child, NODEBUFPTR(bufStart), bufDataLen);

		node->type = type;
		node->is_free = isFree;
	}

	nodeBuf->used = bufStart;
	return node;
}

static struct LicenceTreeNode* append(struct LicenceClassifier *class, char *licence, enum LicenceTreeNodeType rootType, int *rootIsFree) {
	licence = trim(licence, NULL);

	struct LicenceTreeNode *child = spdx_classify(class, licence);
	if(child == NULL) return NULL;

	struct ReBuffer *nodeBuf = class->private;
	if(rebuf_append(nodeBuf, &child, sizeof(struct LicenceTreeNode*)) == NULL) {
		licence_freeTree(child);
		return NULL;
	}

	*rootIsFree = (rootType == LTNT_AND) ? (*rootIsFree && child->is_free) : (*rootIsFree || child->is_free);
	return child;
}

static void spdx_free(struct LicenceClassifier *self) {
	if(self != NULL) {
		if(self->private != NULL) {
			rebuf_free(self->private);
			self->private = NULL;
		}
		free(self);
	}
}

struct LicenceClassifier* classifier_newSPDX(const struct LicenceData *data) {
	struct LicenceClassifier *self = malloc(sizeof(struct LicenceClassifier));
	if(self == NULL) return NULL;

	struct ReBuffer *nodeBuf = rebuf_init();
	if(nodeBuf == NULL) {
		free(self);
		return NULL;
	}

	self->data = data;
	self->private = nodeBuf;
	self->classify = &spdx_classify;
	self->free = &spdx_free;

	return self;
}
