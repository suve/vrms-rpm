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

CMD := 'for I in lang/*.po; do echo "build/locale/$$(basename $$I .po)/LC_MESSAGES/vrms-rpm.mo"; done'
LANGS := $(shell bash -c $(CMD))

CMD := 'for I in man/*.man; do [ "$$I" != "man/en.man" ] && echo "$$(basename "$$I" .man)"; done'
NON_EN_MANS := $(shell bash -c $(CMD))

help:
	@echo "TARGETS:"
	@echo "    build - compile project"
	@echo "    clean - remove compiled artifacts"
	@echo "    install - compile & install project"
	@echo "    remove - uninstall project"
	@echo ""
	@echo "VARIABLES:"
	@echo "    PREFIX - installation prefix (default: /usr/local)"

.PHONY: build
build: $(LANGS) build/vrms-rpm

build/locale/%/LC_MESSAGES/vrms-rpm.mo: lang/%.po
	mkdir -p "$(shell dirname "$@")"
	msgfmt --check -o "$@" "$<"

# force rebuild every time, to make sure PREFIX is correct
.PHONY: build/vrms-rpm
build/vrms-rpm: src/vrms-rpm.sh
	cp -p "$<" "$@"
	sed -e 's|prog_usr="/usr"|prog_usr="/$(PREFIX)"|' -i "$@"

.PHONY: clean
clean:
	rm -rf build

install/bin/vrms-rpm: build/vrms-rpm
	install -vD -p -m 755 "$<" "$@"

install/share/suve/vrms-rpm/good-licenses.txt:
	install -vD -m 644 src/good-licences.txt "$@"

install/share/man/man1/vrms-rpm.1: man/en.man
	install -vD -p -m 644 "$<" "$@"

install/share/man/%/man1/vrms-rpm.1: man/%.man
	install -vD -p -m 644 "$<" "$@"

install/share/locale/%: build/locale/%
	install -vD -p -m 644 "$<" "$@"

install/prepare: build
install/prepare: install/bin/vrms-rpm
install/prepare: install/share/suve/vrms-rpm/good-licenses.txt
install/prepare: install/share/man/man1/vrms-rpm.1
install/prepare: $(shell for MAN in $(NON_EN_MANS); do echo "install/share/man/$$MAN/man1/vrms-rpm.1" ; done)
	rsync -av build/*/ install

.PHONY: install
install: install/prepare
	mkdir -p "$(PREFIX)"
	rsync -av install/* "$(PREFIX)"
	rm -rf install

.PHONY: remove
remove: install/prepare
	find install -type f | sed -e 's|^install|$(PREFIX)|' | xargs rm -vf
	find install -depth -type d | sed -e 's|^install|$(PREFIX)|' | xargs rmdir -v --ignore-fail-on-non-empty
	rm -rf install
