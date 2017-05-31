#
# Makefile for vrms-rpm
# Copyright (C) 2017 Marcin "dextero" Radomski
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

PREFIX ?= /usr/local

PO_FILES := $(shell ls lang/*.po)
MO_FILES := $(PO_FILES:lang/%.po=build/locale/%/LC_MESSAGES/vrms-rpm.mo)

MANS := $(shell ls man/*.man)
MAN_LANGS := $(MANS:man/%.man=%)
NON_EN_MAN_LANGS := $(filter-out en, $(MAN_LANGS))

.PHONY: build build/vrms-rpm clean install remove

help:
	@echo "TARGETS:"
	@echo "    build - compile project"
	@echo "    clean - remove compiled artifacts"
	@echo "    install - compile & install project"
	@echo "    remove - uninstall project"
	@echo ""
	@echo "VARIABLES:"
	@echo "    PREFIX - installation prefix (default: /usr/local)"

build: $(MO_FILES) build/vrms-rpm

build/locale/%/LC_MESSAGES/vrms-rpm.mo: lang/%.po
	mkdir -p "$(shell dirname "$@")"
	msgfmt --check -o "$@" "$<"

# force rebuild every time, to make sure PREFIX is correct
build/vrms-rpm: src/vrms-rpm.sh
	cp -p "$<" "$@"
	sed -e 's|prog_usr="/usr"|prog_usr="/$(PREFIX)"|' -i "$@"

clean:
	rm -rf build

install/bin/vrms-rpm: build/vrms-rpm
	install -vD -p -m 755 "$<" "$@"

install/share/suve/vrms-rpm/good-licences.txt:
	install -vD -m 644 src/good-licences.txt "$@"

install/share/man/man1/vrms-rpm.1: man/en.man
	install -vD -p -m 644 "$<" "$@"

install/share/man/%/man1/vrms-rpm.1: man/%.man
	install -vD -p -m 644 "$<" "$@"

install/share/locale/%: build/locale/%
	install -vD -p -m 644 "$<" "$@"

install/prepare: build
install/prepare: install/bin/vrms-rpm
install/prepare: install/share/suve/vrms-rpm/good-licences.txt
install/prepare: install/share/man/man1/vrms-rpm.1
install/prepare: $(NON_EN_MAN_LANGS:%/install/share/man/%/man1/vrms-rpm.1)
install/prepare: $(MO_FILES:build/%=install/share/%)

install: install/prepare
	mkdir -p "$(PREFIX)"
	rsync -av install/* "$(PREFIX)"
	rm -rf install

remove: install/prepare
	find install -type f | sed -e 's|^install|$(PREFIX)|' | xargs rm -vf
	find install -depth -type d | sed -e 's|^install|$(PREFIX)|' | xargs rmdir -v --ignore-fail-on-non-empty
	rm -rf install
