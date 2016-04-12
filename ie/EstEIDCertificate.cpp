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
#include "CertificateSelectorFactory.h"
extern "C" {
#include "esteid_log.h"
}

STDMETHODIMP CEstEIDCertificate::get_id(BSTR *id){
	EstEID_log("");
	*id = _bstr_t(this->id.c_str()).Detach();
	return S_OK;
}

void CEstEIDCertificate::readFromCertContext() {
	PCCERT_CONTEXT pCertContext = CertificateSelectorFactory::createCertificateSelector()->getCert();
	loadCertContexts(pCertContext);
	if (pCertContext){
		CertFreeCertificateContext(pCertContext);
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
