/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2023-2024 suve (a.k.a. Artur Frenszek-Iwicki)
 * Copyright (C) 2023 Marcin "dextero" Radomski
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

#include "src/buffers.h"
#include "src/classifiers.h"
#include "src/licences.h"
#include "src/stringutils.h"

struct SpdxClassifier {
	struct LicenceClassifier interface;
	const struct LicenceData *data;
	struct ReBuffer *nodeBuf;
	int lenient;
};

#define LOWERCASE_SHIFT ('a' - 'A')

enum LetterCase {
	LC_LOWERCASE = -1,
	LC_NONE = 0,
	LC_UPPERCASE = +1
};

/*
 * Match a letter against another, while maintaining case-sensitivity rules.
 * - If the classifier is set to lenient mode, match_case is disregarded
 *   and lowercase and uppercase letters can be fixed freely. ("lIKE THiS")
 * - If classifier is set to strict mode:
 *   - If match_case is LC_NONE, then both lowercase and uppercase letters
 *     are considered a match.
 *   - If match_case is LC_LOWERCASE, then only lowercase letters match.
 *   - If match_case is LC_UPPERCASE, then only uppercase letters match.
 */
static int match_letter(
	const struct SpdxClassifier *const self,
	const char letter,
	char match_against,
	enum LetterCase *const match_case
) {
	if(letter == match_against) {
		if((!self->lenient) && (*match_case == LC_LOWERCASE)) {
			*match_case = LC_NONE;
			return 0;
		}
		*match_case = LC_UPPERCASE;
		return 1;
	}

	if(letter == match_against + LOWERCASE_SHIFT) {
		if((!self->lenient) && (*match_case == LC_UPPERCASE)) {
			*match_case = LC_NONE;
			return 0;
		}
		*match_case = LC_LOWERCASE;
		return 1;
	}

	return 0;
}

enum WithSearchState {
	WSS_SEARCHING,
	WSS_MATCH_START,
	WSS_MATCHED_W,
	WSS_MATCHED_I,
	WSS_MATCHED_T,
	WSS_MATCHED_H,
};

// TODO: Replace these ugly ternaries with something less offensive.
static char* find_WITH_operator(struct SpdxClassifier *self, char *licence) {
	enum LetterCase ltrcase = LC_NONE;
	enum WithSearchState state = WSS_SEARCHING;
	for(char c = *licence; c != '\0'; c = *(++licence)) {
		switch(state) {
			case WSS_SEARCHING:
				if(c == ' ') state = WSS_MATCH_START;
				break;
			case WSS_MATCH_START:
				state = match_letter(self, c, 'W', &ltrcase) ?
							WSS_MATCHED_W : (c == ' ') ? WSS_MATCH_START : WSS_SEARCHING;
				break;
			case WSS_MATCHED_W:
				state = match_letter(self, c, 'I', &ltrcase) ?
							WSS_MATCHED_I : (c == ' ') ? WSS_MATCH_START : WSS_SEARCHING;
				break;
			case WSS_MATCHED_I:
				state = match_letter(self, c, 'T', &ltrcase) ?
							WSS_MATCHED_T : (c == ' ') ? WSS_MATCH_START : WSS_SEARCHING;
				break;
			case WSS_MATCHED_T:
				state = match_letter(self, c, 'H', &ltrcase) ?
							WSS_MATCHED_H : (c == ' ') ? WSS_MATCH_START : WSS_SEARCHING;
				break;
			case WSS_MATCHED_H:
				if(c == ' ') return licence - 5;
				state = WSS_SEARCHING;
				break;
		}
	}
	return NULL;
}

