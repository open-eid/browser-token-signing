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

#ifndef ESTEID_MAP_H
#define	ESTEID_MAP_H

#include <stdio.h>

struct EstEID_MapEntry {
    const char *key;
    const char *value;
    struct EstEID_MapEntry *next;
};

typedef struct EstEID_MapEntry *EstEID_Map;

EstEID_Map EstEID_mapPut(EstEID_Map map, const char *key, const char *value);

EstEID_Map EstEID_mapPutNoAlloc(EstEID_Map map, const char *key, const char *value);

const char *EstEID_mapGet(EstEID_Map map, const char *key);

void EstEID_mapFree(EstEID_Map map);

unsigned int EstEID_mapSize(EstEID_Map map);

void EstEID_mapPrint(FILE *stream, EstEID_Map map);

EstEID_Map EstEID_mapClone(EstEID_Map map);

#endif
