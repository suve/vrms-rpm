/**
 * vrms-rpm - list non-free packages on an rpm-based Linux distribution
 * Copyright (C) 2026 suve (a.k.a. Artur Frenszek-Iwicki)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program (LICENCE.txt). If not, see <http://www.gnu.org/licenses/>.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include "src/options.h"
#include "test/test.h"

enum Outcome {
	OUTCOME_FAILURE, // Expect child to fail (exit with non-zero code)
	OUTCOME_SUCCESS, // Expect child to bail out early (with zero exit code)
	OUTCOME_PARSED   // Expect child to parse options successfully
};

#define LLBUFSIZ 32

struct TestResult {
	enum Outcome outcome;
	int exitCode;
	struct Options opts;
	char licenceList[LLBUFSIZ];
};

struct SharedMemory {
	int parsed;
	struct Options opts;
	char licenceList[LLBUFSIZ];	
};

/*
 * The way `options_parse()` works, it either succeeds and returns a value,
 * or it fails and exits the program, without returning.
 * This makes testing tricky.
 *
 * The sensible thing to do would be to rewrite the function so that it can
 * return an error value. However, I'm not interested in sensible, so here's
 * a different idea:
 * 1. Allocate some shared memory
 * 2. Fork to create a child process
 * 3. Have the child process call `options_parse()`
 * 4. If the child is still alive, copy the result to the shared memory
 * 5. Collect the child's exit code in the parent process
 *
 * As horrible as this may be, it allows for testing both "should fail"
 * and "should succeed" cases, as well as for checking the result value
 * for the latter scenarios.
 *
 * Since the cmocka testing library runs single-threaded, allocating
 * and freeing shared memory is done once, during setup & teardown.
 *
 * Based on this Stack Overflow answer: https://stackoverflow.com/a/5656561
 */
int test_setup__options(void **state) {
	struct SharedMemory *shared = mmap(
		NULL,
		sizeof(struct SharedMemory),
		PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS,
		-1,
		0
	);
	if(shared == MAP_FAILED) {
		char *reason = strerror(errno);
		fail_msg("Failed to mmap(): %s\n", reason);
	}

	*state = shared;
	return 0;
}

int test_teardown__options(void **state) {
	struct SharedMemory *shared = *state;
	munmap(shared, sizeof(struct SharedMemory));
	return 0;
}

static struct TestResult forbidden_magic(struct SharedMemory *shared, char **argv) {
	memset(shared, 0, sizeof(struct TestResult));

	pid_t pid = fork();
	if(pid == -1) {
		char *reason = strerror(errno);
		fail_msg("Failed to fork(): %s\n", reason);
	}

	// Child
	if(pid == 0) {
		/*
		 * Close stdout and stderr file descriptors and replace them with
		 * ones writing to /dev/null. This will prevent the child process
		 * from writing to the parent's outputs.
		 */
		close(1); open("/dev/null", O_WRONLY, 0666);
		close(2); open("/dev/null", O_WRONLY, 0666);

		int argc = 0;
		while(argv[argc] != NULL) ++argc;
		struct Options opts = options_parse(argc, argv);

		shared->parsed = 1;
		memcpy(&shared->opts, &opts, sizeof(struct Options));
		strncpy(shared->licenceList, opts.licenceList, LLBUFSIZ);

		exit(EXIT_SUCCESS);
	}

	// Parent
	int wstatus;
	waitpid(pid, &wstatus, 0);
	if(!WIFEXITED(wstatus)) {
		fail_msg("Child process did not terminate normally");
	}

	struct TestResult result;
	result.exitCode = WEXITSTATUS(wstatus);
	result.outcome = 
		(shared->parsed) ? OUTCOME_PARSED :
		(result.exitCode == 0) ? OUTCOME_SUCCESS : OUTCOME_FAILURE;

	memcpy(&result.opts, &shared->opts, sizeof(struct Options));
	memcpy(&result.licenceList, &shared->licenceList, LLBUFSIZ);

	return result;
}

static const char* OUTCOME[] = {
	"FAILURE",
	"SUCCESS",
	"PARSED"
};

