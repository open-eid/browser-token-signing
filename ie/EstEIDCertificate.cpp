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
#include "EstEIDCertificate.h"

#include "BinaryUtils.h"
#include "CertificateSelector.h"
#include "Logger.h"

#include <memory>
#include <wincrypt.h>

#define MD5_HASH_LEN 16

STDMETHODIMP CEstEIDCertificate::get_id(BSTR *_id){
	_log("");
	*_id = _bstr_t(id.c_str()).Detach();
	return S_OK;
}

STDMETHODIMP CEstEIDCertificate::Init(VARIANT filter) {
	USES_CONVERSION;

	std::string filterstr;
	if (filter.vt == VT_BSTR)
		filterstr = W2A(filter.bstrVal);
	std::unique_ptr<CertificateSelector> selector(CertificateSelector::createCertificateSelector());
	cert = selector->getCert(filterstr != "AUTH");
	_log("certificate binary length = %i", cert.size());
	hex = BinaryUtils::bin2hex(cert);

	HCRYPTPROV cryptProv;
	if (!CryptAcquireContext(&cryptProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		return -1;
	}

	HCRYPTHASH cryptHash;
	if (!CryptCreateHash(cryptProv, CALG_MD5, 0, 0, &cryptHash)) {
		CryptReleaseContext(cryptProv, 0);
		return -1;
	}

	if (!CryptHashData(cryptHash, cert.data(), cert.size(), 0)) {
		CryptReleaseContext(cryptProv, 0);
		CryptDestroyHash(cryptHash);
		return -1;
	}

	DWORD cbHash = MD5_HASH_LEN;
	std::vector<BYTE> hash(MD5_HASH_LEN, 0);
	if (!CryptGetHashParam(cryptHash, HP_HASHVAL, hash.data(), &cbHash, 0)) {
		CryptReleaseContext(cryptProv, 0);
		CryptDestroyHash(cryptHash);
		return -1;
	}
	CryptReleaseContext(cryptProv, 0);
	CryptDestroyHash(cryptHash);
	id = BinaryUtils::bin2hex(hash);
	_log("Certificate ID (MD5 hash) is %s", id.c_str());
	return S_OK;
}

STDMETHODIMP CEstEIDCertificate::get_certificateAsHex(BSTR *certificate) {
	_log("");
	*certificate = _bstr_t(hex.c_str()).Detach();
	return S_OK;
}

STDMETHODIMP CEstEIDCertificate::get_cert(BSTR *certificate) {
	_log("");
	return get_certificateAsHex(certificate);
}
