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
#include <stdint.h>
#include <stdlib.h>
#include "esteid_json.h"

int isEscapedSymbol(char c) {
	return strchr("\"\\\r\n\t", c) != NULL;
}

char *EstEID_jsonString(const char *src) {
	size_t specialSymbolCount = 0u;
	char *s = (char *)src;
	size_t srcLen = 0u;
	for (; *s; srcLen++) if (isEscapedSymbol(*(s++))) specialSymbolCount++;
	if (SIZE_MAX - srcLen < specialSymbolCount)
		return NULL;
	if (SIZE_MAX - srcLen - specialSymbolCount < 1u)
		return NULL;
	char *result = (char*)malloc(srcLen + specialSymbolCount + 1u);
	if (!result)
		return NULL;
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
	char *key = EstEID_jsonString(entry.key);
	if (!key)
		return NULL;
	char *value = EstEID_jsonString(entry.value);
	if (!value)
		goto EstEID_mapEntryToJson_cleanup;
	size_t const kLen = strlen(key);
	if (SIZE_MAX - kLen < 7u)
		goto EstEID_mapEntryToJson_cleanup_2;
	size_t const vLen = strlen(value);
	if (SIZE_MAX - kLen - 7u < vLen)
		goto EstEID_mapEntryToJson_cleanup_2;
	char *result = (char *)malloc(kLen + vLen + 7u);
	if (result)
		sprintf(result, "\"%s\": \"%s\"", key, value);
	free(key);
	free(value);
	return result;
EstEID_mapEntryToJson_cleanup_2:
	free(value);
EstEID_mapEntryToJson_cleanup:
	free(key);
	return NULL;
}

char *EstEID_mapToJson(EstEID_Map map) {
	size_t rLen = 3u;
	char * result = (char *) malloc(rLen);
	if (!result)
		return NULL;
	memcpy(result, "{", 2u);
	char * entry;
	for (; map; map = map->next) {
		if (map->next) {
			if (SIZE_MAX - rLen < 2u)
				goto EstEID_mapToJson_error_cleanup;
			rLen += 2u;
		}
		if (!(entry = EstEID_mapEntryToJson(*map)))
			goto EstEID_mapToJson_error_cleanup;
		size_t const eLen = strlen(entry);
		if (SIZE_MAX - rLen < eLen)
			goto EstEID_mapToJson_error_cleanup;
		rLen += eLen;
		char * const newResult = (char *) realloc(result, rLen);
		if (!newResult)
			goto EstEID_mapToJson_error_cleanup;
		result = newResult;
		strcat(result, entry);
		free(entry);
		entry = NULL;
		if (map->next) strcat(result, ", ");
	}
	strcat(result, "}");
	return result;
EstEID_mapToJson_error_cleanup:
	free(entry);
	free(result);
	return NULL;
}

