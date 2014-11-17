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

// EstEIDCertificate.cpp : Implementation of CEstEIDCertificate
#include "stdafx.h"
#include "EstEIDCertificate.h"
#include <Cryptuiapi.h>
#include "esteid_log.h"

#define BUFFER_SIZE 1024

#pragma comment(lib, "comsuppw")

STDMETHODIMP CEstEIDCertificate::get_id(BSTR *id){
	EstEID_log("");
	*id = _bstr_t(this->id.c_str()).Detach();
	return S_OK;
}

std::string CEstEIDCertificate::dateToString(FILETIME *filetime) {
	EstEID_log("");
	SYSTEMTIME systemtime;
	FileTimeToSystemTime(filetime, &systemtime);
	char buf[160] = {0};
	sprintf_s(buf, "%02d.%02d.%04d %02d:%02d:%02d", systemtime.wDay, systemtime.wMonth, systemtime.wYear, systemtime.wHour, systemtime.wMinute, systemtime.wSecond);
	std::string result = buf;
	return result;
}

void CEstEIDCertificate::readFromCertContext() {
	LOG_LOCATION;
	PCCERT_CONTEXT pCertContext = NULL;
	HCERTSTORE hCertStore = NULL;
	CRYPTUI_SELECTCERTIFICATE_STRUCT sel = {sizeof(sel)};
	int counter = 0;

	hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
	if(!hCertStore){
		throw CryptoException();
	}
	sel.pvCallbackData = &counter;
	sel.pFilterCallback = filter_proc;
	sel.rghDisplayStores = &hCertStore;
	sel.cDisplayStores = 1;
	
#ifdef _SEB_BUILD	
	EstEID_log("SEB build");
	PCCERT_CONTEXT pCertContextForEnumeration = NULL;
	int certificatesCount = 0;
	while(pCertContextForEnumeration = CertEnumCertificatesInStore(hCertStore, pCertContextForEnumeration)) {
		if(isValidForSigning(pCertContextForEnumeration)) {
			certificatesCount++;	
			pCertContext = pCertContextForEnumeration;
		}
	}

	EstEID_log("Certificates count %i", certificatesCount);

	if(certificatesCount != 1) {
		pCertContext = CryptUIDlgSelectCertificate(&sel);
	}
#else
	pCertContext = CryptUIDlgSelectCertificate(&sel);
#endif
	if(!pCertContext) {
		EstEID_log("User didn't select sertificate");
		throw CryptoException(ESTEID_USER_CANCEL);
	}

	loadCertContexts(pCertContext);
	if(pCertContext){
		CertFreeCertificateContext(pCertContext);
	}
	if(hCertStore) {
		CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
	}
}

void CEstEIDCertificate::loadCertContexts(PCCERT_CONTEXT certContext) {
	USES_CONVERSION;
	EstEID_log("");
	CryptoErrorHandler(CertGetNameString(certContext, CERT_NAME_ATTR_TYPE, 0, szOID_COMMON_NAME, this->CN, 2048));
	std::string s = W2A(this->CN);
	EstEID_log("Certificate = %s", s.c_str());
		
	CryptoErrorHandler(CertGetNameString(certContext, CERT_NAME_ATTR_TYPE, CERT_NAME_ISSUER_FLAG, szOID_COMMON_NAME, this->issuerCN, 2048));

	std::stringstream buf;
	for(size_t i = certContext->pCertInfo->SerialNumber.cbData ; i > 0  ;i--) 
		buf << std::hex << std::setfill('0') << std::setw(2) << (int) certContext->pCertInfo->SerialNumber.pbData[i-1] << " ";
	std::string strBuf = buf.str();
	EstEID_log("serial=%s", strBuf.c_str());

	this->validFrom = dateToString(&certContext->pCertInfo->NotBefore);
	this->validTo = dateToString(&certContext->pCertInfo->NotAfter);

	this->certificate = (BYTE*)malloc(certContext->cbCertEncoded + 1);
	memcpy(this->certificate, certContext->pbCertEncoded, certContext->cbCertEncoded);
	this->certificate[certContext->cbCertEncoded] = '\0';
	EstEID_log("certificate binary length = %i", certContext->cbCertEncoded);
	
	calculateMD5Hash(certContext->cbCertEncoded);
	binCert2Hex(certContext->cbCertEncoded);
}

STDMETHODIMP CEstEIDCertificate::get_CN(BSTR *cn) {
	EstEID_log("");
	*cn = _bstr_t(this->CN).Detach();
	return S_OK;
}

STDMETHODIMP CEstEIDCertificate::get_issuerCN(BSTR *issuerCN) {
	EstEID_log("");
	*issuerCN = _bstr_t(this->issuerCN).Detach();
	return S_OK;
}

STDMETHODIMP CEstEIDCertificate::get_validFrom(BSTR *validFrom) {
	EstEID_log("");
	*validFrom = _bstr_t(this->validFrom.c_str()).Detach();
	return S_OK;
}

STDMETHODIMP CEstEIDCertificate::get_validTo(BSTR *validTo) {
	EstEID_log("");
	*validTo = _bstr_t(this->validTo.c_str()).Detach();
	return S_OK;
}

STDMETHODIMP CEstEIDCertificate::get_certificateAsPem(BSTR *certificate) {
	EstEID_log("");
	*certificate = _bstr_t("not implemented yet").Detach();
	return S_OK;
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


void CEstEIDCertificate::CryptoErrorHandler(BOOL result) {
	EstEID_log("");
	if (!result) {
		throw CryptoException();
	}
}

void CEstEIDCertificate::binCert2Hex(const unsigned int binLength) {
	EstEID_log("");
	char *hex = (char*)CEstEIDHelper::getBytesAsHexString((void*)this->certificate, binLength);
	this->certificateAsHex.assign(hex);
	free(hex);	
}

void CEstEIDCertificate::calculateMD5Hash(unsigned int certLength) {
	EstEID_log("");
	this->id.assign(CEstEIDHelper::calculateMD5Hash((char*)this->certificate));
}
