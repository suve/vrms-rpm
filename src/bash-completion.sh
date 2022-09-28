# bash-completion file for vrms-rpm
# Copyright (C) 2018, 2020 Artur "suve" Iwicki
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

_vrms_rpm() {
	COMPREPLY=()

	local curr="${COMP_WORDS[COMP_CWORD]}"
	local prev="${COMP_WORDS[COMP_CWORD-1]}"
	local opts="--ascii --colour --describe --explain --help --image --licence-list --list --version"

	if [[ "$prev" == "--color" ]] || [[ "$prev" == "--colour" ]]; then
		local colourmodes="auto always never"
		COMPREPLY=( $(compgen -W "$colourmodes" -- "$curr") )
	elif [[ "$prev" == "--licence-list" ]] || [[ "$prev" == "--license-list" ]]; then
		# If the current argument looks like a file path, suggest something from the file system
		# Otherwise suggest something from the built-in licence lists
		if [[ "${curr:0:1}" == "/" ]] || [[ "${curr:0:2}" == "./" ]] || [[ "${curr:0:2}" == "~/" ]]  || [[ "${curr:0:3}" == "../" ]]; then
			COMPREPLY=( $(compgen -df -- "$curr") )
		else
			local licences="__LICENCE_LIST__"
			COMPREPLY=( $(compgen -W "$licences \~ . .. /" -- "$curr") )
		fi
	elif [[ "$prev" == "--list" ]]; then
		local listmodes="none free non-free all"
		COMPREPLY=( $(compgen -W "$listmodes" -- "$curr") )
	else
		COMPREPLY=( $(compgen -W "$opts" -- "$curr") )
	fi

	return 0
}
complete -o filenames -F _vrms_rpm vrms-rpm
