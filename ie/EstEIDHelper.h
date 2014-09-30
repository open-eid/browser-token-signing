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

#ifndef __ESTEID_HELPER_H__
#define __ESTEID_HELPER_H__

#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/md5.h>

#pragma comment(lib, "crypt32.lib")
#include <stdio.h>
#include <windows.h>
#include <Wincrypt.h>
#include <string>
//#include "esteid_log.h"


#define MD5_HASH_LEN 16

using namespace std;

class CEstEIDHelper
{
public:
	CEstEIDHelper(void){};
	~CEstEIDHelper(void);

	static char* calculateMD5Hash(char* certificate) {	
		unsigned char certMD5[MD5_DIGEST_LENGTH];
		char result[2 * MD5_DIGEST_LENGTH + 1] = "";
		char chunk[3];
		int i;

		MD5((unsigned char*)certificate, strlen(certificate), certMD5);
		
		for(i = 0; i < MD5_DIGEST_LENGTH; i++){
			sprintf_s(chunk, 3, "%02X", certMD5[i]);
			strcat_s(result, MD5_DIGEST_LENGTH * 2 + 1, chunk);
		}
		
		//EstEID_log("cert hash as HEX string: %s", result);	
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

#endif