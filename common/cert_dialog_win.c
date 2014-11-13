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

#include "cert_dialog_win.h"

BOOL isCardInReader(PCCERT_CONTEXT certContext) {
#ifndef WIN_XP
	DWORD flags = CRYPT_ACQUIRE_CACHE_FLAG|CRYPT_ACQUIRE_COMPARE_KEY_FLAG|CRYPT_ACQUIRE_SILENT_FLAG;
	NCRYPT_KEY_HANDLE key = 0;
	DWORD spec = 0;
	BOOL ncrypt = FALSE;
	CryptAcquireCertificatePrivateKey(certContext, flags, 0, &key, &spec, &ncrypt);
	if(!key) {
		return FALSE;
	}
	if(ncrypt) {
		NCryptFreeObject(key);
	}
#endif
	return TRUE;
}

BOOL WINAPI filter_proc(PCCERT_CONTEXT certContext, BOOL *pfInitialSelectedCert, void *pvCallbackData) {
	return isValidForSigning(certContext);
}

BOOL WINAPI isValidForSigning(PCCERT_CONTEXT certContext) {
	BYTE keyUsage;
	CertGetIntendedKeyUsage(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, certContext->pCertInfo, &keyUsage, 1);
	if (!(keyUsage & CERT_NON_REPUDIATION_KEY_USAGE)) {
		return FALSE;
	}
	if(CertVerifyTimeValidity(NULL, certContext->pCertInfo) != 0) {
		return FALSE;
	}

	return isCardInReader(certContext);
}


