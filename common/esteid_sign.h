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

#ifndef __ESTEID_SIGN__
#define __ESTEID_SIGN__

#include "esteid_certinfo.h"

typedef struct {
    char *(*promptFunction)(void *, const char *, const char *, unsigned, int);
    void (*alertFunction)(void *, const char *);
    void *nativeWindowHandle;
	char *pin2;
} EstEID_PINPromptData;

typedef struct {
	CK_SESSION_HANDLE session;
	CK_RV result;
} EstEID_PINPadThreadData;

#ifdef _WIN32
typedef struct {
	EstEID_PINPromptData pinPromptData;
	char* name;
	char* message;
	unsigned int minPin2Length;
} EstEID_PINPromptDataEx;
#endif

int EstEID_signHashHex(char **signatureHex, CK_SLOT_ID slotID, EstEID_Map cert, const char *hashHex, EstEID_PINPromptData pinPromptData);

char *EstEID_sign(char *cn, char *hash, EstEID_PINPromptData pinPrompt);

int EstEID_getRemainingTries(CK_SLOT_ID slotID);

int EstEID_findNonRepuditionCert(EstEID_Certs *certs, const char *certId);

char* EstEID_getFullNameWithPersonalCode(EstEID_Map cert);

int EstEID_isPinPad(CK_SLOT_ID slotID);

int EstEID_getSlotId(char* certId, CK_SLOT_ID* slotId);

char *EstEID_hex2bin(const char *hex);

#endif