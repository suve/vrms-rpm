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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program (LICENCE.txt). If not, see <http://www.gnu.org/licenses/>.
#

# Variable containing program name - in case we need to change it someday
prog_name="vrms-rpm"

# Function for printing translated printf-messages
function printmsg() {
	local translated=`gettext -s "msg_$1"`
	shift

	printf "$translated\n" $@
}

# Initialize gettext
export TEXTDOMAINDIR="/usr/share/suve/$prog_name/locale/"
export TEXTDOMAIN="vrms-rpm"

# Treat unitiliased variables as errors
set -u       

# Set initial option values
ascii="0"
explain="0"
list="nonfree"

# Read options
while [[ "$#" -gt 0 ]]; do
	case "$1" in
		--ascii)
			ascii="1"
		;;
		
		--explain)
			explain="1"
		;;
		
		--help)
			printmsg "help_usage"
			
			echo "  --ascii"
			printmsg "help_option_ascii"
			
			echo "  --explain"
			printmsg "help_option_explain"
			
			echo "  --help"
			printmsg "help_option_help"
  			
			echo "  --list <none,free,nonfree,all>"
			printmsg "help_option_list"
			
			echo "  --version"
			printmsg "help_option_version"
			
			exit
		;;
		
		--list)
			if [[ "$#" -eq 1 ]]; then
				printmsg "list_noargument" "$prog_name"
				exit 1
			fi
			if [ "$2" != "none" ] && [ "$2" != "free" ] && [ "$2" != "nonfree" ] && [ "$2" != "non-free" ] && [ "$2" != "all" ]; then
				printmsg "list_badargument" "$prog_name"
				exit 1
			fi
			
			list=$2
			shift
		;;
		
		--version)
			echo "$prog_name v.1.1 by suve"
			exit
		;;

		*)
			printmsg "unknown_option" "$prog_name" "$1"
		;;
	esac
	
	shift
done


