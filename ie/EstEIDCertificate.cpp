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

#include "stdafx.h"
#include "HostExceptions.h"
#include "EstEIDCertificate.h"
#include "EstEIDHelper.h"
extern "C" {
#include "esteid_log.h"
}

STDMETHODIMP CEstEIDCertificate::get_id(BSTR *id){
	EstEID_log("");
	*id = _bstr_t(this->id.c_str()).Detach();
	return S_OK;
}

void CEstEIDCertificate::readFromCertContext() {
	LOG_LOCATION;
	PCCERT_CONTEXT pCertContext = NULL;
	HCERTSTORE hCertStore = NULL;
	CRYPTUI_SELECTCERTIFICATE_STRUCT sel = {sizeof(sel)};
	int counter = 0;

	hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
	if(!hCertStore){
		throw TechnicalException("Failed to open Cert Store");
	}

	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);

	sel.pvCallbackData = &counter;
	sel.pFilterCallback = filter_proc;
	sel.rghDisplayStores = &hCertStore;
	sel.cDisplayStores = 1;
	
#ifdef _SEB_BUILD	
	EstEID_log("SEB build");
	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);
	PCCERT_CONTEXT pCertContextForEnumeration = NULL;
	int certificatesCount = 0;
	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);
	while(pCertContextForEnumeration = CertEnumCertificatesInStore(hCertStore, pCertContextForEnumeration))
	{
		if(isValidForSigning(pCertContextForEnumeration))
		{
			certificatesCount++;	
			pCertContext = pCertContextForEnumeration;
			loadCertContexts(pCertContext);
		}
	}

	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);

	EstEID_log("Certificates count %i", certificatesCount);

	if(certificatesCount != 1)
	{
		pCertContext = CryptUIDlgSelectCertificate(&sel);
	}

	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);

	if (pCertContextForEnumeration){
		CertFreeCertificateContext(pCertContextForEnumeration);
	}
	if (!pCertContext) {
		EstEID_log("User didn't select sertificate");
		throw UserCancelledException();
	}
#else
	pCertContext = CryptUIDlgSelectCertificate(&sel);
	if (!pCertContext) {
		EstEID_log("User didn't select sertificate");
		throw UserCancelledException();
	}
	loadCertContexts(pCertContext);

	if (pCertContext){
		CertFreeCertificateContext(pCertContext);
	}
#endif
	

	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);

	if (hCertStore) {
		CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
	}
	
}

void CEstEIDCertificate::loadCertContexts(PCCERT_CONTEXT certContext) {
	USES_CONVERSION;
	EstEID_log("");

	this->certificate = (BYTE*)malloc(certContext->cbCertEncoded + 1);
	memcpy(this->certificate, certContext->pbCertEncoded, certContext->cbCertEncoded);
	this->certificate[certContext->cbCertEncoded] = '\0';
	EstEID_log("certificate binary length = %i", certContext->cbCertEncoded);
	
	calculateMD5Hash(certContext->cbCertEncoded);
	binCert2Hex(certContext->cbCertEncoded);
}

STDMETHODIMP CEstEIDCertificate::get_certificateAsHex(BSTR *certificate) {
	EstEID_log("");
	*certificate = _bstr_t(this->certificateAsHex.data()).Detach();
	return S_OK;
}

STDMETHODIMP CEstEIDCertificate::get_cert(BSTR *certificate) {
	EstEID_log("");
	*certificate = _bstr_t(this->certificateAsHex.data()).Detach();
	return S_OK;
}

void CEstEIDCertificate::binCert2Hex(const unsigned int binLength) {
	EstEID_log("");
	char *hex = (char*)CEstEIDHelper::getBytesAsHexString((void*)this->certificate, binLength);
	this->certificateAsHex.assign(hex);
	free(hex);	
}

void CEstEIDCertificate::calculateMD5Hash(unsigned int certLength) {
	EstEID_log("");
	char *certMd5Hash = CEstEIDHelper::calculateMD5Hash((char*)this->certificate);
	EstEID_log("Certificate ID (MD5 hash) is %s", certMd5Hash);
	this->id.assign(certMd5Hash);
}
