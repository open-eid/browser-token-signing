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

// EstEIDIEPluginBHO.cpp : Implementation of CEstEIDIEPluginBHO
#include "stdafx.h"
#include "resource.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include "EstEIDIEPluginBHO.h"
#include "CryptoException.h"
#include <string.h>
#include "version.h"
#include <tchar.h>
#include "atlsafe.h"
#include <vector>
#include "../firefox/dialogs-win.h"
#include "EstEIDPin2Dlg.h"
#include "EstEIDPinPadDlg.h"
extern "C" {
#include "l10n.h"
#include "../firefox/certselection-win.h"
#include "esteid_dialog_common.h"
#include "dialogs.h"
#include "esteid_log.h"
#include "esteid_sign.h"
#include "esteid_error.h"
}
#define FAIL_IF_SITE_IS_NOT_ALLOWED if(!isSiteAllowed()){return Error((this->errorMessage).c_str());}

#pragma comment(lib, "comsuppw")  // ??

// CEstEIDIEPluginBHO
extern "C" {
extern int EstEID_errorCode;
}
#ifdef WIN_XP
CEstEidPinPadDlg *pinPadDlg;
#endif
EstEID_PINPromptData pinPromptData = {promptForPIN, showAlert, NULL};

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
		this->errorMessage = getErrorMessage(this->errorCode);
		EstEID_log("Protocol not allowed");
	}
	SysFreeString(url_buffer);	

	return allowed;
}

STDMETHODIMP CEstEIDIEPluginBHO::getCertificate(IDispatch **_certificate){
	EstEID_log("");
	FAIL_IF_SITE_IS_NOT_ALLOWED
	EstEID_setLocale(CW2A(this->language));
	try {
		if(!this->certificate || !isSameCardInReader(this->certificate)) {
			this->certificate.CoCreateInstance(CLSID_EstEIDCertificate);
		}
		
		CComPtr<IEstEIDCertificate> cert;
		
		this->certificate.CopyTo(&cert);

		*_certificate = cert.Detach();
	}
	catch(CryptoException e) {
		mapInternalErrorCodes(e.windowsErrorCode);
		return Error((this->errorMessage).c_str());
	}
	setError(ESTEID_NO_ERROR);
	
	return S_OK;
}

BOOL CEstEIDIEPluginBHO::isSameCardInReader(CComPtr<IEstEIDCertificate> _cert){ //todo: must check is card changed
	/* not implemented yet */
	this->certificate = NULL;
	return false;
}

void showAlert(void *nativeWindowHandle, const char *message) {
	MessageBox((HWND)nativeWindowHandle, CA2W(message), CA2W(l10n("Error")), MB_OK | MB_ICONERROR);
}

char* promptForPIN(void* nativeWindowHandle, const char* name, const char* message, unsigned minPin2Length, int usePinPad) {
	LOG_LOCATION;
#ifdef WIN_XP
	if(usePinPad){
		pinPadDlg = new CEstEidPinPadDlg();
		pinPadDlg->DoModal();

	}
#endif
	return "";
}


void CEstEIDIEPluginBHO::signWithPKCS11(HINSTANCE hInst, BSTR id, BSTR hash, BSTR *signature) {
	LOG_LOCATION;
	char *_signature = NULL;
	EstEID_errorCode = 0;

	CK_SLOT_ID slotId;	
	if(id == NULL) {
		throw CryptoException(ESTEID_CERT_NOT_FOUND_ERROR);
	}
	if(EstEID_getSlotId(CW2A(id), &slotId)){
		if(EstEID_isPinPad(slotId)) {
#ifdef WIN_XP
			_signature = EstEID_sign(strdup(CW2A(id)), strdup(CW2A(hash)), pinPromptData);
			if(pinPadDlg != NULL) {
				pinPadDlg->EndDialog(ESTEID_NO_ERROR);
			}
#endif
		}
		else {
			CEstEidPin2Dlg *pin2Dlg = new CEstEidPin2Dlg();
			pin2Dlg->SetCertId(id);
			pin2Dlg->SetHash(hash);
			INT_PTR result = pin2Dlg->DoModal();
			if(!result) {
				throw CryptoException(ESTEID_USER_CANCEL);
			}
			_signature = pin2Dlg->GetSignature();
		}
		if(_signature == NULL) {
			throw CryptoException(EstEID_errorCode == 0 ? ESTEID_PKCS11_ERROR : EstEID_errorCode);
		}
	}
	else {
		throw CryptoException(ESTEID_CERT_NOT_FOUND_ERROR);
	}
	if (_signature) {
		*signature = _bstr_t(_signature).Detach();
		free(_signature);
	}
	else {
		//TODO:error handling
		//EstEID_log("EstEID_error=%s", EstEID_error);
		throw CryptoException(ESTEID_UNKNOWN_ERROR);
	}
}


