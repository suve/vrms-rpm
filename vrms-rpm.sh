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


# List based on https://fedoraproject.org/wiki/Licensing:Main#Good_Licenses
good_licences=(
	"AFL"
	"AGPLv1"
	"AGPLv3"
	"AGPLv3+"
	"APSL 2.0"
	"Artistic"
	"ASL 1.0"
	"ASL 1.1"
	"ASL 2.0"
	"Boost"
	"BSD"
	"CC0"
	"CC-BY"
	"CC-BY-SA"
	"CDDL"
	"DMIT"
	"EPL"
	"GPL+"
	"GPLv2"
	"GPLv2+"
	"GPLv3"
	"GPLv3+"
	"GPL-3.0"
	"ImageMagick"
	"Interbase"
	"ISC"
	"JasPer"
	"Julius"
	"Knuth"
	"libtiff"
	"LGPLv2"
	"LGPLv2+"
	"LGPLv3"
	"LGPLv3+"
	"LLGPL"
	"LPPL"
	"MIT"
	"mplus"
	"MPLv1.0"
	"MPLv1.1"
	"MPLv2.0"
	"Nmap"
	"OFL"        # Open Font License
	"OpenLDAP"
	"OpenSSL"
	"OSL 1.0"
	"OSL 1.1"
	"OSL 2.0"
	"OSL 2.1"
	"OSL 3.0"
	"PHP"
	"Public domain"
	"Public Domain"
	"Python"
	"Qhull"
	"Romio"
	"SISSL"
	"SNIA"
	"SPL"
	"TCL"
	"TTWL"
	"Unicode"
	"Unlicense"
	"Vim"
	"W3C"
	"WTFPL"
	"wxWidgets"
	"zlib"
)

good_licences_argstring=""
for ((l=0; l<${#good_licences[@]}; ++l)); do
	good_licences_argstring="$good_licences_argstring	--regexp	${good_licences[$l]}"
done

function classify_package() {
	echo "$2" | grep --quiet --fixed-strings --word-regexp $good_licences_argstring
	
	if [[ "$?" -eq 0 ]]; then
		free[${#free[@]}]=$1
	else
		nonfree[${#nonfree[@]}]=$1
	fi
}

# For splitting the package list line-by-line
IFS="
"

packages=`rpm --all --query --queryformat '%{NAME}:%{LICENSE}\n'`
packages=($packages)

free=()
nonfree=()

# Will be used later when expanding $good_licences_argstring to separate args
IFS="	"

for ((i=0; i< ${#packages[@]}; ++i)); do
	line=${packages[$i]}
	name=${line%%:*}
	licence=${line##*:}
	
	classify_package "$name" "$licence"
done

echo "${#free[@]} free packages"
echo "${#nonfree[@]} nonfree packages"
