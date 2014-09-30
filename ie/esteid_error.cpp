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

#include "esteid_error.h"

typedef struct {
	unsigned int code;
	char *message;
} ErrorInfo;

ErrorInfo errorInfo[] = {
	{ESTEID_NO_ERROR, ""},
	{ESTEID_USER_CANCEL, "User canceled"},
	{ESTEID_CERT_NOT_FOUND_ERROR, "Certificate not found"},
	{ESTEID_MD5_ERROR, "MD5 error"}, 
	{ESTEID_CRYPTO_API_ERROR, "Crypto API error"},
	{ESTEID_UNKNOWN_ERROR, "Unknown error"},
	{ESTEID_PKCS11_ERROR, "PKCS#11 error"},
	{ESTEID_LIBRARY_LOAD_ERROR, "Library loading failed"},
	{ESTEID_INVALID_HASH_ERROR, "Invalid hash"},
	{ESTEID_PTHREAD_ERROR, "Posix thread error"},
	{ESTEID_SITE_NOT_ALLOWED, "Site not allowed"}
};

char *getErrorMessage(unsigned int code)
{
	int position = 0;
	int length = sizeof(errorInfo) / sizeof(ErrorInfo);
	for(int i = 0; i < length; i++) {
		if(errorInfo[i].code == code) {
			position = i;
			break;
		}
	}	
	return errorInfo[position].message;
}
