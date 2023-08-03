#!/usr/bin/php
<?php
/**
 * Licence list update script for vrms-rpm
 * Copyright (C) 2023 suve (a.k.a. Artur Frenszek-Iwicki)
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
 * this program (LICENCE.txt). If not, see <https://www.gnu.org/licenses/>.
 */

function read_file($url) {
	$content = file_get_contents($url);
	if($content === false) {
		fprintf(STDERR, "Failed to read file at \"%s\"\n", $url);
		die(1);
	}

	return $content;
}


function read_json_file($url) {
	$content = read_file($url);
	$json = json_decode($content, null, 32, JSON_OBJECT_AS_ARRAY | JSON_INVALID_UTF8_SUBSTITUTE);
	if(json_last_error() !== JSON_ERROR_NONE) {
		fprintf(STDERR, "Failed to decode JSON: %s\n", json_last_error_msg());
		die(1);
	}

	return $json;
}

function write_to_file($filename, $data) {
	$data = implode("\n", $data) . "\n"; // Make sure file ends with a newline
	if(file_put_contents($filename, $data) === false) {
		fprintf(STDERR, "Failed to write to file \"%s\"\n", $filename);
		die(1);
	}
}

function get_nested_value($jsonObj, ...$keys) {
	foreach($keys as $key) {
		if(!array_key_exists($key, $jsonObj)) return NULL;
		$jsonObj = $jsonObj[$key];
	}

	return $jsonObj;
}

function allowed_in_fedora($licence) {
	$statusList = get_nested_value($licence, 'license', 'status');
	if($statusList !== NULL) {
		// The proper way
		foreach($statusList as $status) {
			switch($status) {
				case 'allowed':
				case 'allowed-content':
				case 'allowed-documentation':
				case 'allowed-fonts':
					return true;

				case 'not-allowed':
					return false;

				default:
					fprintf(STDERR, "Unknown Fedora licence status: \"%s\"\n", $status);
					die(1);
			}
		}
	} else {
		// The deprecated way
		$approved = get_nested_value($licence, 'approved');
		if($approved === "yes") return true;
		if($approved === "no") return false;
	}

	fprintf(STDERR, "Failed to determine Fedora licence status:\n%s\n", print_r($licence, true));
	die(1);
}

function update_fedora() {
	$fedoraSourceUrl = 'https://gitlab.com/fedora/legal/fedora-license-data/-/jobs/artifacts/main/raw/fedora-licenses.json?job=json';
	$fedoraData = read_json_file($fedoraSourceUrl);

	$list = [];
	foreach($fedoraData as $licence) {
		if(!allowed_in_fedora($licence)) continue;

		// The proper way
		$spdxExpr = get_nested_value($licence, 'license', 'expression');
		if($spdxExpr !== NULL) $list[] = $spdxExpr;

		$legacyAbbrevs = get_nested_value($licence, 'fedora', 'legacy-abbreviation');
		if($legacyAbbrevs !== NULL) {
			foreach($legacyAbbrevs as $abbrev) $list[] = $abbrev;
		}

		// The deprecated way
		$spdxExpr = get_nested_value($licence, 'spdx_abbrev');
		if($spdxExpr !== NULL) $list[] = $spdxExpr;

		$legacyAbbr = get_nested_value($licence, 'fedora_abbrev');
		if($legacyAbbr !== NULL) $list[] = $legacyAbbr;
	}

	// TODO: Probably need to write custom function for case-insensitive sort+uniq
	$list = array_unique($list, SORT_STRING);
	sort($list, SORT_STRING | SORT_FLAG_CASE);
	write_to_file('../licences/fedora.txt', $list);
}

function array_key_is_true($array, $key) {
	if(!array_key_exists($key, $array)) return false;
	return (bool)($array[$key]);
}

