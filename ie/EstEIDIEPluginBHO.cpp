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
#include "EstEIDIEPluginBHO.h"
#include "esteid_error.h"
#include "version.h"
#include "CngCapiSigner.h"
#include "HostExceptions.h"
#include <string.h>
extern "C" {
#include "esteid_log.h"
}
#define FAIL_IF_SITE_IS_NOT_ALLOWED if(!isSiteAllowed()){return Error((this->errorMessage).c_str());}

extern "C" {
extern int EstEID_errorCode;
}

STDMETHODIMP CEstEIDIEPluginBHO::SetSite(IUnknown* pUnkSite) {
	EstEID_log("");
	IObjectWithSiteImpl<CEstEIDIEPluginBHO>::SetSite(pUnkSite);

	CComPtr<IServiceProvider> pSP;
	HRESULT hr = GetSite(IID_IServiceProvider, reinterpret_cast<LPVOID *>(&pSP));
	if(S_OK != hr) {
		return hr;
	}
	hr = pSP->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, reinterpret_cast<LPVOID *>(&webBrowser));

	return S_OK;
}

STDMETHODIMP CEstEIDIEPluginBHO::get_version(BSTR *result) {
	EstEID_log("");
	*result = _bstr_t(ESTEID_PLUGIN_VERSION).Detach();
	return S_OK;
}

STDMETHODIMP CEstEIDIEPluginBHO::get_pluginLanguage(BSTR *result) {
	EstEID_log("");
	*result = _bstr_t(this->language ? this->language : _T("")).Detach();
	return S_OK;
}


STDMETHODIMP CEstEIDIEPluginBHO::put_pluginLanguage(BSTR language) {
	EstEID_log("");
	this->language = _bstr_t(language).Detach();
	return S_OK;
}


STDMETHODIMP CEstEIDIEPluginBHO::get_errorMessage(BSTR *result) {
	EstEID_log("");
	*result = _bstr_t(this->errorMessage.c_str()).Detach();
	return S_OK;
}

STDMETHODIMP CEstEIDIEPluginBHO::get_errorCode(BSTR *result) {
	EstEID_log("");
	*result = _bstr_t(this->errorCode).Detach();
	return S_OK;
}

BOOL CEstEIDIEPluginBHO::isSiteAllowed() {
	EstEID_log("");
	BSTR url_buffer;
	if(webBrowser == NULL){
		EstEID_log("Browser object is not initialized!!!");
		return FALSE;
	}

	webBrowser->get_LocationURL(&url_buffer);
	BOOL allowed = wcsstr(url_buffer, _T("https://")) == url_buffer;
#ifdef DEVELOPMENT_MODE
	allowed = TRUE;
	EstEID_log("*** Development Mode, all protocols allowed ***");
#endif	
	if(!allowed){
		this->errorCode = ESTEID_SITE_NOT_ALLOWED;
		this->errorMessage.assign("Site not allowed");
		EstEID_log("Protocol not allowed");
	}
	SysFreeString(url_buffer);	
	return allowed;
}

STDMETHODIMP CEstEIDIEPluginBHO::getCertificate(IDispatch **_certificate){
	EstEID_log("");
	FAIL_IF_SITE_IS_NOT_ALLOWED;
	try {
		if(!this->certificate || !isSameCardInReader(this->certificate)) {
			this->certificate.CoCreateInstance(CLSID_EstEIDCertificate);
		}
		CComPtr<IEstEIDCertificate> cert;
		this->certificate.CopyTo(&cert);
		*_certificate = cert.Detach();
		clearErrors();
		return S_OK;
	}
	catch (BaseException &e) {
		EstEID_log("Exception caught when getting certificate: %s: %s", e.getErrorMessage().c_str(), e.getErrorDescription().c_str());
		setError(e);
		return Error((this->errorMessage).c_str());
	}
}

BOOL CEstEIDIEPluginBHO::isSameCardInReader(CComPtr<IEstEIDCertificate> _cert){ //todo: must check is card changed
	/* not implemented yet */
	this->certificate = NULL;
	return false;
}

STDMETHODIMP CEstEIDIEPluginBHO::sign(BSTR id, BSTR hash, BSTR language, BSTR *signature){
	LOG_LOCATION;
	FAIL_IF_SITE_IS_NOT_ALLOWED;
	USES_CONVERSION;
	try {
		wstring hashToSign(hash, SysStringLen(hash));
		std::string hashString(hashToSign.begin(), hashToSign.end());
		char * certId = W2A(id);
		CngCapiSigner *signer = new CngCapiSigner(hashString, certId);
		string result = signer->sign();
		*signature = _bstr_t(result.c_str()).Detach();
		clearErrors();
		EstEID_log("Signing ended");
		return S_OK;
	}
	catch (BaseException &e) {
		EstEID_log("Exception caught during signing: %s: %s", e.getErrorMessage().c_str(), e.getErrorDescription().c_str());
		setError(e);
		return Error((this->errorMessage).c_str());
	}
}

BOOL CEstEIDIEPluginBHO::certificateMatchesId(PCCERT_CONTEXT certContext, BSTR id) {
	USES_CONVERSION;
	BYTE *cert;
	cert = (BYTE*)malloc(certContext->cbCertEncoded + 1);
	memcpy(cert, certContext->pbCertEncoded, certContext->cbCertEncoded);
	cert[certContext->cbCertEncoded] = '\0';
	std::string hashAsString;
	hashAsString = CEstEIDHelper::calculateMD5Hash((char*)cert);
	free(cert);
	BOOL result = (strcmp(hashAsString.c_str(), W2A(id)) == 0);
	EstEID_log("Cert match check result: %s", result ? "matches" : "does not match");
	return result;
}

void CEstEIDIEPluginBHO::clearErrors() {
	EstEID_log("");
	this->errorCode = 0;
	this->errorMessage.assign("");
}

void CEstEIDIEPluginBHO::setError(BaseException &exception) {
	EstEID_log("");
	this->errorCode = exception.getErrorCode();
	this->errorMessage.assign(exception.getErrorDescription());
	EstEID_log("Set error: %s (HEX %Xh, DEC %u)", this->errorMessage.c_str(), exception.getErrorCode(), exception.getErrorCode());
}
