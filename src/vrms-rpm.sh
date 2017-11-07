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
# prog_usr is set appropriately during "make build"
# prog_dir tells us where to look for data files
prog_name="vrms-rpm"
prog_usr="/usr"
prog_dir="$prog_usr/share/suve/$prog_name"

# Function for printing translated printf-messages
function printmsg() {
	local translated=`gettext -s "msg_$1"`
	shift

	printf "$translated\n" $@
}

# Initialize gettext
export TEXTDOMAINDIR="$prog_usr/share/locale/"
export TEXTDOMAIN="vrms-rpm"

# Test gettext and revert to English if we don't get back a translated string
testgettext=`gettext -s "msg_help_usage"`
if [[ "$testgettext" == "msg_help_usage" ]]; then
	export LANGUAGE="en"
fi


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
			printmsg "help_usage" "$prog_name"

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
			echo "$prog_name v.1.2 by suve"
			exit
		;;

		-*)
			if [ $1 == '--' ]; then
				shift
				break
			else
				printmsg "unknown_option" "$prog_name" "$1"
				exit 1
			fi
		;;
	esac
	
	shift
done


function classify_package() {
	local push_value
	if [[ "$explain" -eq 1 ]]; then
		push_value="$1: $2"
	else
		push_value="$1"
	fi
	
	
	echo "$2" | grep --quiet --fixed-strings --word-regexp --file "$prog_dir/good-licences.txt"
	
	if [[ "$?" -eq 0 ]]; then
		free[${#free[@]}]="$push_value"
	else
		nonfree[${#nonfree[@]}]="$push_value"
	fi
}

# Set the field separator to a newline for splitting the package list
IFS=$'\n'

# In Fedora, package names cannot contain the ":" character, so it's safe to use as delimiter
packages=`rpm --all --query --queryformat '%{NAME}:%{LICENSE}\n' | sort`
packages=($packages)

free=()
nonfree=()

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