function update_spdx() {
	$spdxSourceUrl = 'https://raw.githubusercontent.com/spdx/license-list-data/main/json/licenses.json';
	$spdxData = read_json_file($spdxSourceUrl);

	$fsfOnly = [];
	$osiOnly = [];
	$either = [];
	$both = [];

	foreach($spdxData['licenses'] as $licence) {
		$identifier = $licence['licenseId'];
		$fsfApproved = array_key_is_true($licence, 'isFsfLibre');
		$osiApproved = array_key_is_true($licence, 'isOsiApproved');

		if($fsfApproved) $fsfOnly[] = $identifier;
		if($osiApproved) $osiOnly[] = $identifier;
		if($fsfApproved || $osiApproved) $either[] = $identifier;
		if($fsfApproved && $osiApproved) $both[] = $identifier;
	}

	sort($fsfOnly, SORT_STRING | SORT_FLAG_CASE);
	write_to_file('../licences/spdx-only-fsf.txt', $fsfOnly);

	sort($osiOnly, SORT_STRING | SORT_FLAG_CASE);
	write_to_file('../licences/spdx-only-osi.txt', $osiOnly);

	sort($either, SORT_STRING | SORT_FLAG_CASE);
	write_to_file('../licences/spdx-fsf-or-osi.txt', $either);

	sort($both, SORT_STRING | SORT_FLAG_CASE);
	write_to_file('../licences/spdx-fsf-and-osi.txt', $both);
}

// SUSE licensing guidelines say to use SPDX identifiers.
// The spreadsheet exists mostly to list licenses that do not
// have an SPDX identifier yet, plus some legacy identifiers.
//
// TODO: Take the SPDX data and include it here.
function update_suse() {
	$suseSourceUrl = 'https://docs.google.com/spreadsheets/d/14AdaJ6cmU0kvQ4ulq9pWpjdZL5tkR03exRSYJmPGdfs/pub';

	$dom = new DOMDocument("1.0", "UTF-8");
	if($dom->loadHTML(read_file($suseSourceUrl)) === false) {
		fprintf(STDERR, "Failed to read HTML\n");
		die(1);
	}

	$viewport = $dom->getElementById("sheets-viewport");
	if($viewport === NULL) {
		fprintf(STDERR, "Failed to find #sheets-viewport in HTML\n");
		die(1);
	}

	$list = [];
	foreach($viewport->getElementsByTagName("td") as $cell) {
		// The SUSE spreadsheet is a mess.
		// It contains both alternative names for licences
		// and some assorted comments.
		$text = trim($cell->textContent);

		// We do not want to pull in "Freeware" or "NonFree".
		if(($text === "Freeware") || ($text === "NonFree") || ($text === "SUSE-Freeware") || ($text === "SUSE-NonFree")) {
			continue;
		}

		// URLs are definitely not licence names.
		if(str_starts_with($text, "https://") || str_starts_with($text, "http://")) {
			continue;
		}

		// Assume that anything with a parenthesis in it is not a licence name.
		if(str_contains($text, "(")) {
			continue;
		}

		// Assume that anything with a space is not a licence name.
		if(str_contains($text, " ")) {
			// ...unless the space is there because of "WITH".
			if(!str_contains($text, " WITH ")) {
				continue;
			}
		}

		$list[] = $text;
	}

	$list = array_unique($list, SORT_STRING);
	sort($list, SORT_STRING | SORT_FLAG_CASE);
	write_to_file('../licences/suse.txt', $list);
}

function update_tweaked() {
	$combined = [];

	$files = glob('../licences/*.txt', GLOB_NOSORT);
	foreach($files as $file) {
		$content = read_file($file);
		$lines = explode("\n", $content);
		$combined = array_merge($combined, $lines);
	}

	$combined = array_unique($combined, SORT_STRING);
	sort($combined, SORT_STRING | SORT_FLAG_CASE);
	write_to_file('../licences/tweaked.txt', $combined);
}

chdir(__DIR__);
update_fedora();
update_spdx();
update_suse();
update_tweaked();