void CEstEIDIEPluginBHO::signWithCSP(BSTR id, BSTR hash, BSTR *signature) {
	LOG_LOCATION;

	HCRYPTPROV cryptoProvider = NULL;
	HCRYPTHASH _hash = NULL ;
	PCCERT_CONTEXT certContext = NULL;
	HCERTSTORE cert_store;
	BOOL must_release_provider = false;		

	try{
		if(!id || !strlen(CW2A(id))) {
			throw CryptoException(ESTEID_CERT_NOT_FOUND_ERROR);
		}
		EstEID_log("signing started, selected certificate id = %s", CW2A(id));	

#define VALID_HASH_LENGTH 40
		if (_bstr_t(hash).length() != VALID_HASH_LENGTH) {
			throw CryptoException("invalid hash");
		}

		cert_store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
		if(!cert_store) throw CryptoException();
		
		while(certContext = CertFindCertificateInStore(cert_store, X509_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, certContext)) {
			if(certificateMatchesId(certContext, id)) {
				DWORD key_type = 0;
				if (CryptAcquireCertificatePrivateKey(certContext, CRYPT_ACQUIRE_CACHE_FLAG,
					NULL, &cryptoProvider, &key_type, &must_release_provider)) {
						BYTE hashBytes[21];
						DWORD hashBytesLength = 20;
						
						CryptStringToBinary(hash, VALID_HASH_LENGTH, CRYPT_STRING_HEX, hashBytes, &hashBytesLength, 0, 0);
						hashBytes[hashBytesLength] = '\0';
						EstEID_log("Number of bytes stored in hashBytes buffer = %u", hashBytesLength);

						CryptoErrorHandler(CryptCreateHash(cryptoProvider, CALG_SHA1, 0, 0, &_hash));
						EstEID_log("CryptCreateHash() set hash object pointer to %p", _hash);  

						CryptoErrorHandler(CryptSetHashParam(_hash, HP_HASHVAL, hashBytes, 0));

						#define SIGNATURE_LENGTH 1024 
						BYTE _signature[SIGNATURE_LENGTH];
						DWORD signatureLength = SIGNATURE_LENGTH;
						INT retCode = CryptSignHash(_hash, AT_SIGNATURE, NULL, 0, _signature, &signatureLength);
						DWORD lastError = GetLastError();
						EstEID_log("CryptSignHash() return code: %u (%s) %x", retCode, retCode ? "SUCCESS" : "FAILURE", lastError);
						if(!retCode) throw CryptoException(lastError);

						CryptDestroyHash(_hash);
						CryptReleaseContext(cryptoProvider, 0);	
						CertFreeCertificateContext(certContext);
						if(must_release_provider) CertCloseStore(cert_store, 0);

						std::stringstream ss;
						for (DWORD i = 0; i < signatureLength; i++) {
							ss << std::hex << std::setfill('0') << std::setw(2) << (short)_signature[signatureLength - i - 1];
						}

						*signature = _bstr_t(ss.str().c_str()).Detach();
				}
				else {
					INT lastError = GetLastError();
					EstEID_log("ERROR: CryptAcquireCertificatePrivateKey() failed, error code = %lXh", GetLastError());
					switch(lastError) {
					case NTE_BAD_PUBLIC_KEY:
						EstEID_log("       error code %lXh: NTE_BAD_PUBLIC_KEY", lastError);
						break;
					case NTE_SILENT_CONTEXT:
						EstEID_log("       error code %lXh: NTE_SILENT_CONTEXT", lastError);
					}
				}				
				break;
			}
		}	
	}
	catch(CryptoException e) {
		*signature = _bstr_t("").Detach();
		if(_hash) CryptDestroyHash(_hash);
		if(cryptoProvider) CryptReleaseContext(cryptoProvider, 0);
		if(certContext) CertFreeCertificateContext(certContext);
		if(must_release_provider && cert_store) CertCloseStore(cert_store, 0);

		throw(CryptoException(e.windowsErrorCode));
	}
}

