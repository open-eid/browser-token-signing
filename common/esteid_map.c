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
#include <string.h>
#include "esteid_map.h"

EstEID_Map EstEID_mapPut(EstEID_Map map, const char *key, const char *value) {
	return EstEID_mapPutNoAlloc(map, strdup(key), strdup(value));
}

EstEID_Map EstEID_mapPutNoAlloc(EstEID_Map map, const char *key, const char *value) {
	if (!map) {
		map = (EstEID_Map)malloc(sizeof(struct EstEID_MapEntry));
		map->key = key;
		map->value = value;
		map->next = NULL;
	}
	else if (!strcmp(map->key, key)) {
		free((void *)map->key);
		free((void *)map->value);
		map->key = key;
		map->value = value;
	}
	else {
		map->next = EstEID_mapPut(map->next, key, value);
	}
	return map;
}

const char *EstEID_mapGet(EstEID_Map map, const char *key) {
	if (!map) {
		return NULL;
	}
	if (!strcmp(map->key, key)) {
		return map->value;
	}
	return EstEID_mapGet(map->next, key);
}

unsigned int EstEID_mapSize(EstEID_Map map) {
	if (!map) return 0;
	return 1 + EstEID_mapSize(map->next);
}

void EstEID_mapFree(EstEID_Map map) {
	if (!map) return;
	if (map->next) EstEID_mapFree(map->next);
	free((void *)map->value);
	free((void *)map->key);
	free(map);
}

void EstEID_mapPrint(FILE *stream, EstEID_Map map) {
	if (!map) return;
	fprintf(stream, "\t%s = %s\n", map->key, map->value);
	EstEID_mapPrint(stream, map->next);
}

EstEID_Map EstEID_mapClone(EstEID_Map map) {
	EstEID_Map result;
	if (!map) return NULL;
	result = EstEID_mapPut(NULL, map->key, map->value);
	result->next = EstEID_mapClone(map->next);
	return result;
}

