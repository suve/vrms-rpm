/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2024-2025 suve (a.k.a. Artur Frenszek-Iwicki)
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

#include "src/json.h"

static void encodeString(FILE *output, const char *str) {
	putc('"', output);

	while(1) {
		const unsigned char c = *str++;
		if(c >= ' ') switch(c) {
			case '\\': fprintf(output, "\\\\"); break;
			case '"': fprintf(output, "\\\""); break;
			default: putc(c, output);
		} else switch(c) {
			case '\0': goto exit;
			case '\t': fprintf(output, "\\t"); break;
			case '\n': fprintf(output, "\\n"); break;
			case '\r': fprintf(output, "\\r"); break;
			default: fprintf(output, "\\u00%02x", c);
		}
	}

exit:
	putc('"', output);
}

struct Element {
	char type;
	unsigned int children;
};

#define MAX_DEPTH 16

struct Document {
	FILE *output;
	int pretty;
	int depth;
	struct Element elems[MAX_DEPTH];
};
#define DOCUMENT(dest, source) struct Document *(dest) = ((struct Document*)(source));

static struct Document* doc_new(FILE *output, int pretty) {
	struct Document *doc = malloc(sizeof(struct Document));
	if(doc == NULL) return NULL;

	doc->output = output;
	doc->pretty = pretty;
	doc->depth = 0;

	doc->elems[0] = (struct Element){
		.type = '}',
		.children = 0,
	};

	putc('{', doc->output);
	return doc;
}

static void doc_addMember(struct Document *doc, const char *const key) {
	if(doc->elems[doc->depth].children > 0) putc(',', doc->output);
	doc->elems[doc->depth].children += 1;

	if(doc->pretty) {
		putc('\n', doc->output);
		for(int i = 0; i <= doc->depth; ++i) putc('\t', doc->output);
	}

	if(key != NULL) {
		encodeString(doc->output, key);
		putc(':', doc->output);

		if(doc->pretty) putc(' ', doc->output);
	}
}

static void doc_deepen(struct Document *doc, const char type) {
	putc(type, doc->output);
	doc->depth += 1;

	// Turn a closing bracket/brace into a closing one.
	// 0x5B: '[', 0x5C: '\\', 0x5D: ']'
	// 0x7B: '{', 0x7C: '|',  0x7D: '}'
	doc->elems[doc->depth].type = type + 2;
	doc->elems[doc->depth].children = 0;
}

static void doc_shallow(struct Document *doc) {
	if((doc->pretty) && (doc->elems[doc->depth].children > 0)) {
		putc('\n', doc->output);
		for(int i = 0; i < doc->depth; ++i) putc('\t', doc->output);
	}

	putc(doc->elems[doc->depth].type, doc->output);
	doc->depth -= 1;
}

void json_new(
	FILE *output,
	int pretty,
	JsonObjectCallback callback,
	void *userdata
) {
	struct Document *doc = doc_new(output, pretty);
	callback((struct JsonObject*)doc, userdata);
	doc_shallow(doc);
	free(doc);
}

void jsonObj_pushBool(
	struct JsonObject *obj,
	const char *const key,
	const int value
) {
	DOCUMENT(doc, obj);

	doc_addMember(doc, key);
	fprintf(doc->output, "%s", value ? "true": "false");
}

void jsonObj_pushInt(
	struct JsonObject *obj,
	const char *const key,
	const int value
) {
	DOCUMENT(doc, obj);

	doc_addMember(doc, key);
	fprintf(doc->output, "%d", value);
}

void jsonObj_pushStr(
	struct JsonObject *obj,
	const char *const key,
	const char *const value
) {
	DOCUMENT(doc, obj);

	doc_addMember(doc, key);
	encodeString(doc->output, value);
}

void jsonObj_pushArr(
	struct JsonObject *obj,
	const char *const key,
	JsonArrayCallback callback,
	void *userdata
) {
	DOCUMENT(doc, obj);

	doc_addMember(doc, key);
	doc_deepen(doc, '[');
	callback((struct JsonArray*)doc, userdata);
	doc_shallow(doc);
}

void jsonObj_pushObj(
	struct JsonObject *obj,
	const char *const key,
	JsonObjectCallback callback,
	void *userdata
) {
	DOCUMENT(doc, obj);

	doc_addMember(doc, key);
	doc_deepen(doc, '{');
	callback((struct JsonObject*)doc, userdata);
	doc_shallow(doc);
}

void jsonArr_pushStr(struct JsonArray *arr, const char *const value) {
	DOCUMENT(doc, arr);

	doc_addMember(doc, NULL);
	encodeString(doc->output, value);
}

void jsonArr_pushObj(
	struct JsonArray *arr,
	JsonObjectCallback callback,
	void *userdata
) {
	DOCUMENT(doc, arr);

	doc_addMember(doc, NULL);
	doc_deepen(doc, '{');
	callback((struct JsonObject*)doc, userdata);
	doc_shallow(doc);
}
