/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2024 suve (a.k.a. Artur Frenszek-Iwicki)
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
		const char c = *str++;
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

struct JsonPrinter {
	FILE *output;
	int pretty;
	int members;
	int depth;
};
#define PRINTER(dest, source) struct JsonPrinter *(dest) = ((struct JsonPrinter*)(source));

static struct JsonPrinter* printer_new(FILE *output, int pretty, int depth, char type) {
	struct JsonPrinter *printer = malloc(sizeof(struct JsonPrinter));
	if(printer == NULL) return NULL;

	printer->output = output;
	printer->pretty = pretty;
	printer->depth = depth;
	printer->members = 0;

	putc(type, printer->output);
	return printer;
}

static struct JsonPrinter* printer_newChild(struct JsonPrinter *parent, char type) {
	return printer_new(parent->output, parent->pretty, parent->depth + 1, type);
}

static void printer_indent(struct JsonPrinter *printer) {
	putc('\n', printer->output);

	int depth = printer->depth;
	while(depth --> 0) putc('\t', printer->output);
}

static void printer_free(struct JsonPrinter *printer, char type) {
	if(printer->pretty) printer_indent(printer);
	putc(type, printer->output);
	free(printer);
}

static void printer_addMember(struct JsonPrinter *printer, const char *const key) {
	if(printer->members) {
		putc(',', printer->output);
	} else {
		printer->members = 1;
	}

	if(printer->pretty) printer_indent(printer);

	if(key != NULL) {
		encodeString(printer->output, key);
		putc(':', printer->output);

		if(printer->pretty) putc(' ', printer->output);
	}
}

struct JsonObject* jsonObj_open(FILE *output, int pretty) {
	struct JsonPrinter *printer = printer_new(output, pretty, 0, '{');
	return (struct JsonObject*)printer;
}

void jsonObj_pushInt(struct JsonObject *obj, const char *const key, const int value) {
	PRINTER(printer, obj);

	printer_addMember(printer, key);
	fprintf(printer->output, "%d", value);
}

void jsonObj_pushStr(struct JsonObject *obj, const char *const key, const char *const value) {
	PRINTER(printer, obj);

	printer_addMember(printer, key);
	encodeString(printer->output, value);
}

struct JsonArray*  jsonObj_pushArr(struct JsonObject *obj, const char *const key) {
	PRINTER(printer, obj);

	printer_addMember(printer, key);
	return (struct JsonArray*)printer_newChild(printer, '[');
}

struct JsonObject* jsonObj_pushObj(struct JsonObject *obj, const char *const key) {
	PRINTER(printer, obj);

	printer_addMember(printer, key);
	return (struct JsonObject*)printer_newChild(printer, '{');
}

void jsonObj_close(struct JsonObject *obj) {
	PRINTER(printer, obj);
	printer_free(printer, '}');
}

void jsonArr_pushStr(struct JsonArray *arr, const char *const value) {
	PRINTER(printer, arr);

	printer_addMember(printer, NULL);
	encodeString(printer->output, value);
}

struct JsonObject* jsonArr_pushObj(struct JsonArray *arr) {
	PRINTER(printer, arr);

	printer_addMember(printer, NULL);
	return (struct JsonObject*)printer_newChild(printer, '{');
}

void jsonArr_close(struct JsonArray *arr) {
	PRINTER(printer, arr);
	printer_free(printer, ']');
}
