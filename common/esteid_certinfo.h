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

#ifndef ESTEID_CERTINFO_H
#define ESTEID_CERTINFO_H

#define CRYPTOKI_COMPAT
#include "pkcs11.h"
#include "esteid_map.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif

#ifdef _WIN32
#define THREAD_RETURN_TYPE DWORD WINAPI
#define FAIL_IF_THREAD_ERROR(function, retval) if (!retval) { sprintf(EstEID_error, "%s failed with result %lu", function, GetLastError()); return FAILURE; }
#else
#define THREAD_RETURN_TYPE void *
#define FAIL_IF_PTHREAD_ERROR(function, retval) EstEID_log(""); if (retval) { sprintf(EstEID_error, "%s failed with result %i", function, retval); EstEID_errorCode = ESTEID_PTHREAD_ERROR;  return FAILURE; }
#endif


typedef struct {
    unsigned int count;
    EstEID_Map *certs;
    CK_SLOT_ID *slotIDs;
} EstEID_Certs;

int EstEID_CK_failure(const char *name, CK_RV result);

int EstEID_loadLibrary();

int EstEID_initializeCryptoki();

EstEID_Certs *EstEID_loadCerts();

void EstEID_printCerts();

void EstEID_freeCerts();

EstEID_Map EstEID_getNonRepudiationCert();
EstEID_Map EstEID_getNonRepudiationCertById(char* certID);

const char *EstEID_getCertPropertyName(const char *name);

void EstEID_waitForSlotEvent();

int EstEID_tokensChanged();

char *EstEID_bin2hex(const char *bin, const int binLength);

void EstEID_clear_error(void);

char *EstEID_getCertHash(char *certificate);

#endif
