#!/bin/bash
#
# vrms-rpm - list non-free packages on an rpm-based Linux distribution
# Copyright (C) 2018 Artur "suve" Iwicki
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 3,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program (LICENCE.txt). If not, see <http://www.gnu.org/licenses/>.
#

print_usage() {
	echo "generate-config.sh: This script is not meant to be executed manually."
	echo "generate-config.sh: It is a helper script ran during \"make config\"."
}

default_licence_list=""
install_dir=""
licence_lists=""

if [ "$#" -eq 0 ]; then
	print_usage
	exit 1
fi

while getopts 'd:i:l:' OPTNAME; do
	case "$OPTNAME" in
		d)
			default_licence_list=$OPTARG
		;;
		
		i)
			install_dir=$OPTARG
		;;
		
		l)
			licence_lists=$OPTARG
		;;
		
		*)
			echo "Unknown option '$OPTNAME'"
			exit 1
		;;
	esac
done

if [ "$default_licence_list" == "" ] || [ "$install_dir" == "" ] || [ "$licence_lists" == "" ]; then
	print_usage
	exit 1
fi

# Converts the list from "a b c" to "'a', 'b', 'c'", nicely wrapped to 76 columns and then indented by four spaces (so it fits in 80 columns).
licence_lists=$(echo "$licence_lists" | sed -e 's|\([^ ]*\)|'"'\1\'"'|g' -e 's| |, |g' | fold --spaces --width 76 | tr $'\n' '$' | head --bytes=-1 | sed -e 's|\$|\\n    |g')

cat <<EOF
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
#ifndef VRMS_RPM_CONFIG_H
#define VRMS_RPM_CONFIG_H

#define INSTALL_DIR  "$install_dir"

#define ALL_LICENCE_LISTS  "$licence_lists"
#define DEFAULT_LICENCE_LIST  "$default_licence_list"

#endif
EOF
