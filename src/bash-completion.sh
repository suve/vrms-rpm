# bash_completion file for vrms-rpm
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

_vrms_rpm() {
	COMPREPLY=()

	local curr="${COMP_WORDS[COMP_CWORD]}"
	local prev="${COMP_WORDS[COMP_CWORD-1]}"
	local opts="--ascii --explain --help --list --version"

	if [[ "$prev" == "--list" ]]; then
		local listmodes="none free non-free all"
		COMPREPLY=( $(compgen -W "$listmodes" -- "$curr") )
	else
		COMPREPLY=( $(compgen -W "$opts" -- "$curr") )
	fi

	return 0
}
complete -F _vrms_rpm vrms-rpm