static int is_free(struct SpdxClassifier *self, char *licence) {
	if(licences_find(self->data, licence) >= 0) return 1;

	// SPDX allows specifying additional rights ("licensing exceptions")
	// through the use of the "WITH" operator.
	char* with = find_WITH_operator(self, licence);
	if(with != NULL) {
		// The "WITH" operator can be preceded by any number of spaces.
		size_t withPos = (with - licence);
		while((withPos > 0) && (licence[withPos - 1] == ' ')) --withPos;

		with = licence + withPos;
		*with = '\0';
	}

	// SPDX allows specifying "or later version" by tacking a "+" to the licence name.
	const size_t len = strlen(licence);
	size_t plusPos = len;
	char plusChar = 0;
	if((len > 0) && (licence[len - 1] == '+')) {
		plusPos = len - 1;
		plusChar = '+';

		// The SPDX spec mandates no whitespace between LicenceName and '+'.
		// If running in lenient mode, check and remove any spaces.
		if(self->lenient) {
			while((plusPos > 0) && (licence[plusPos - 1] == ' ')) {
				plusPos -= 1;
				plusChar = ' ';
			}
		}
		// Trim the string to desired length.
		licence[plusPos] = '\0';
	}

	// If we didn't find the "WITH" operator nor the "+" operator, bail out early.
	if((plusPos >= len) && (with == NULL)) return 0;

	// Try matching the licence name, stripped from the +/WITH parts, again.
	int found = licences_find(self->data, licence);

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

static enum LicenceTreeNodeType detect_type(struct SpdxClassifier *self, const char *licence) {
	int found_and = 0;
	int found_or = 0;

	enum LetterCase ltrcase = LC_NONE;
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
				if(match_letter(self, c, 'A', &ltrcase))
					state = DT_FOUND_AND_A;
				else if(match_letter(self, c, 'O', &ltrcase))
					state = DT_FOUND_OR_O;
				else if(c != ' ')
					state = DT_SEARCHING;
			break;

			case DT_FOUND_AND_A:
				state = match_letter(self, c, 'N', &ltrcase) ? DT_FOUND_AND_N : DT_SEARCHING;
			break;

			case DT_FOUND_AND_N:
				state = match_letter(self, c, 'D', &ltrcase) ? DT_FOUND_AND_D : DT_SEARCHING;
			break;

			case DT_FOUND_AND_D:
				if(c == ' ') found_and = 1;
				state = DT_SEARCHING;
			break;

			case DT_FOUND_OR_O:
				state = match_letter(self, c, 'R', &ltrcase) ? DT_FOUND_OR_R : DT_SEARCHING;
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

/*
 * Helper function for spdx_classify(), should not be called from other places.
 *
 * Check if text[0] is followed by "AND"/"OR", and then either a space (' ') or an opening parenthesis ('(').
 * Returns 0 if match is found, or number of characters that can be skipped if it isn't.
 */
static int skip_non_joiner(struct SpdxClassifier *self, const char *const text, const enum LicenceTreeNodeType type) {
	enum LetterCase ltrcase = LC_NONE;
	if(type == LTNT_AND) {
		if(!match_letter(self, text[1], 'A', &ltrcase)) return 1;
		if(!match_letter(self, text[2], 'N', &ltrcase)) return 2;
		if(!match_letter(self, text[3], 'D', &ltrcase)) return 3;
		if((text[4] != ' ') && (text[4] != '(')) return 4;
	} else {
		if(!match_letter(self, text[1], 'O', &ltrcase)) return 1;
		if(!match_letter(self, text[2], 'R', &ltrcase)) return 2;
		if((text[3] != ' ') && (text[3] != '(')) return 3;
	}
	return 0;
}

static struct LicenceTreeNode* append(struct SpdxClassifier *self, char *licence, enum LicenceTreeNodeType rootType, int *rootIsFree);
static void cleanup(struct SpdxClassifier *self, size_t bufStart);

// Helper macro: append a child node, or bail out if an error occurs
#define try_append(lic, type, isFree) \
	do { \
		if(append(self, (lic), (type), (isFree)) == NULL) { \
			cleanup(self, bufStart); \
			return NULL; \
		} \
	} while(0)

// Helper macro: make a pointer to a LicenceTreeNode from the value located in the nodeBuf at given offset
#define NODEBUFPTR(offset) ((struct LicenceTreeNode*)(((char*)self->nodeBuf->data) + (offset)))

static struct LicenceTreeNode* spdx_classify(struct LicenceClassifier *class, char *licence) {
	struct SpdxClassifier *self = (struct SpdxClassifier*)class;

	enum LicenceTreeNodeType type = detect_type(self, licence);
	if(type == LTNT_LICENCE) {
		/*
		 * If the detected type is LICENCE, but we've got an opening parenthesis,
		 * then that means one of the two following scenarios:
		 * 1. The whole string is parenthesized (e.g. "(text)" instead of "text")
		 * 2. The licence string contains parenthesized text (e.g. "(Very) Bad Licence")
		 * If we're in scenario 1, strip the parentheses and try again.
		 */
		if(*licence == '(') {
			char *closingParen = find_closing_paren(licence);
			if((closingParen != NULL) && (*(closingParen + 1) == '\0')) {
				++licence; // Advance one char to skip the opening paren
				*closingParen = '\0'; // Shorten the string to remove the closing paren
				return spdx_classify(class, licence);
			}
		}

		struct LicenceTreeNode *node = malloc(sizeof(struct LicenceTreeNode));
		if(node != NULL) {
			node->type = LTNT_LICENCE;
			node->licence = licence;
			node->is_free = is_free(self, licence);
		}
		return node;
	}

	const size_t bufStart = self->nodeBuf->used;
	int isFree = (type == LTNT_AND) ? 1 : 0;

	char *s = licence;
	while(1) {
		while((*s != '\0') && (*s != '(') && (*s != ')') && (*s != ' ')) ++s;
		if(*s == '\0') {
			if(s > licence) try_append(licence, type, &isFree);
			break;
		}
		if(*s == '(') {
			char* closingParen = find_closing_paren(s);
			s = (closingParen != NULL) ? closingParen : (s + 1);
			continue;
		}

		// Not a NUL byte and not an opening paren - must be closing paren or space.
		// Check if this is followed by AND/OR.
		const int skip = skip_non_joiner(self, s, type);
		if(skip > 0) {
			s += skip;
			continue;
		}

		if(*s == ')') ++s;
		*s = '\0';

		try_append(licence, type, &isFree);

		while((*s != ' ') && (*s != '(')) {
			*s = '\0';
			++s;
		}
		licence = s;
	}

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

static struct LicenceTreeNode* append(struct SpdxClassifier *self, char *licence, enum LicenceTreeNodeType rootType, int *rootIsFree) {
	licence = trim(licence, NULL);

	struct LicenceTreeNode *child = spdx_classify(&self->interface, licence);
	if(child == NULL) return NULL;

	struct ReBuffer *nodeBuf = self->nodeBuf;
	if(rebuf_append(nodeBuf, &child, sizeof(struct LicenceTreeNode*)) == NULL) {
		licence_freeTree(child);
		return NULL;
	}

	*rootIsFree = (rootType == LTNT_AND) ? (*rootIsFree && child->is_free) : (*rootIsFree || child->is_free);
	return child;
}

static void cleanup(struct SpdxClassifier *self, size_t bufStart) {
	// Free any child nodes allocated so far.
	for(size_t pos = bufStart; bufStart < self->nodeBuf->used; pos += sizeof(struct LicenceTreeNode*)) {
		licence_freeTree(NODEBUFPTR(pos));
	}

	// Re-wind the nodeBuffer so we can pretend we never appended anything.
	self->nodeBuf->used = bufStart;
}

static void spdx_free(struct LicenceClassifier *class) {
	if(class != NULL) {
		struct SpdxClassifier *self = (struct SpdxClassifier*)class;
		if(self->nodeBuf != NULL) {
			rebuf_free(self->nodeBuf);
			self->nodeBuf = NULL;
		}
		free(self);
	}
}

struct LicenceClassifier* classifier_newSPDX(const struct LicenceData *data, int lenient) {
	struct SpdxClassifier *self = malloc(sizeof(struct SpdxClassifier));
	if(self == NULL) return NULL;

	struct ReBuffer *nodeBuf = rebuf_init(1024);
	if(nodeBuf == NULL) {
		free(self);
		return NULL;
	}

	self->data = data;
	self->nodeBuf = nodeBuf;
	self->lenient = lenient;

	self->interface.classify = &spdx_classify;
	self->interface.free = &spdx_free;
	return &self->interface;
}