#define run_testcase(Outcome, ...) \
	do { \
		char *argv[] = { "vrms-rpm", __VA_ARGS__, NULL }; \
		result = forbidden_magic(*state, argv); \
\
		if(Outcome != result.outcome) { \
			fail_msg( \
				"Expected the outcome to be %s, but it was %s(%d)", \
				OUTCOME[Outcome], \
				OUTCOME[result.outcome], \
				result.exitCode \
			); \
		} \
	} while(0)

void test__optionsDefault(void **state) {
	struct TestResult result;
	run_testcase(OUTCOME_PARSED, NULL);
}

void test__optionsFormat(void **state) {
	struct TestResult result;
	run_testcase(OUTCOME_FAILURE, "--format");
	run_testcase(OUTCOME_FAILURE, "--format", "AlienSpeak");

	run_testcase(OUTCOME_PARSED, NULL);
	assert_int_equal(result.opts.format, OPT_FORMAT_TEXT);

	run_testcase(OUTCOME_PARSED, "--format", "json");
	assert_int_equal(result.opts.format, OPT_FORMAT_JSON);

	run_testcase(OUTCOME_PARSED, "--format", "json-pretty");
	assert_int_equal(result.opts.format, OPT_FORMAT_JSON_PRETTY);

	run_testcase(OUTCOME_PARSED, "--format", "pretty-json");
	assert_int_equal(result.opts.format, OPT_FORMAT_JSON_PRETTY);

	run_testcase(OUTCOME_PARSED, "--format", "text");
	assert_int_equal(result.opts.format, OPT_FORMAT_TEXT);
}

void test__optionsGrammar(void **state) {
	struct TestResult result;
	run_testcase(OUTCOME_FAILURE, "--grammar");
	run_testcase(OUTCOME_FAILURE, "--grammar", "NonExistent");

	run_testcase(OUTCOME_PARSED, "--grammar", "loose");
	assert_int_equal(result.opts.grammar, OPT_GRAMMAR_LOOSE);

	run_testcase(OUTCOME_PARSED, "--grammar", "spdx-lenient");
	assert_int_equal(result.opts.grammar, OPT_GRAMMAR_SPDX_LENIENT);

	run_testcase(OUTCOME_PARSED, "--grammar", "spdx-strict");
	assert_int_equal(result.opts.grammar, OPT_GRAMMAR_SPDX_STRICT);
}

void test__optionsHelp(void **state) {
	struct TestResult result;
	run_testcase(OUTCOME_SUCCESS, "--help");
}

void test__optionsLicenceList(void **state) {
	struct TestResult result;
	run_testcase(OUTCOME_PARSED, "--licence-list", "BigBeatifulList");
	assert_string_equal(result.licenceList, "BigBeatifulList");
}

void test__optionsList(void **state) {
	struct TestResult result;
	run_testcase(OUTCOME_FAILURE, "--list");
	run_testcase(OUTCOME_FAILURE, "--list", "Epstein's");

	run_testcase(OUTCOME_PARSED, NULL);
	assert_int_equal(result.opts.list, OPT_LIST_NONFREE);

	run_testcase(OUTCOME_PARSED, "--list", "none");
	assert_int_equal(result.opts.list, OPT_LIST_NONE);

	run_testcase(OUTCOME_PARSED, "--list", "free");
	assert_int_equal(result.opts.list, OPT_LIST_FREE);

	run_testcase(OUTCOME_PARSED, "--list", "non-free");
	assert_int_equal(result.opts.list, OPT_LIST_NONFREE);

	run_testcase(OUTCOME_PARSED, "--list", "all");
	assert_int_equal(result.opts.list, OPT_LIST_BOTH);
}

void test__optionsRequiredArgs(void **state) {
	struct TestResult result;
	run_testcase(OUTCOME_FAILURE, "--colour");
	run_testcase(OUTCOME_FAILURE, "--evra");
	run_testcase(OUTCOME_FAILURE, "--format");
	run_testcase(OUTCOME_FAILURE, "--grammar");
	run_testcase(OUTCOME_FAILURE, "--licence-list");
	run_testcase(OUTCOME_FAILURE, "--list");
}

void test__optionsUnknown(void **state) {
	struct TestResult result;
	run_testcase(OUTCOME_FAILURE, "--no-such-option");
}

void test__optionsVersion(void **state) {
	struct TestResult result;
	run_testcase(OUTCOME_SUCCESS, "--version");
}
