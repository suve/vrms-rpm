#
# Makefile for vrms-rpm
# Copyright (C) 2017 Marcin "dextero" Radomski
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

PREFIX ?= /usr/local

PO_FILES := $(shell ls lang/*.po)
MO_FILES := $(PO_FILES:lang/%.po=build/locale/%/LC_MESSAGES/vrms-rpm.mo)

MANS := $(shell ls man/*.man)
MAN_LANGS := $(MANS:man/%.man=%)
NON_EN_MAN_LANGS := $(filter-out en, $(MAN_LANGS))

SOURCES := $(shell ls src/*.c)
OBJECTS := $(SOURCES:src/%.c=build/%.o)

.PHONY: build clean install remove

help:
	@echo "TARGETS:"
	@echo "    build - compile project"
	@echo "    clean - remove compiled artifacts"
	@echo "    install - compile & install project"
	@echo "    remove - uninstall project"
	@echo ""
	@echo "VARIABLES:"
	@echo "    PREFIX - installation prefix (default: /usr/local)"
	@echo "             used during build to set up file paths"

build: $(MO_FILES) build/good-licences.txt build/vrms-rpm

build/locale/%/LC_MESSAGES/vrms-rpm.mo: lang/%.po
	mkdir -p "$(shell dirname "$@")"
	msgfmt --check -o "$@" "$<"

build/good-licences.txt: src/good-licences.txt
	cat "$<" | sort --dictionary-order | uniq > "$@"

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o "$@" "$<"

build/vrms-rpm: $(OBJECTS)
	$(CC) $(CFLAGS) -o "$@" $^

clean:
	rm -rf build

install/bin/vrms-rpm: build/vrms-rpm
	install -vD -p -m 755 "$<" "$@"

install/share/suve/vrms-rpm/good-licences.txt: build/good-licences.txt
	install -vD -m 644 build/good-licences.txt "$@"

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
install/prepare: $(NON_EN_MAN_LANGS:%=install/share/man/%/man1/vrms-rpm.1)
install/prepare: $(MO_FILES:build/%=install/share/%)

install: install/prepare
	mkdir -p "$(PREFIX)"
	cp -a install/* "$(PREFIX)"
	rm -rf install

remove: install/prepare
	find install -type f | sed -e 's|^install|$(PREFIX)|' | xargs rm -vf
	find install -depth -type d | sed -e 's|^install|$(PREFIX)|' | xargs rmdir -v --ignore-fail-on-non-empty
	rm -rf install
