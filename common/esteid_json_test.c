/*
 * Estonian ID card plugin for web browsers
 *
 * Copyright (C) 2010-2011 Codeborne <info@codeborne.com>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esteid_json.h"

void assertStringEquals(const char *expected, char *actual) {
	assert(expected);
	if (!actual) {
		fprintf(stderr, "Out of memory!\n");
		abort();
	}
	if (strcmp(expected, actual)) {
		printf("assertion failed - expected: [%s], actual [%s]\n", expected, actual);
		free(actual);
		exit(1);
	}
	free(actual);
}

int main(void) {
	assertStringEquals("foo", EstEID_jsonString("foo"));
	assertStringEquals("foo\\\"", EstEID_jsonString("foo\""));
	assertStringEquals("foo\\\\", EstEID_jsonString("foo\\"));
	assertStringEquals("\\n", EstEID_jsonString("\n"));
	assertStringEquals("foo\\\\bar\\r\\n\\\"test\\\"", EstEID_jsonString("foo\\bar\r\n\"test\""));

	EstEID_Map map = NULL;
	assertStringEquals("{}", EstEID_mapToJson(map));
	map = EstEID_mapPut(map, "foo", "bar");
	assertStringEquals("{\"foo\": \"bar\"}", EstEID_mapToJson(map));

	map = EstEID_mapPut(map, "a\nb", "\"123\"");
	assertStringEquals("{\"foo\": \"bar\", \"a\\nb\": \"\\\"123\\\"\"}", EstEID_mapToJson(map));

	EstEID_mapFree(map);

	return 0;
}

