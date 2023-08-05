#
# Makefile for vrms-rpm
# Copyright (C) 2017,2023 Marcin "dextero" Radomski
# Copyright (C) 2018-2023 suve (a.k.a. Artur Frenszek-Iwicki)
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

DEFAULT_GRAMMAR ?= loose
DEFAULT_LICENCE_LIST ?= tweaked
DESTDIR ?=
PREFIX ?= /usr/local
WITH_LIBRPM ?= 1

CFLAGS += -std=c11 -iquote ./ -Wall -Wextra -D_POSIX_C_SOURCE=200112L
CWARNS := -Wfloat-equal -Wparentheses
CERRORS := -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=int-conversion -Werror=div-by-zero -Werror=sequence-point -Werror=uninitialized -Werror=duplicated-cond

ifeq "$(WITH_LIBRPM)" "1"
	CFLAGS += -DWITH_LIBRPM
	LDLIBS += -lrpm -lrpmio
else ifneq "$(WITH_LIBRPM)" "0"
	# The following line must *NOT* be indented, otherwise it's a syntax error
$(error "WITH_LIBRPM" must be "0" or "1", found "$(WITH_LIBRPM)")
endif

LICENCE_FILENAMES := $(basename $(notdir $(wildcard licences/*.txt)))
LICENCE_FILES := $(addprefix build/, $(wildcard licences/*.txt))

PO_FILES := $(wildcard lang/*.po)
MO_FILES := $(PO_FILES:lang/%.po=build/locale/%/LC_MESSAGES/vrms-rpm.mo)

MANS := $(wildcard man/*.man)
MAN_LANGS := $(MANS:man/%.man=%)
NON_EN_MAN_LANGS := $(filter-out en, $(MAN_LANGS))
MAN_FILES := $(MANS:man/%.man=build/man/%.man)

IMAGES := $(wildcard images/*)

SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:src/%.c=build/%.o)

TEST_SOURCES := $(wildcard test/*.c)
TEST_OBJECTS := $(TEST_SOURCES:test/%.c=build/test/%.o)


# -- variables end


.PHONY: all build executable lang-files man-pages install install/prepare remove test fuzz fuzz-coverage fuzz-classifier-spdx-strict fuzz-classifier-spdx-lenient fuzz-classifier-loose

all: build

build: executable lang-files man-pages $(LICENCE_FILES) build/bash-completion.sh

executable: build/vrms-rpm

lang-files: $(MO_FILES)

man-pages: $(MAN_FILES)

clean:
	rm -rf build/ src/config.h

install: install/prepare
	mkdir -p "$(DESTDIR)$(PREFIX)"
	cp -a install/* "$(DESTDIR)$(PREFIX)"
	rm -rf install

remove: install/prepare
	find install -type f | sed -e 's|^install|$(DESTDIR)$(PREFIX)|' | xargs rm -vf
	find install -depth -type d | sed -e 's|^install|$(DESTDIR)$(PREFIX)|' | xargs rmdir -v --ignore-fail-on-non-empty
	rm -rf install

test: build/test-suite
	./build/test-suite

fuzz: build/fuzz-classifier
	afl-fuzz -i test/fuzz/input -o test/fuzz/output "$(PWD)/build/fuzz-classifier" "$(FUZZ_CLASSIFIER)"

fuzz-classifier-spdx-strict: FUZZ_CLASSIFIER = spdx-strict
fuzz-classifier-spdx-strict: fuzz

fuzz-classifier-spdx-lenient: FUZZ_CLASSIFIER = spdx-lenient
fuzz-classifier-spdx-lenient: fuzz

fuzz-classifier-loose: FUZZ_CLASSIFIER = loose
fuzz-classifier-loose: fuzz

fuzz-coverage: CFLAGS += --coverage
fuzz-coverage: LDLIBS += -lgcov
fuzz-coverage: fuzz

help:
	@echo "TARGETS:"
	@echo "    all - build whole project (excluding tests)"
	@echo "    executable - compile C code and build the executable"
	@echo "    lang-files - compile translation files"
	@echo "    man-pages - compile man pages"
	@echo ""
	@echo "    clean - remove compiled artifacts"
	@echo "    install - install project"
	@echo "    remove - uninstall project"
	@echo ""
	@echo "    test - compile and run the test suite (requires cmocka)"
	@echo "    fuzz - compile and run the SPDX fuzz test. Variants for"
	@echo "           specific classifiers:"
	@echo "           * fuzz-classifier-spdx-strict (default)"
	@echo "           * fuzz-classifier-spdx-lenient"
	@echo "           * fuzz-classifier-loose"
	@echo "    fuzz-coverage - compile and run the SPDX fuzz test with"
	@echo "                    coverage instrumentation"
	@echo ""
	@echo "VARIABLES:"
	@echo "    DEFAULT_GRAMMAR"
	@echo "        default grammar rules to use for parsing licence strings"
	@echo "        changed at run-time using --grammar"
	@echo "        possible values: 'loose' (default), 'spdx-strict', 'spdx-lenient'"
	@echo "    DEFAULT_LICENCE_LIST"
	@echo "        default list of good licences to use (default: tweaked)"
	@echo "        changed at run-time using --licence-list"
	@echo "        possible values: $(LICENCE_FILENAMES)"
	@echo "    DESTDIR"
	@echo "        installation destination (default: empty)"
	@echo "        helpful when packaging the application"
	@echo "    PREFIX"
	@echo "        installation prefix (default: /usr/local)"
	@echo "        used to set up file paths"
	@echo "    WITH_LIBRPM"
	@echo "        when set to \"0\", disables linking against librpm"
	@echo "    FUZZ_CLASSIFIER"
	@echo "        set to pick a specific classifier in `make fuzz`:"
	@echo "        * spdx-strict (default)"
	@echo "        * spdx-lenient"
	@echo "        * loose"


# -- PHONY targets end


src/config.h: src/generate-config.sh
	src/generate-config.sh -d '$(DEFAULT_LICENCE_LIST)' -g '$(DEFAULT_GRAMMAR)' -l '$(LICENCE_FILENAMES)' -p '$(PREFIX)' > "$@"

build/bash-completion.sh: src/bash-completion.sh
	mkdir -p "$(dir $@)"
	sed -e 's|__LICENCE_LIST__|$(LICENCE_FILENAMES)|' < "$<" > "$@"

build/man/%.man: man/%.man src/convert-man.sh
	mkdir -p "$(dir $@)"
	cp "$<" "$@"
	src/convert-man.sh -d '$(DEFAULT_LICENCE_LIST)' -g '$(DEFAULT_GRAMMAR)' -l '$(LICENCE_FILENAMES)' -p '$(PREFIX)' -f "$@"

build/locale/%/LC_MESSAGES/vrms-rpm.mo: lang/%.po
	mkdir -p "$(dir $@)"
	msgfmt --check -o "$@" "$<"

build/licences/%.txt: licences/%.txt
	mkdir -p "$(dir $@)"
	LC_COLLATE=C sort --ignore-case < "$<" | uniq > "$@"

build/%.o: src/%.c src/config.h
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CWARNS) $(CERRORS) -c -o "$@" "$<"

build/test/%.o: test/%.c
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CWARNS) $(CERRORS) -c -o "$@" "$<"

build/vrms-rpm: $(OBJECTS)
	$(CC) $(CFLAGS) $(CWARNS) $(CERRORS) $(LDFLAGS) -o "$@" $^ $(LDLIBS)

build/test-suite: $(filter-out build/vrms-rpm.o, $(OBJECTS)) $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(CWARNS) $(CERRORS) $(LDFLAGS) -lcmocka -o "$@" $^ $(LDLIBS)

build/fuzz-classifier: CC = afl-gcc-fast
build/fuzz-classifier: LDLIBS += -lcmocka
build/fuzz-classifier: build/test/fuzz/classifier.o build/test/licences.o $(filter-out build/vrms-rpm.o, $(OBJECTS))
	$(CC) $(CFLAGS) $(CWARNS) $(CERRORS) $(LDFLAGS) -o "$@" $^ $(LDLIBS)

install/bin/vrms-rpm: build/vrms-rpm
	install -vD -p -m 755 "$<" "$@"

install/share/bash-completion/completions/vrms-rpm: build/bash-completion.sh
	install -vD -m 644 "$<" "$@"

install/share/suve/vrms-rpm/images/%: images/%
	install -vD -m 644 "$<" "$@"

install/share/suve/vrms-rpm/licences/%.txt: build/licences/%.txt
	install -vD -m 644 "$<" "$@"

install/share/man/man1/vrms-rpm.1: build/man/en.man
	install -vD -p -m 644 "$<" "$@"

install/share/man/%/man1/vrms-rpm.1: build/man/%.man
	install -vD -p -m 644 "$<" "$@"

install/share/locale/%: build/locale/%
	install -vD -p -m 644 "$<" "$@"

install/prepare: build
install/prepare: install/bin/vrms-rpm
install/prepare: install/share/bash-completion/completions/vrms-rpm
install/prepare: install/share/man/man1/vrms-rpm.1
install/prepare: $(NON_EN_MAN_LANGS:%=install/share/man/%/man1/vrms-rpm.1)
install/prepare: $(MO_FILES:build/%=install/share/%)
install/prepare: $(LICENCE_FILES:build/%=install/share/suve/vrms-rpm/%)
install/prepare: $(IMAGES:%=install/share/suve/vrms-rpm/%)
