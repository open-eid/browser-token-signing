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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esteid_map.h"

void assertIntEquals(int expected, int actual) {
	if (expected != actual) {
		printf("assertion failed - expected: %i, actual %i", expected, actual);
		exit(1);
	}
}

void assertStringEquals(const char *expected, const char *actual) {
	if (strcmp(expected, actual)) {
		printf("assertion failed - expected: [%s], actual [%s]", expected, actual);
		exit(1);
	}
}

void assertNull(const void *ptr) {
	if (ptr) {
		printf("assertion failed - expected: NULL, actual [%p]", ptr);
		exit(1);
	}
}

void assertNotNull(const void *ptr) {
	if (!ptr) {
		printf("assertion failed - expected: !NULL, actual NULL");
		exit(1);
	}
}

int main(void) {

	EstEID_Map map = NULL;
	assertIntEquals(0, EstEID_mapSize(map));

	map = EstEID_mapPut(map, "foo", "bar");
	assertNotNull(map);
	assertIntEquals(1, EstEID_mapSize(map));

	map = EstEID_mapPut(map, "foo", "FOO");
	assertIntEquals(1, EstEID_mapSize(map));

	char *k = strdup("key");
	char *v = strdup("value");
	map = EstEID_mapPut(map, k, v);
	free(k);
	free(v);
	assertIntEquals(2, EstEID_mapSize(map));
	assertStringEquals("value", EstEID_mapGet(map, "key"));

	assertNull(EstEID_mapGet(map, "xyz"));

	EstEID_mapFree(map);


	EstEID_Map m = EstEID_mapPut(NULL, "a", "A");
	EstEID_mapPut(m, "b", "B");
	EstEID_mapPut(m, "c", "C");

	printf("map:\n");
	EstEID_mapPrint(stdout, m);

	EstEID_Map c = EstEID_mapClone(m);
	EstEID_mapFree(m);

	printf("clone:\n");
	EstEID_mapPrint(stdout, c);
	EstEID_mapFree(c);

}

