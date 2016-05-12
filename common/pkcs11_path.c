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

#include "pkcs11_path.h"
#include "atr_fetcher.h"
#include "esteid_map.h"
#include "esteid_log.h"
#include "string.h"

#ifdef __APPLE__
const char *estPath = "/Library/EstonianIDCard/lib/esteid-pkcs11.so";
const char *latPath = "/Library/latvia-eid/lib/otlv-pkcs11.so";
const char *finPath = "/Library/mPolluxDigiSign/libcryptoki.dylib";
//const char *finPath = "/Library/OpenSC/lib/opensc-pkcs11.so";
const char *litPath = "/System/Library/Security/tokend/CCSuite.tokend/Contents/Frameworks/libccpkip11.dylib";
#else
const char *estPath = "opensc-pkcs11.so";
const char *latPath = "otlv-pkcs11.so";
const char *finPath = "opensc-pkcs11.so";
const char *litPath = "/usr/lib/ccs/libccpkip11.so";
#endif

enum country {EST, LIT, LAT, FIN, UNKNOWN};
enum country pathCountry = UNKNOWN;

static EstEID_Map createMap() {
    EstEID_Map map = NULL;
    map = EstEID_mapPut(map, "3BFE9400FF80B1FA451F034573744549442076657220312E3043", estPath);
    map = EstEID_mapPut(map, "3B6E00FF4573744549442076657220312E30", estPath);
    map = EstEID_mapPut(map, "3BDE18FFC080B1FE451F034573744549442076657220312E302B", estPath);
    map = EstEID_mapPut(map, "3B5E11FF4573744549442076657220312E30", estPath);
    map = EstEID_mapPut(map, "3B6E00004573744549442076657220312E30", estPath);
    
    map = EstEID_mapPut(map, "3BFE1800008031FE454573744549442076657220312E30A8", estPath);
    map = EstEID_mapPut(map, "3BFE1800008031FE45803180664090A4561B168301900086", estPath);
    map = EstEID_mapPut(map, "3BFE1800008031FE45803180664090A4162A0083019000E1", estPath);
    map = EstEID_mapPut(map, "3BFE1800008031FE45803180664090A4162A00830F9000EF", estPath);
    
    map = EstEID_mapPut(map, "3BF9180000C00A31FE4553462D3443432D303181", estPath);
    map = EstEID_mapPut(map, "3BF81300008131FE454A434F5076323431B7", estPath);
    map = EstEID_mapPut(map, "3BFA1800008031FE45FE654944202F20504B4903", estPath);
    map = EstEID_mapPut(map, "3BFE1800008031FE45803180664090A4162A00830F9000EF", estPath);
    
    map = EstEID_mapPut(map, "3BDD18008131FE45904C41545649412D65494490008C", latPath);
    
    map = EstEID_mapPut(map, "3B7B940000806212515646696E454944", finPath);
    
    map = EstEID_mapPut(map, "3BF81300008131FE45536D617274417070F8", litPath);
    map = EstEID_mapPut(map, "3B7D94000080318065B08311C0A983009000", litPath);
    return map;
}

bool isLithuanianDriverLoaded() {
    return pathCountry == LIT;
}

void setPathCountry(const char *path) {
    if (!strcmp(path, estPath)) {
        pathCountry = EST;
    } else if (!strcmp(path, latPath)) {
        pathCountry = LAT;
    } else if (!strcmp(path, litPath)) {
        pathCountry = LIT;
    } else if (!strcmp(path, finPath)) {
        pathCountry = FIN;
    } else {
        pathCountry = UNKNOWN;
    }
}

const char *getPkcs11ModulePath() {
    LOG_LOCATION
    EstEID_Map atrToDriverMap = createMap();
    char *atrs = fetchAtrs();

    if (!atrs) {
        EstEID_log("No ATRs found, using default driver path %s", estPath);
        setPathCountry(estPath);
        EstEID_mapFree(atrToDriverMap);
        return estPath;
    }
    
    LOG_LOCATION
    for (size_t i = 0; i < strlen(atrs) - 1; ++i) {
        const char *path = EstEID_mapGet(atrToDriverMap, &atrs[i]);

        if (path) {
            //use the first match found
            EstEID_log("driver path = %s", path);
            setPathCountry(path);
            const char *path_copy_to_return = strdup(path);

            EstEID_mapFree(atrToDriverMap);
            atrToDriverMap = NULL;
            path = NULL;
            free(atrs);
            atrs = NULL;

            return path_copy_to_return;
        }
        i+= strlen(&atrs[i]);
    }
    EstEID_log("no driver for ATR found, using default driver path %s", estPath);
    setPathCountry(estPath);

    EstEID_mapFree(atrToDriverMap);
    atrToDriverMap = NULL;
    free(atrs);
    atrs = NULL;

    return estPath;
}
