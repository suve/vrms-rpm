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

function read_json_file($url) {
	$content = file_get_contents($url);
	if($content === false) {
		fprintf(STDERR, "Failed to read file at \"%s\"\n", $url);
		die(1);
	}

	$json = json_decode($content, null, 32, JSON_OBJECT_AS_ARRAY | JSON_INVALID_UTF8_SUBSTITUTE);
	if(json_last_error() !== JSON_ERROR_NONE) {
		fprintf(STDERR, "Failed to decode JSON: %s\n", json_last_error_msg());
		die(1);
	}

	return $json;
}

function array_key_is_true($array, $key) {
	if(!array_key_exists($key, $array)) return false;
	return (bool)($array[$key]);
}

function write_to_file($filename, $data) {
	$data = implode("\n", $data) . "\n"; // Make sure file ends with a newline
	if(file_put_contents($filename, $data) === false) {
		fprintf(STDERR, "Failed to write to file \"%s\"\n", $filename);
		die(1);
	}
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

chdir(__DIR__);
update_spdx();

