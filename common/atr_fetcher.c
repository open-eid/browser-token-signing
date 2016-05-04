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

#include "atr_fetcher.h"
#include "esteid_log.h"
#include "string.h"

char *bin2hex(const BYTE *bin, const int binLength) {
    int j;
    char *hex = (char *)malloc(binLength * 2 + 1);
    for (j = 0; j < binLength; j++) sprintf(hex + (j * 2), "%02X", (unsigned char)bin[j]);
    hex[binLength * 2] = '\0';
    return hex;
}

char *getAtrFromReader(SCARDCONTEXT hContext, const char *readerName) {
    EstEID_log("finding ATR for reader: %s", readerName);
    
    SCARDHANDLE hCard;
    DWORD dwActiveProtocol;
    LONG rv;
    
    rv = SCardConnect(hContext, readerName, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
    if (rv != SCARD_S_SUCCESS) {
        EstEID_log("SCardConnect ERROR: 0x%08X", rv);
        return NULL;
    }
    
    BYTE pbAtr[MAX_ATR_SIZE];
    DWORD dwState, dwProtocol, dwReaderLen;
    DWORD dwAtrLen = sizeof(pbAtr);
    
    rv = SCardStatus(hCard, NULL, &dwReaderLen, &dwState, &dwProtocol, pbAtr, &dwAtrLen);
    if (rv != SCARD_S_SUCCESS) {
        EstEID_log("SCardStatus ERROR: 0x%08X", rv);
        return NULL;
    }
    char *atr = bin2hex(pbAtr, dwAtrLen);
    EstEID_log("ATR = %s", atr);
    SCardDisconnect(hCard, SCARD_LEAVE_CARD);
    return atr;
}

const char *fetchAtrs() {
    LONG rv;
    SCARDCONTEXT hContext;
    LPTSTR mszReaders;
    DWORD dwReaders;
    
    rv = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (rv != SCARD_S_SUCCESS) {
        EstEID_log("SCardEstablishContext ERROR: 0x%08X", rv);
        return NULL;
    }
    
    rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
    if (rv != SCARD_S_SUCCESS) {
        EstEID_log("SCardListReaders ERROR: 0x%08X", rv);
        SCardReleaseContext(hContext);
        return NULL;
    }
    
    mszReaders = calloc(dwReaders, sizeof(char));
    rv = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);
    if (rv != SCARD_S_SUCCESS) {
        EstEID_log("SCardListReaders ERROR: 0x%08X", rv);
        free(mszReaders);
        SCardReleaseContext(hContext);
        return NULL;
    }
    
    int readerCount = 0;
    int atrCount = 0;
    char *atrs;
    atrs = calloc(50, sizeof(char));
    for (size_t i = 0; i < dwReaders - 1; ++i) {
        char *atr = getAtrFromReader(hContext, &mszReaders[i]);
        if (atr) {
            atrs = (char *)realloc(atrs, strlen(atr) + 1 + strlen(atrs) + 1);
            strcat(atrs, atr);
            free(atr);
            atrCount++;
        }
        i+= strlen(&mszReaders[i]);
        readerCount++;
    }
    EstEID_log("found %i readers and %i ATRs", readerCount, atrCount);
    
    free(mszReaders);
    SCardReleaseContext(hContext);
    return atrCount > 0 ? atrs : NULL;
}