void CEstEIDIEPluginBHO::signWithCNG(BSTR id, BSTR hash, BSTR *signature) {
	LOG_LOCATION;
#ifdef WIN_XP
	EstEID_log("WARNING: CNG is not supported for windows XP");
#else
	int methodResult = true;


	#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)
	SECURITY_STATUS secStatus = ERROR_SUCCESS;
	NCRYPT_KEY_HANDLE hKey = NULL;
	DWORD cbSignature = 0;
	NTSTATUS status = ((NTSTATUS)0xC0000001L);
	PBYTE pbSignature = NULL;
	PCCERT_CONTEXT certContext = NULL;
	HCERTSTORE cert_store;
	BOOL must_release_provider;		
		
	int hashHexLength = _bstr_t(hash).length()/2;

	BCRYPT_PKCS1_PADDING_INFO padInfo;
	padInfo.pszAlgId = 0;
	switch(hashHexLength) {
		case BINARY_SHA1_LENGTH:
			padInfo.pszAlgId = NCRYPT_SHA1_ALGORITHM;break; 
		case BINARY_SHA224_LENGTH:
			padInfo.pszAlgId = L"SHA224"; break;
		case BINARY_SHA256_LENGTH :
			padInfo.pszAlgId = NCRYPT_SHA256_ALGORITHM; break;
		case BINARY_SHA512_LENGTH:
			padInfo.pszAlgId = NCRYPT_SHA512_ALGORITHM; break;
		default:
			break; 
	}
	
	try {
		if(!id || !strlen(CW2A(id))) {
			throw CryptoException(ESTEID_CERT_NOT_FOUND_ERROR);
		}
		EstEID_log("signing started, selected certificate id = %s", CW2A(id));	

		if(padInfo.pszAlgId == 0) {
			throw CryptoException(ESTEID_INVALID_HASH_ERROR);
		}


		cert_store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
		if(!cert_store) throw CryptoException();

		while(certContext = CertFindCertificateInStore(cert_store, X509_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, certContext)) {
			if(certificateMatchesId(certContext, id)) {
				if (!CryptAcquireCertificatePrivateKey(certContext, CRYPT_ACQUIRE_ONLY_NCRYPT_KEY_FLAG|CRYPT_ACQUIRE_COMPARE_KEY_FLAG, NULL, &hKey,
					NULL, &must_release_provider)) {
					throw CryptoException(ESTEID_CRYPTO_API_ERROR);
				}

				BYTE hashBytes[65];
				CryptStringToBinary(hash, hashHexLength*2, CRYPT_STRING_HEX, hashBytes, (DWORD*)&hashHexLength, 0, 0);
				EstEID_log("Number of bytes stored in hashBytes buffer = %u", hashHexLength);
		
				EstEID_log("signing with %s", CW2A(padInfo.pszAlgId));
				if(FAILED(secStatus = NCryptSignHash(hKey, &padInfo, (PBYTE)hashBytes, hashHexLength, NULL, 0, &cbSignature, 0))) {
					throw CryptoException(secStatus);
				}

				pbSignature = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbSignature);
				if(NULL == pbSignature){
					throw CryptoException(ESTEID_CRYPTO_API_ERROR);
				}
				
				if(FAILED(secStatus = NCryptSignHash(hKey, &padInfo, (PBYTE)hashBytes, hashHexLength, pbSignature, cbSignature, &cbSignature, BCRYPT_PAD_PKCS1))) {
					throw CryptoException(secStatus);
				}

				std::stringstream ss;
				for (DWORD i = 0; i < cbSignature; i++) {
					ss << std::hex << std::setfill('0') << std::setw(2) << (short)pbSignature[i];
				}
				*signature = _bstr_t(ss.str().c_str()).Detach();
			}
		}
	}
	catch(CryptoException e) {
		*signature = _bstr_t("").Detach();
		if(pbSignature) HeapFree(GetProcessHeap(), 0, pbSignature);
		if(hKey) NCryptFreeObject(hKey);
		if(certContext) CertFreeCertificateContext(certContext);
		if(must_release_provider && cert_store) CertCloseStore(cert_store, 0);

		throw CryptoException(e.windowsErrorCode);
	}
	CertFreeCertificateContext(certContext);
	if(must_release_provider) CertCloseStore(cert_store, 0);