# List based on https://fedoraproject.org/wiki/Licensing:Main#Good_Licenses
good_licences=(
	"AAL"
	"Abstyles"
	"Adobe"
	"ADSL"
	"AFL"
	"Afmparse"
	"AGPLv1"
	"AGPLv3"
	"AGPLv3+"
	"AMDPLPA"
	"AML"
	"AMPAS BSD"
	"AMS"
	"APAFML"
	"App-s2p"
	"APSL 2.0"
	"ARL"
	"Arphic"
	"Artistic 2.0"
	"Artistic clarified"
	"ASL 1.0"
	"ASL 1.1"
	"ASL 2.0"
	"Baekmuk"
	"Bahyph"
	"Barr"
	"Beerware"
	"BeOpen"
	"Bibtex"
	"Bitstream Vera"
	"BitTorrent"
	"Boost"
	"Borceux"
	"BSD"
	"CATOSL"
	"CC0"
	"CC-BY"
	"CC-BY-SA"
	"CDDL"
	"CDL"
	"CeCILL"
	"CeCILL-B"
	"CeCILL-C"
	"Charter"
	"CNRI"
	"Condor"
	"Copyright only"
	"CPAL"
	"CPL"
	"CRC32"
	"Crossword"
	"Crystal Stacker"
	"Cube"
	"diffmark"
	"DMIT"
	"DOC"
	"Dotseqn"
	"DoubleStroke"
	"DSDP"
	"DSL"
	"dvipdfm"
	"DWPL"
	"ec"
	"ECL 1.0"
	"ECL 2.0"
	"eCos"
	"EFL 2.0"
	"eGenix"
	"Elvish"
	"Entessa"
	"EPICS"
	"EPL"
	"ERPL"
	"EU Datagrid"
	"EUPL 1.1"
	"Eurosym"
	"Fair"
	"FBSDDL"
	"Free Art"
	"FSFUL"
	"FSFULLR"
	"FTL"
	"GeoGratis"
	"GFDL"
	"Giftware"
	"GL2PS"
	"Glide"
	"Glulxe"
	"gnuplot"
	"GPL"
	"GPL+"
	"GPLv1"
	"GPLv2"
	"GPLv2+"
	"GPLv3"
	"GPLv3+"
	"Green OpenMusic"
	"HaskellReport"
	"Hershey"
	"HSRL"
	"IBM"
	"IEEE"
	"IJG"
	"ImageMagick"
	"iMatix"
	"Imlib2"
	"Intel ACPI"
	"Interbase"
	"IPA"
	"ISC"
	"Jabber"
	"JasPer"
	"JPython"
	"Julius"
	"Knuth"
	"Latex2e"
	"LBNL BSD"
	"LDPL"
	"Leptonica"
	"LGPL-2.0"
	"LGPLv2"
	"LGPLv2+"
	"LGPLv3"
	"LGPLv3+"
	"Lhcyr"
	"Liberation"
	"libtiff"
	"LLGPL"
	"Logica"
	"LOSLA"
	"LPL"
	"LPPL"
	"Lucida"
	"MakeIndex"
	"mecab-ipadic"
	"MgOpen"
	"midnight"
	"MirOS"
	"MIT"
	"MITNFA"
	"mod_macro"
	"Motosoto"
	"mplus"
	"MPLv1.0"
	"MPLv1.1"
	"MPLv2.0"
	"MS-PL"
	"MS-RL"
	"MTLL"
	"Mup"
	"Naumen"
	"NCSA"
	"NetCDF"
	"Netscape"
	"Newmat"
	"Newsletr"
	"NGPL"
	"NLPL"
	"Nmap"
	"Nokia"
	"NOSL"
	"Noweb"
	"OAL"
	"OFL"
	"OFSFDL"
	"OGL"
	"OML"
	"OpenLDAP"
	"OpenPBS"
	"Open Publication"
	"OpenSSL"
	"OReilly"
	"OSL 1.0"
	"OSL 1.1"
	"OSL 2.0"
	"OSL 2.1"
	"OSL 3.0"
	"Par"
	"Phorum"
	"PHP"
	"PlainTeX"
	"Plexus"
	"PostgreSQL"
	"psfrag"
	"psutils"
	"PTFL"
	"pubkey"
	"Public domain"
	"Public Domain"
	"Public use"
	"Public Use"
	"Punknova"
	"Python"
	"Qhull"
	"QPL"
	"Rdisc"
	"REX"
	"RiceBSD"
	"Romio"
	"RPSL"
	"Rsfs"
	"Ruby"
	"Saxpath"
	"SCEA"
	"SCRIP"
	"Sendmail"
	"Sequence"
	"SISSL"
	"Sleepycat"
	"SLIB"
	"SNIA"
	"softSurfer"
	"SPL"
	"STIX"
	"STMPL"
	"SWL"
	"TCGL"
	"TCL"
	"Teeworlds"
	"TGPPL"
	"Threeparttable"
	"TMate"
	"Tolua"
	"TORQUEv1.1"
	"TOSL"
	"TPDL"
	"TPL"
	"TTWL"
	"UCAR"
	"UCD"
	"Unicode"
	"Unlicense"
	"Utopia"
	"Verbatim"
	"Vim"
	"VNLSL"
	"VOSTROM"
	"VSL"
	"W3C"
	"Wadalab"
	"Webmin"
	"Wsuipa"
	"WTFPL"
	"wxWidgets"
	"XANO"
	"Xerox"
	"xinetd"
	"xpp"
	"XSkat"
	"YPLv1.1"
	"Zed"
	"Zend"
	"zlib"
	"ZPLv1.0"
	"ZPLv2.0"
	"ZPLv2.1"
)

