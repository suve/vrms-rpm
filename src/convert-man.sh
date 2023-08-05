#!/bin/sh
#
# vrms-rpm - list non-free packages on an rpm-based Linux distribution
# Copyright (C) 2018, 2023 suve (a.k.a. Artur Frenszek-Iwicki)
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

set -eu

print_usage() {
	echo "convert-man.sh: This script is not meant to be executed manually." >&2
	echo "convert-man.sh: It is a helper script ran during \"make build\"." >&2
}

# TODO: Would be great to match and replace everything at once instead of calling this multiple times.
replace_string() {
	cp "${1}" "${1}.bak"
	(
		while read -r line; do
			if [ "${line}" = "${2}" ]; then
				echo "${3}"
			else
				echo "${line}"
			fi
		done
	) < "${1}.bak" > "${1}"
	rm "${1}.bak"
}

default_grammar=""
default_licence_list=""
install_prefix=""
licence_lists=""
file=""

if [ "$#" -eq 0 ]; then
	print_usage
	exit 1
fi

while getopts 'd:g:f:l:p:' OPTNAME; do
	case "${OPTNAME}" in
		d)
			default_licence_list="${OPTARG}"
		;;
		
		f)
			file="${OPTARG}"
		;;

		g)
			default_grammar="${OPTARG}"
		;;

		l)
			licence_lists="${OPTARG}"
		;;
		
		p)
			install_prefix="${OPTARG}"
		;;
		
		*)
			echo "Unknown option '${OPTNAME}'"
			exit 1
		;;
	esac
done

if [ -z "${default_grammar}" ] || [ -z "${default_licence_list}" ] || [ -z "${file}" ] || [ -z "${install_prefix}" ] || [ -z "${licence_lists}" ]; then
	print_usage
	exit 1
fi

# Converts the list from "a b c" to "'\fIa\fR', '\fIb\fR', '\fIc\fR'", so it looks nice when the man page is viewed
licence_lists=$(echo "${licence_lists} .br" | sed -e 's|\([^ ]*\) |.br\n\\(bu \1\n|g')

replace_string "${file}" "__BUNDLED_LICENCE_LISTS__" "${licence_lists}"
replace_string "${file}" "__DEFAULT_LICENCE_LIST__" "\"\\fI${default_licence_list}\\fR\"."
replace_string "${file}" "__DEFAULT_GRAMMAR__" "\"\\fI${default_grammar}\\fR\"."
