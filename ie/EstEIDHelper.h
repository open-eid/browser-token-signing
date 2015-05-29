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

#pragma once

#include <windows.h>
#include <Wincrypt.h>
#include <string>

#define MD5_HASH_LEN 16

using namespace std;

class CEstEIDHelper
{
public:
	CEstEIDHelper(void){};
	~CEstEIDHelper(void);

	static char* calculateMD5Hash(char* certificate) {	
		HCRYPTPROV cryptProv;
		if (!CryptAcquireContext(&cryptProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
			return NULL;
		}

		HCRYPTHASH cryptHash;
		if (!CryptCreateHash(cryptProv, CALG_MD5, 0, 0, &cryptHash)) {
			CryptReleaseContext(cryptProv, 0);
			return NULL;
		}

		if (!CryptHashData(cryptHash, (BYTE*)certificate, strlen(certificate), 0)) {
			CryptReleaseContext(cryptProv, 0);
			CryptDestroyHash(cryptHash);
			return NULL;
		}

		DWORD cbHash = MD5_HASH_LEN;
		BYTE hash[MD5_HASH_LEN];
		if (!CryptGetHashParam(cryptHash, HP_HASHVAL, hash, &cbHash, 0)) {
			CryptReleaseContext(cryptProv, 0);
			CryptDestroyHash(cryptHash);
			return NULL;
		}

		char *hex = "0123456789abcdef";
		char result[2 * MD5_HASH_LEN + 1] = "";
		for (unsigned int i = 0; i < cbHash; i++) {
			result[i * 2] = hex[hash[i] >> 4];
			result[(i * 2) + 1] = hex[hash[i] & 0xF];
		}
		CryptReleaseContext(cryptProv, 0);
		CryptDestroyHash(cryptHash);
		return _strdup(result);
	};


	static void* CEstEIDHelper::getBytesAsHexString(void* bytes, unsigned int numOfBytes) {
		// NB! Releasing of allocated buffer is responsibility of caller
		char *hex = (char *)malloc(numOfBytes * 2 + 1);
		unsigned int limit = sizeof(char) * 2 + 1;
		for (unsigned int i = 0; i < numOfBytes; i++) {
			sprintf_s((hex + (i * 2)), limit, "%02X", ((unsigned char *)bytes)[i]);
		}
		hex[numOfBytes * 2] = '\0';
		return hex;
	};
};
