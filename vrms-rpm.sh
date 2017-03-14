#!/bin/bash
#
# vrms-rpm - list non-free packages on an rpm-based Linux distribution
# Copyright (C) 2017 Artur "suve" Iwicki
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 3,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program (LICENCE.txt). If not, see <http://www.gnu.org/licenses/>.
#

free=()
nonfree=()

function classify_package() {
	good_licenses=(
		"AGPLv3"
		"AGPLv3+"
		"BSD"
		"CC0"
		"CC-BY"
		"CC-BY-SA"
		"GPL+"
		"GPLv2"
		"GPLv2+"
		"GPLv3"
		"GPL-3.0"
		"LGPLv2"
		"LGPLv2+"
		"LGPLv3"
		"MIT"
		"MPLv1.0"
		"MPLv1.1"
		"MPLv2.0"
		"Public domain"
		"Public Domain"
		"Unlicense"
		"WTFPL"
		"zlib"
	)
	
	for ((l=0; l<${#good_licenses[@]}; ++l)); do
		echo "$2" | grep --quiet --fixed-strings --word-regexp --regexp "${good_licenses[$l]}"
		
		if [[ "$?" -eq 0 ]]; then
			free[${#free[@]}]=$1
			return
		fi
	done
	
	nonfree[${#nonfree[@]}]=$1
}

IFS="
"

packages=`rpm --all --query --queryformat '%{NAME}:%{LICENSE}\n'`
packages=($packages)

for ((i=0; i< ${#packages[@]}; ++i)); do
	line=${packages[$i]}
	name=${line%%:*}
	license=${line##*:}
	
	classify_package $name $license
done

echo "${#free[@]} free packages"
echo "${#nonfree[@]} nonfree packages"