# Prepare a string composed of the good licence names.
# It will later be expanded into arguments to be consumed by grep
good_licences_argstring=""
for ((l=0; l<${#good_licences[@]}; ++l)); do
	good_licences_argstring="$good_licences_argstring	--regexp	${good_licences[$l]}"
done

function classify_package() {
	local push_value
	if [[ "$explain" -eq 1 ]]; then
		push_value="$1: $2"
	else
		push_value="$1"
	fi
	
	
	echo "$2" | grep --quiet --fixed-strings --word-regexp $good_licences_argstring
	
	if [[ "$?" -eq 0 ]]; then
		free[${#free[@]}]="$push_value"
	else
		nonfree[${#nonfree[@]}]="$push_value"
	fi
}

# For splitting the package list line-by-line
IFS="
"

# In Fedora, package names cannot contain the ":" character, so it's safe to use as delimiter
packages=`rpm --all --query --queryformat '%{NAME}:%{LICENSE}\n' | sort`
packages=($packages)

free=()
nonfree=()

# Will be used later when expanding $good_licences_argstring to separate args
IFS="	"

for ((i=0; i< ${#packages[@]}; ++i)); do
	line=${packages[$i]}
	
	# Remove the longest :* pattern from end of $line = everything after first colon.
	name=${line%%:*}
	# Remove the shortest *: pattern from beginning of $line = everything up to the first colon.
	licence=${line#*:}
	
	classify_package "$name" "$licence"
done


total_free=${#free[@]}
total_nonfree=${#nonfree[@]}
total=`expr $total_free + $total_nonfree`
percentage_nonfree=`expr $total_nonfree '*' 100 / $total`


printmsg "total_free" $total_free
if [ "$list" == "free" ] || [ "$list" == "all" ]; then
	for ((p=0; p<$total_free; ++p)); do
		echo " - ${free[$p]}"
	done
fi


printmsg "total_nonfree" $total_nonfree
if [ "$list" == "nonfree" ] || [ "$list" = "non-free" ] || [ "$list" == "all" ]; then
	for ((p=0; p<$total_nonfree; ++p)); do
		echo " - ${nonfree[$p]}"
	done
fi


if [[ "$total_nonfree" -eq 0 ]]; then
	rms_happy=$(cat <<EOHAPPY
?????????????????++++=~===,~::+IIIIIIIIIIIIIIII???+==+++++?IIIIII?+++++?II?+++?I
?7I?IIIII?????????++~,::,,~,:,::?IIIIIIIIIIIII????+=+++++?+IIIIII??++++?II?+++?I
?I7IIIIII???II????~.......:::..:~IIIIIIIIIIIIII????++++++?+IIIIII??++++?III+++?I
?I7II7IIII??II??+:....,.,~++???.,==IIIIIIIIIIII????++++++?+IIIIII??++++?III+++?I
?I7II7777I?III?=:.....:=+++?????~=~~I?IIIIIIIII?????+++++??IIIIII??++++?III?++?I
?I7II7777I?II???=..,,,+++++????I?=,::~??II7IIII??I??++?++??IIIIII??++++?III?++?I
?IIII7777I?II+=II..,~=++++++???I?I~~~:+??IIIIIIIIII?+??++??IIIIII??++++?III?+++I
??IIII777I?I7++?I?.,==++????+~,=I??..,++?IIIIIIIIII????+~==IIIIII+~::::+III+::=I
??IIII777IIII++?+++~~~=~,==+?,.~I+??:.::~??IIIIIIII????+++?IIIIII??+++??III?++?I
??IIII777III7++~+????:~~,,,+??+=+????~,:,~+IIIIIIII????++??IIIIIII??????III?+??I
??IIII777III7I=+++++?::~=+=++??++?I?+==.,:~+?IIIIII?????+??IIIIIII??????III????I
??II?II7IIIII7++=++?+?~=++~+?+=+==+?+=:..:+=I+??III?????+??IIIIIII??????IIII???I
??IIII7777III7++++=?I7~+++=::.:=~=++=~:....,~==++I?++???+??IIIIIII??????IIII???I
??I7II7777III7+++?.?II7=+~~~~,~+?:~~::...,.,:,~+????+???+??IIIII7I??????IIII???I
??IIII7777III7++++=,III7=+:~~~,==~=:::.......::~~+I?????????77777I??????IIII???I
??II???????II?++++=,?III?~~~~=~:=+=:~,.,....,,..,=??+???????77777II?????IIII???I
???IIIII7I?II+++++=::?II7=~:====+~~~~.,:,,~::~:::,~+????????I7777II?????IIII???I
???III7777??I++?+=,=~+~=I?I~+~+~+=::,,,.,::::~~:~=:+=???????77777II?????IIII???I
???III7777I?+++++.,==~??+II::=~~::,.,,:::::::::~~=+~==??????777777I?????IIII???I
???III7777?+++?+=,====.?I??.,,,..,:,,,,.,::,~:~~~~=.++??????I77777I?????IIII????
???III777I=++???+,=+++=??+?,,,,,,,,,,,:,:,~::~~~~~~=.+??????I77777I?????IIII+??I
+??II????I+++???+:~++++~??+..,,,,,:::,,,:::::~~~~~==+=??????I77777I?????IIII????
++???????++++????,=+++?=+++.,.:,:::,:,,::::::~~~~~==????????I77777II????IIII????
++++?????++???II,,=+++??::=,,,,,::,,.,::::::~~~~~~~=????????I7777I?I????IIII????
+++??III+++??III.,=++++?+,,,,,,::,:,,:::::::~~~~~~==????????IIIIII??????IIIII???
++++???++????II?,,.=+++++++:,,,,,,,,,:::::::~~~~~~==????????IIIIIIII?????IIII???
=+++??++++??III?.,.,=+++++++++,,,,,,,::::~~~~~~~~~==?????+??IIIIII???????III????
=+++=++++???II?+...,,,=++++++++==...,::,:~~~~~~~~~=+?????+???IIIIIII?????IIII???
++++++++++????+=...,,.,~+++++++++++::~:~:,~~~~~~~==+?++++=++?IIIIII?+++++III?+++
++++++++++????+=...,,,,,~+++++++++?:::::~~:~~~~~~==~+======++IIIIII?+==++III?+=+
++?+++++?+???++~.,.,,,,,.=++++++++++::::~~~~~~~~~====++++=+++IIIIII?+++++IIII+++
=++=+++++??+++=~.,,,,,,,..=++++++?++++:::~~~~~~~===~?++++++++?IIIII?++++?IIII?++
=++~++++++++++=:,,,,,,:,,,.==++++++++?+=~~~~~~~~===~?++++++++?IIIII?++++??III?++
EOHAPPY
);
	
	if [[ "$ascii" -eq 1 ]]; then
		echo ""
		echo "$rms_happy"
		printmsg "rms_happy_ascii"
	else
		printmsg "rms_happy"
	fi
elif [[ "$percentage_nonfree" -ge 10 ]]; then
	rms_disappoint=$(cat <<EODISAPPOINT
......................,,:::::::+==I?++~~~??????????+??+~::,,,:,,.,.,............
......................,:::::::+??+++~:,~+?????????????+=,:,,,,,,,,,.,.,,,,......
......................,,:::::??+=~:::+????IIII???????++=~,,,,,,,,,,,.,::,.......
......................,,:::????????I?+????IIII???????+===,,.,,,,,:,,,,,,,.......
......................,,:??+++=~~:::,+???+=:~=+???++++=:~:,..,:,......,,........
......................,???+=~:,,...,:==++=~:~+++=+++++++::,,...,,,..............
.....................+???++=~:.,.,:::,:?+~::,=+~===+++?+=:,::,........,,........
..................~++??+?=:,.....,~==::I?=~:,,,~=:==+???+::.:~::,.....,.,.......
...............+?????++===:..,......=~II?++=~~==++?????++~,:,,,,:.,...:..,.,,,,,
............:?????+++===~:,,,,..,,~~~?II?????++?????????+=~,,,.,,.,...,..,,,,,,,
..........=?????++====~~:,,,,,..:~~~?II?+???????????????+++~,:,,,,.,......,,,,..
........+?????+=~~~~~::,,,,,....~++++++=???+????????????++++~:.,,,.,,.,.,,,,,,,,
......??????++=~:::::,,,,,,.....~+++,::...=+++????????+++++++~,.....,..,.,,,,,,,
..,=???????+=~:::,:,,,,,,,:.....~+==~,:,:==+?=++??????=+====+~::........,,,,,,,,
:???????++==~~:,:,....,,:::,....~==~~:~,:~:~~~+++??+?+==~~===~:~,,.,.....,,,,,,,
??????+====~:,:,......,,::::....~=~~:::,:~:~~~~=???+=+==~:~=~:::~.,........,,:,,
?????+++==~:,,,.......,::::,...,~~:,,,.,.,~~:::~++===~=~:,::~~:.............,,:,
???+++==~::,,.........,,::::..,,:::,,:~~~~====::===~~~:~::,:=::.....,.,.....,,:,
??+++=~~:,,,..........,::::,..,,:~,:~~~:~:~~==~::::~~,:,:~~:~:,....,,.......,,,,
+++==~::,,....,,......,,:::..:,,,::,~~:::~===~=+:::=,~:,~~,:~,....,,.....,.....,
+==~~:,,,.............,,:,:,.,,.:~==~:,==~==~?~~,:,,:,~,:,,:,:::,............,,,
=~::::,........,.,....,:::::..,:.==~=~+:=:~+=:~,,~:,,,,,,,::~~:,.:,,....,.,.....
~::::,................,,:,,.,,,,,:=~=::~~,:::,,,,~,:,:,~~~~~~~~.,:===========~,.
::::,.........,.......,,,,,,...,::::~:=::~::.,,,.::~::::::~~~~=================~
::::,.........,,..,...,,,:,,,,,,:::~=:,,..::,...,.:::,.,~~============~=~~~::,,,
~::,,.........,..,,,..,,:::::::,::,::,,...,,.,..~:,.,,,=============~~~::::~~~~~
:,,::,.......,,...,,.,,,::,::::::~::,:,....,::,,,,,,~=============~::::~~=======
,:~~:~::~~,..,,,.....,,,::::::::,,.,,,,,.::,,,,,,:==========~~===~::~~~=========
~~~~~~~~~:~~~,:::~,,~:~~:~::::,..,....::,:,,,,:======~~===~~~~~~~::~~~==========
~~~~~~~~~~~~~~~~~~~~:~~~~~~:.,::,,,,~:::,,,~=========~==~~~~~~~~:~~~===========~
~~~~~~~~~~~~~~~~~~~:~~~~~:,,::,.,:~::::~~~========~~===~~~~~~~~,~~===~==========
~~~:~~~~~~~~~~:~~~:~~~~~:,:~:.:~~~:?=~~~===========~~=~:~~~~~~::=======~~~~~~~~~
~~~:~~~:~~~~~~:~~~~~~~~:,~~,,~~~~~~===============~~~~~~~~~~~~::~~~~~~~=~=======
~~~::~~:~~~~~~:~~~~~~~,,~~::~~~~~=~===============~~~~~~~:::~:::===~~~~~~~=====~
::~:::~::::~:~:~~~~~~,:~~:~~~~~==================~~~~:~~~~::::~~~~==~~~::::::::~
EODISAPPOINT
);

	if [[ "$ascii" -eq 1 ]]; then
		echo ""
		echo "$rms_disappoint"
		printmsg "rms_disappoint_ascii"
	else
		printmsg "rms_disappoint"
	fi
fi
