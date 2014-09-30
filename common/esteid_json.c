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

#include <string.h>
#include <stdlib.h>
#include "esteid_json.h"

int isEscapedSymbol(char c) {
	return strchr("\"\\\r\n\t", c) != NULL;
}

char *EstEID_jsonString(const char *src) {
	int specialSymbolCount = 0;
	char *s = (char *)src;
	while (*s) if (isEscapedSymbol(*(s++))) specialSymbolCount++;
	char *result = (char*)malloc(strlen(src) + specialSymbolCount + 1);
	s = (char *)src;
	char *r = result;
  while (*s) {
		if (isEscapedSymbol(*s)) *(r++) = '\\';
		switch (*s) {
			case '\n' : *(r++) = 'n'; break;
			case '\r' : *(r++) = 'r'; break;
			case '\t' : *(r++) = 't'; break;
			default: *(r++) = *s;
		}
		s++;
	}
	*r = 0;
	return result;
}

char *EstEID_mapEntryToJson(struct EstEID_MapEntry entry) {
	char *k = EstEID_jsonString(entry.key);
	char *v = EstEID_jsonString(entry.value);
	char *result = (char *)malloc(strlen(k) + strlen(v) + 7);
	sprintf(result, "\"%s\": \"%s\"", k, v);
	free(k);
	free(v);
	return result;
}

char *EstEID_mapToJson(EstEID_Map map) {
	char *result = (char *)malloc(3);
	sprintf(result, "{");
	while (map) {
		char *entry = EstEID_mapEntryToJson(*map);
		result = (char *)realloc(result, strlen(result) + strlen(entry) + 4);
		strcat(result, entry);
		free(entry);
		if (map->next) strcat(result, ", ");
		map = map->next;
	}
	strcat(result, "}");
	return result;
}