#endif

}

STDMETHODIMP CEstEIDIEPluginBHO::sign(BSTR id, BSTR hash, BSTR language, BSTR *signature){
	LOG_LOCATION;

	FAIL_IF_SITE_IS_NOT_ALLOWED;

	try
	{
		EstEID_setLocale(CW2A(this->language));

		if (canUseCNG(id))
		{
			EstEID_log("CNG can be used with selected certificate");
			signWithCNG(id, hash, signature);
			EstEID_log("CNG sign OK");
		}
		else
		{
			try
			{
				EstEID_log("CNG cannot be used with selected certificate. Using CAPI.");
				signWithCSP(id, hash, signature);
				EstEID_log("CAPI sign OK");
			}
			catch (CryptoException e)
			{
				EstEID_log("CAPI signing failed. Trying signing over PKCS11 interface.");
				signWithPKCS11(NULL, id, hash, signature);
				EstEID_log("PKCS11 sign OK");
			}
		}
	}
	catch (CryptoException e)
	{
		EstEID_log("CryptoException caught during signing!");
		if(e._reason.compare("User cancel")){
			setError(ESTEID_USER_CANCEL);
			EstEID_log("CryptoExcepton reason - user cancel");
		}
		mapInternalErrorCodes(e.windowsErrorCode);
		return Error((this->errorMessage).c_str());
	}
	setError(ESTEID_NO_ERROR);
	EstEID_log("Signing ended");
	return S_OK;
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

BOOL CEstEIDIEPluginBHO::isWinVistaOrLater() {
    DWORD version = GetVersion();
    DWORD major = (DWORD) (LOBYTE(LOWORD(version)));
    DWORD minor = (DWORD) (HIBYTE(LOWORD(version)));

    return (major > 6) || ((major == 6) && (minor >= 0));
}

BOOL CEstEIDIEPluginBHO::canUseCNG(BSTR id)
{
	HCERTSTORE cert_store;
	PCCERT_CONTEXT certContext = NULL;
	NCRYPT_KEY_HANDLE hKey = NULL;
	BOOL must_release_provider;

	if (isCNGInstalled())
	{
		if (!id || !strlen(CW2A(id)))
		{
			throw CryptoException(ESTEID_CERT_NOT_FOUND_ERROR);
		}

		cert_store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
		if (!cert_store) throw CryptoException();

		while (certContext = CertFindCertificateInStore(cert_store, X509_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, certContext))
		{
			if (certificateMatchesId(certContext, id))
			{
				if (!CryptAcquireCertificatePrivateKey(certContext, CRYPT_ACQUIRE_ONLY_NCRYPT_KEY_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG | CRYPT_ACQUIRE_SILENT_FLAG, NULL, &hKey, NULL, &must_release_provider))
				{
					CertFreeCertificateContext(certContext);
					if (must_release_provider) CertCloseStore(cert_store, 0);
					return false;
				}
				else
				{
					CertFreeCertificateContext(certContext);
					if (must_release_provider) CertCloseStore(cert_store, 0);
					return true;
				}
			}
		}
	}
	CertFreeCertificateContext(certContext);
	if (must_release_provider) CertCloseStore(cert_store, 0);
	return false;
}

BOOL CEstEIDIEPluginBHO::isCNGInstalled() {
	HKEY hKey;
	long result = 0;
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards", 0, KEY_READ, &hKey);
	if(result != ERROR_SUCCESS) {
		return true;
	}
	
	DWORD subkeyIndex = 0;
	WCHAR subkeyValue[512];
	DWORD subkeySize = sizeof(subkeyValue);
	
	result = ERROR_SUCCESS;
	WCHAR driverName[64];
	DWORD driverNameSize = sizeof(driverName);
	while(result != ERROR_NO_MORE_ITEMS) {
		result = RegEnumKey(hKey, subkeyIndex++, subkeyValue, subkeySize);
		if(wcsncmp(subkeyValue, L"EstEID National ID Card", wcslen(L"EstEID National ID Card")) == 0) {
		
			HKEY driverSubkey;
			
			long errorCode = RegOpenKeyEx(hKey, subkeyValue, 0, KEY_READ, &driverSubkey);
			if(errorCode != ERROR_SUCCESS) {
				return true;
			}
			errorCode = RegQueryValueEx(driverSubkey, L"Crypto Provider", NULL, NULL, (LPBYTE) driverName, &driverNameSize );
			if( errorCode != ERROR_SUCCESS){
				driverName[0] = '\0';
			}

			RegCloseKey(driverSubkey);
			EstEID_log("driverName = %s", CW2A(driverName));
			return wcscmp(driverName, L"EstEID Card CSP") != 0;
		}
	}
	
	RegCloseKey(hKey);
	return true;
}

void CEstEIDIEPluginBHO::CryptoErrorHandler(BOOL result) {
	EstEID_log("");
	if (!result) {		
		EstEID_log("Throwing CryptoException");
		throw CryptoException();
	}
}

void CEstEIDIEPluginBHO::mapInternalErrorCodes(unsigned int code) {
	EstEID_log("Mapping error code %Xh:", code, code);

	if(code == ESTEID_USER_CANCEL || code == 0x3E5 || code == 0xB7 || code == 0x05DE || code == SCARD_W_CANCELLED_BY_USER){ // 0x03E5 for ???; 0x05DE for SL base package; SCARD_W_CANCELLED_BY_USER (0x8010006E) for SK base package
		EstEID_log("    mapped to ESTEID_USER_CANCEL (%Xh)", ESTEID_USER_CANCEL);
		setError(ESTEID_USER_CANCEL);
	}
	else if(code == 0x80092004){		
		EstEID_log("    mapped to ESTEID_CERT_NOT_FOUND_ERROR (%Xh)", ESTEID_CERT_NOT_FOUND_ERROR);
		setError(ESTEID_CERT_NOT_FOUND_ERROR);
	}
	else if(code > 0x80000000) {		
		EstEID_log("    mapped to ESTEID_CRYPTO_API_ERROR (%Xh)", ESTEID_CRYPTO_API_ERROR);
		setError(ESTEID_CRYPTO_API_ERROR);
	}
	else {
		EstEID_log("    mapped to ESTEID_UNKNOWN_ERROR (%Xh)", ESTEID_UNKNOWN_ERROR);
		setError(ESTEID_UNKNOWN_ERROR);
	}
}

void CEstEIDIEPluginBHO::setError(unsigned int code)
{
	EstEID_log("");
	this->errorCode = code;
	this->errorMessage.assign(getErrorMessage(code));
	EstEID_log("Set error: %s (HEX %Xh, DEC %u)", this->errorMessage.c_str(), code, code);
}
