/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2018 Artur "suve" Iwicki
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

enum LicenceTreeNodeType {
	LTNT_LICENCE,
	LTNT_AND,
	LTNT_OR
};

struct LicenceTreeNode {
	enum LicenceTreeNodeType type;
	int is_free;
};

struct LicenceTreeNode_Licence {
	enum LicenceTreeNodeType type;
	int is_free;
	char *licence;
};

struct LicenceTreeNode_Joiner {
	enum LicenceTreeNodeType type;
	int is_free;
	int members;
	struct LicenceTreeNode *child[];
};


int licences_read(void);
void licences_free(void);

struct LicenceTreeNode* licence_classify(char *licence);

#endif
