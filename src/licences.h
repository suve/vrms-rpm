/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018, 2022-2023 suve (a.k.a. Artur Frenszek-Iwicki)
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

#ifndef VRMS_RPM_LICENCES_H
#define VRMS_RPM_LICENCES_H

struct LicenceData {
	struct ReBuffer *list;
	struct ChainBuffer *buffer;
};

enum LicenceTreeNodeType {
	LTNT_LICENCE,
	LTNT_AND,
	LTNT_OR
};

struct LicenceTreeNode {
	enum LicenceTreeNodeType type;
	int is_free;

	union {
		char *licence;
		struct {
			unsigned int members;
			struct LicenceTreeNode *child[];
		};
	};
};

extern struct LicenceData* licences_read(void);
extern int licences_find(const struct LicenceData *data, const char *licence);
extern void licences_free(struct LicenceData *data);

extern void licence_freeTree(struct LicenceTreeNode *node);

#endif
