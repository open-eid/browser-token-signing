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

#include <stdlib.h>
#include <string.h>
#include "plugin-class.h"
#include "version.h"
#include "cert-class.h"
#include "esteid_time.h"
#include "l10n.h"
#include "esteid_error.h"
#include "npapi.h"
#include "esteid_certinfo.h"
#include "esteid_sign.h"
#include <ncrypt.h>
#include "cert_dialog_win.h"
#include "atlsafe.h"

#define VALID_HASH_LENGTH 40

extern HINSTANCE pluginInstance;

extern NPNetscapeFuncs* browserFunctions;
extern char EstEID_error[1024];
extern int EstEID_errorCode;

bool allowedSite;
char pluginLanguage[3];

bool isAllowedSite() {
	if(!allowedSite) {
		sprintf(EstEID_error, "Site is not allowed");
		EstEID_errorCode = ESTEID_SITE_NOT_ALLOWED;
		EstEID_log("called from forbidden site");
		return false;
	}
	return true;
}

bool pluginInvokeDefault(PluginInstance *obj, NPVariant *args, unsigned argCount, NPVariant *result) {
	return false;
}

void pluginInvalidate() {
}

bool pluginSetProperty(PluginInstance *obj, NPIdentifier name, const NPVariant *variant) {
	LOG_LOCATION;
	if(isSameIdentifier(name, "pluginLanguage")) {
		memset(pluginLanguage, '\0', 3);
		if(NPVARIANT_IS_STRING(*variant)) {
			strncpy(pluginLanguage, NPVARIANT_TO_STRING(*variant).UTF8Characters, 2);
			return true;
		}
		return true;
	}
	return false;
}

bool pluginHasMethod(PluginInstance *obj, NPIdentifier name) {
	return 
		isSameIdentifier(name, "sign") ||
		isSameIdentifier(name, "getCertificate") ||		
		isSameIdentifier(name, "getVersion");
}

bool pluginHasProperty(NPClass *theClass, NPIdentifier name) {
	return 
		isSameIdentifier(name, "version") ||
		isSameIdentifier(name, "errorMessage") ||
		isSameIdentifier(name, "errorCode") ||
		isSameIdentifier(name, "authCert") ||
		isSameIdentifier(name, "pluginLanguage") ||
		isSameIdentifier(name, "signCert");
}

void *getNativeWindowHandle(PluginInstance *obj) {
	void *nativeWindowHandle = obj->nativeWindowHandle;
	EstEID_log("_WIN32 detected, forcing nativeWindowHandle query from browser");
	nativeWindowHandle = 0;
	if (!nativeWindowHandle) {
		browserFunctions->getvalue(obj->npp, NPNVnetscapeWindow, &nativeWindowHandle);
		EstEID_log("reading nativeWindowHandle=%p from browserFunctions", nativeWindowHandle);
	}
	else {
		EstEID_log("reading nativeWindowHandle=%p from PluginInstance", nativeWindowHandle);
	}
	return nativeWindowHandle;
}

int getHashAlgorithm(int hashHexLength) {
	int hashAlgorithm = -1;

	switch (hashHexLength) {
	case BINARY_SHA1_LENGTH * 2:
		hashAlgorithm = CALG_SHA1;
		break;
	case BINARY_SHA256_LENGTH * 2:
		hashAlgorithm = CALG_SHA_256;
		break;
	case BINARY_SHA512_LENGTH * 2:
		hashAlgorithm = CALG_SHA_512;
		break;
	}
	if (hashAlgorithm == -1) {
		sprintf(EstEID_error, "invalid incoming hash length: %i", hashHexLength);
		EstEID_log(EstEID_error);
		EstEID_errorCode = ESTEID_INVALID_HASH_ERROR;
		return -1;
	}
	return hashAlgorithm;
}

char *getLanguageFromOptions(PluginInstance *obj, NPVariant options) {
	LOG_LOCATION;
	NPObject *object = NPVARIANT_TO_OBJECT(options);
	NPIdentifier identifier = browserFunctions->getstringidentifier("language");
	NPVariant languageResult;
	if (browserFunctions->getproperty(obj->npp, object, identifier, &languageResult) && NPVARIANT_IS_STRING(languageResult)) {
		char *language = createStringFromNPVariant(&languageResult);
		EstEID_log("returning language '%s'", language);
		return language;
	}
	EstEID_log("unable to read language from options, returning empty string");
	return "";
}

BOOL isCNGInstalled() {
	HKEY hKey;
	long result = 0;
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards", 0, KEY_READ, &hKey);
	if (result != ERROR_SUCCESS) {
		return true;
	}

	DWORD subkeyIndex = 0;
	WCHAR subkeyValue[512];
	DWORD subkeySize = sizeof(subkeyValue);

	result = ERROR_SUCCESS;
	WCHAR driverName[64];
	DWORD driverNameSize = sizeof(driverName);
	while (result != ERROR_NO_MORE_ITEMS) {
		result = RegEnumKey(hKey, subkeyIndex++, subkeyValue, subkeySize);
		if (wcsncmp(subkeyValue, L"EstEID National ID Card", wcslen(L"EstEID National ID Card")) == 0) {

			HKEY driverSubkey;

			long errorCode = RegOpenKeyEx(hKey, subkeyValue, 0, KEY_READ, &driverSubkey);
			if (errorCode != ERROR_SUCCESS) {
				return true;
			}
			errorCode = RegQueryValueEx(driverSubkey, L"Crypto Provider", NULL, NULL, (LPBYTE)driverName, &driverNameSize);
			if (errorCode != ERROR_SUCCESS){
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

BOOL canUseCNG(char* id)
{
	HCERTSTORE cert_store;
	PCCERT_CONTEXT certContext = NULL;
	NCRYPT_KEY_HANDLE hKey = NULL;
	BOOL must_release_provider = FALSE;

	if (isCNGInstalled())
	{
		if (!id || !strlen(id))
		{
			return false;
		}

		cert_store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
		if (!cert_store) return false;;

		while (certContext = CertFindCertificateInStore(cert_store, X509_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, certContext))
		{
			if (certificateMatchesId(certContext, id))
			{
				if (!CryptAcquireCertificatePrivateKey(certContext, CRYPT_ACQUIRE_ONLY_NCRYPT_KEY_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG, NULL, &hKey, NULL, &must_release_provider))
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

bool doSignCNG(PluginInstance *obj, BCRYPT_PKCS1_PADDING_INFO padInfo, char *hash, int hashHexLength, char **signature)
{
	NCRYPT_KEY_HANDLE hKey = NULL;
	SECURITY_STATUS secStatus = ERROR_SUCCESS;
	DWORD cbSignature = 0;
	PBYTE pbSignature = NULL;

	LOG_LOCATION;
	if (!CryptAcquireCertificatePrivateKey(obj->certContext, CRYPT_ACQUIRE_ONLY_NCRYPT_KEY_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG, NULL, &hKey, NULL, NULL)) {
		LOG_LOCATION;
		EstEID_log("**** Error 0x%x returned by CryptAcquireCertificatePrivateKey\n", GetLastError());
		handleError("CryptAcquireCertificatePrivateKey", obj);
		return false;
	}

	LOG_LOCATION;
	HWND window = (HWND)getNativeWindowHandle(obj);
	NCryptSetProperty(hKey, NCRYPT_WINDOW_HANDLE_PROPERTY, (PBYTE)window, NULL, NULL);
	LOG_LOCATION;

	if (FAILED(secStatus = NCryptSignHash(hKey, &padInfo, (PBYTE)hash, hashHexLength, NULL, 0, &cbSignature, 0))) {
		LOG_LOCATION;
		handleErrorWithCode(secStatus, "NCryptSignHash", obj);
		return false;
	}

	LOG_LOCATION;
	//allocate the signature buffer
	pbSignature = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbSignature);
	if (NULL == pbSignature){
		handleErrorWithCode(ESTEID_CRYPTO_API_ERROR, "HeapAlloc", obj);
		return false;
	}
	LOG_LOCATION;
	if (FAILED(secStatus = NCryptSignHash(hKey, &padInfo, (PBYTE)hash, hashHexLength, pbSignature, cbSignature, &cbSignature, BCRYPT_PAD_PKCS1))) {
		LOG_LOCATION;
		EstEID_log("**** Error 0x%x returned by NCryptSignHash\n", secStatus);
		handleErrorWithCode(secStatus, "NCryptSignHash", obj);
		return false;
	}

	LOG_LOCATION;
	*signature = byteToChar(pbSignature, cbSignature);

	EstEID_log("Signature: %s", *signature);
	
	LOG_LOCATION;
	if (pbSignature) HeapFree(GetProcessHeap(), 0, pbSignature);
	if (hKey) NCryptFreeObject(hKey);
	return true;
}

bool doSignCSP(PluginInstance *obj, BCRYPT_PKCS1_PADDING_INFO padInfo, char *inHash, int hashHexLength, char **signature)
{
	WCHAR* hash = NULL;
	int wHashLen = 0;
	BOOL must_release_provider = false;
	HCRYPTPROV cryptoProvider = NULL;
	DWORD key_type = 0;
	HCRYPTHASH _hash = NULL;

	int hashAlgorithm = getHashAlgorithm(hashHexLength*2);
	if (hashAlgorithm == -1) {
		browserFunctions->setexception(&obj->header, EstEID_error);
		return false;
	}

	if (CryptAcquireCertificatePrivateKey(obj->certContext, 0, NULL, &cryptoProvider, &key_type, &must_release_provider))
	{
		EstEID_log("must_release_provider = %i", must_release_provider);

		BYTE* hashBytes = (BYTE*)malloc(sizeof(BYTE)*hashHexLength);
		memcpy(hashBytes, inHash, sizeof(BYTE)*hashHexLength);

		for (int i = 0; i < sizeof(hashBytes); i++)
			EstEID_log("hashBytes: %02X", hashBytes[i]);

		EstEID_log("_hash = %p, cryptoProvider = %p", _hash, cryptoProvider);
		if (!CryptCreateHash(cryptoProvider, hashAlgorithm, 0, 0, &_hash)){
			handleError("CryptCreateHash", obj);
			free(hashBytes);
			return false;
		}
		EstEID_log("CryptCreateHash() set hash object pointer to %p", _hash);

		if (!CryptSetHashParam(_hash, HP_HASHVAL, hashBytes, 0)){
			handleError("CryptSetHashParam", obj);
			return false;
		}

		BYTE         *pbHash;
		DWORD        dwHashLen;
		DWORD        dwHashLenSize = sizeof(DWORD);
		CryptGetHashParam(_hash, HP_HASHSIZE, (BYTE *)&dwHashLen, &dwHashLenSize, 0);
		pbHash = (BYTE*)malloc(dwHashLen);
		CryptGetHashParam(_hash, HP_HASHVAL, pbHash, &dwHashLen, 0);
		EstEID_log("Hash len in bytes: %i", dwHashLen);
		for (int i = 0; i < dwHashLen; i++)
			EstEID_log("hashBytes: %02X", pbHash[i]);

#define SIGNATURE_LENGTH 1024

		BYTE _signature[SIGNATURE_LENGTH];
		DWORD signatureLength = SIGNATURE_LENGTH;
		if (!CryptSignHash(_hash, AT_SIGNATURE, NULL, 0, _signature, &signatureLength))
		{
			DWORD err = GetLastError();
			EstEID_log("CryptSignHash() AT_SIGNATURE ended with ERROR 0x%08X", err);

			if (err == NTE_NO_KEY)
			{
				if (!CryptSignHash(_hash, AT_KEYEXCHANGE, NULL, 0, _signature, &signatureLength))
				{
					EstEID_log("CryptSignHash() AT_KEYEXCHANGE ended with ERROR 0x%08X", GetLastError());
					handleError("CryptSignHash AT_KEYEXCHANGE", obj);
					return false;
				}
				else
					EstEID_log("CryptSignHash() AT_KEYEXCHANGE ended with OK");
			}
			else
			{
				handleError("CryptSignHash AT_SIGNATURE", obj);
				return false;
			}
		}
		else
			EstEID_log("CryptSignHash() AT_SIGNATURE ended with SUCCESS");

		CryptDestroyHash(_hash);
		if (must_release_provider) {
			CryptReleaseContext(cryptoProvider, 0);
		}

		reverseBytes(_signature, signatureLength);
		*signature = byteToChar(_signature, signatureLength);
		free(hashBytes);
	}
	else
	{
		if (hash) free(hash);
		handleError("CryptAcquireCertificatePrivateKey", obj);
		return false;
	}

	return true;
}

bool doSign(PluginInstance *obj, NPVariant *args, unsigned argCount, NPVariant *result)
{
	EstEID_log("obj=%p, obj->certContex=%p, name=sign argCount=%u", obj, obj->certContext, argCount);
	int methodResult = true;
	
	#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)
	
	
	
	NTSTATUS status = ((NTSTATUS)0xC0000001L);
	
	char *hash = NULL;
	char *certId = NULL;
	HCERTSTORE cert_store = NULL;
	PCCERT_CONTEXT certContext = NULL;

	FAIL_IF_NOT_ALLOWED_SITE;
	
	if(argCount > 2 && NPVARIANT_IS_OBJECT(args[2])){
		strncpy(pluginLanguage, getLanguageFromOptions(obj, args[2]), 2);
	}
	certId = createStringFromNPVariant(&args[0]);
	if (obj->certContext == NULL) {
		cert_store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
		if(!cert_store){
			sprintf(EstEID_error, "CertOpenStore failed");
			EstEID_log(EstEID_error);
			EstEID_errorCode = ESTEID_CRYPTO_API_ERROR;
			browserFunctions->setexception(&obj->header, EstEID_error);
		}		
		else {
			while(certContext = CertFindCertificateInStore(cert_store, X509_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, certContext)) {
				EstEID_log("certificateMatchesId started: %s", certId);
				if(certificateMatchesId(certContext, certId)) {
					obj->certContext = certContext;
					break;
				}
			}
		}
		if (obj->certContext == NULL) {
			sprintf(EstEID_error, "No certificate selected");
			EstEID_log("EstEID_error=%s", EstEID_error);
			EstEID_errorCode = ESTEID_CERT_NOT_FOUND_ERROR;
			browserFunctions->setexception(&obj->header, EstEID_error);
			return false;
		}
	}	

	if (argCount < 2) {
		browserFunctions->setexception(&obj->header, "Missing arguments");
		sprintf(EstEID_error, "Missing arguments");
		EstEID_log(EstEID_error);
		EstEID_errorCode = ESTEID_CRYPTO_API_ERROR;
		browserFunctions->setexception(&obj->header, EstEID_error);
		return false;
	}
	
	EstEID_setLocale(pluginLanguage);

	int hashHexLength = strlen(createStringFromNPVariant(&args[1]))/2;

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

	if(padInfo.pszAlgId == 0) {
		sprintf(EstEID_error, "invalid incoming hash length: %i", hashHexLength);
		EstEID_log(EstEID_error);
		EstEID_errorCode = ESTEID_INVALID_HASH_ERROR;
		browserFunctions->setexception(&obj->header, EstEID_error);
		return false;
	}

	hash = EstEID_hex2bin(createStringFromNPVariant(&args[1]));
	
	//Convert certId from char * to BSTR
	/*WCHAR* wCertID = NULL;
	int wHashLen = 0;

	int certIDHexLength = strlen(certID);
	wHashLen = MultiByteToWideChar(CP_ACP, 0, certId, -1, NULL, 0);
	hash = new WCHAR[hashHexLength];
	MultiByteToWideChar(CP_ACP, 0, certId, -1, (LPWSTR)wCertID, certIDHexLength);*/


	LOG_LOCATION;
	char *signature = "";

	if (canUseCNG(certId))
	{
		EstEID_log("CNG can be used with selected certificate");
		if (!doSignCNG(obj, padInfo, hash, hashHexLength, &signature))
		{
			EstEID_log("CNG signing. Using CAPI.");
			methodResult = false;
			goto Cleanup;
		}
		else
		{
			EstEID_log("CNG sign OK");
			methodResult = true;
			EstEID_log("Returned signature: %s", signature);
		}
	}
	else
	{
		EstEID_log("CNG cannot be used with selected certificate. Using CAPI.");
		if (!doSignCSP(obj, padInfo, hash, hashHexLength, &signature))
		{
			EstEID_log("CAPI signing failed.");
			methodResult = false;
			goto Cleanup;
		}
		else
		{
			EstEID_log("CAPI sign OK");
			methodResult = true;
			EstEID_log("Returned signature: %s", signature);
		}
	}
	

	LOG_LOCATION;
	copyStringToNPVariant(signature, result);
	LOG_LOCATION;
	if (signature)
	{
		LOG_LOCATION;
		free(signature);
		LOG_LOCATION;
	}
	LOG_LOCATION;

Cleanup:
	
	LOG_LOCATION;
	if(hash) free(hash);
	if(certId) free(certId);
	EstEID_log("Signing ended");
	return methodResult;
}

bool certificateMatchesId(PCCERT_CONTEXT certContext, char* id) {
	BYTE *cert;
	cert = (BYTE*)malloc(certContext->cbCertEncoded + 1);
	memcpy(cert, certContext->pbCertEncoded, certContext->cbCertEncoded);
	cert[certContext->cbCertEncoded] = '\0';

	char *hashAsString = EstEID_getCertHash((char*)cert);
	free(cert);

	BOOL result = (strcmp(hashAsString, id) == 0);

	EstEID_log("Cert match check result: %s", result ? "matches" : "does not match");

	return result;
}

bool doGetCertificate(PluginInstance *obj, NPVariant *result) {
	LOG_LOCATION;
	FAIL_IF_NOT_ALLOWED_SITE;
	PCCERT_CONTEXT cert_context;
	
	if(!selectCertificate(obj, &cert_context)) {
		return false;
	}
	EstEID_log("Cerftificate context=%p", cert_context);

	CertInstance *certInstance = (CertInstance *)browserFunctions->createobject(obj->npp, certClass());
	certInstance->npp = obj->npp;
	obj->certContext = cert_context;
	EstEID_log("Cerftificate context=%p", obj->certContext);

	char* certificate = (char*)malloc(cert_context->cbCertEncoded + 1);
	LOG_LOCATION
	memcpy(certificate, cert_context->pbCertEncoded, cert_context->cbCertEncoded);
	LOG_LOCATION
	certificate[cert_context->cbCertEncoded] = '\0';
	EstEID_log("certificate binary length = %i", cert_context->cbCertEncoded);
	certInstance->certificate = strdup(certificate);
	certInstance->certificateAsHex = EstEID_bin2hex(certificate, cert_context->cbCertEncoded);
	
	free(certificate);

	TCHAR cn[64];
	CertGetNameString(cert_context, CERT_NAME_ATTR_TYPE, 0, szOID_COMMON_NAME, cn, 64);
	certInstance->CN = (char*)malloc(65);
	Unicode16ToAnsi(cn, certInstance->CN, 64);
	EstEID_log("Certificate CN = %s", certInstance->CN);

	CertGetNameString(cert_context, CERT_NAME_ATTR_TYPE, CERT_NAME_ISSUER_FLAG, szOID_COMMON_NAME, cn, 64);
	certInstance->issuerCN = (char*)malloc(65);
	Unicode16ToAnsi(cn, certInstance->issuerCN, 64);
	EstEID_log("Certificate issuerCN = %s", certInstance->issuerCN);

	certInstance->validFrom = dateToString(&cert_context->pCertInfo->NotBefore);
	certInstance->validTo = dateToString(&cert_context->pCertInfo->NotAfter);
	
	EstEID_log("certObject=%p", certInstance);
	OBJECT_TO_NPVARIANT((NPObject *)certInstance, *result);
	EstEID_log("result=%p", result);
	return true;
}

bool selectCertificate(PluginInstance *obj, PCCERT_CONTEXT *certContext) {
	HCERTSTORE cert_store = NULL;
	int counter = 0;
	CRYPTUI_SELECTCERTIFICATE_STRUCT sel = {sizeof(sel)};

	cert_store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
	if(!cert_store){
		EstEID_log("CertOpenStore failed");
		return false;
	}
	sel.hwndParent = (HWND)getNativeWindowHandle(obj);
	sel.pvCallbackData = &counter;
	sel.pFilterCallback = filter_proc;
	sel.rghDisplayStores = &cert_store;
	sel.cDisplayStores = 1;
	
	PCCERT_CONTEXT cert_context = CryptUIDlgSelectCertificate(&sel);
	
	EstEID_log("sel.pvCallbackData = %i", counter);
	if(!cert_context) {
		if(cert_store){
			if (!CertCloseStore(cert_store,0)){
				sprintf(EstEID_error, "CertCloseStore failed");
				EstEID_log(EstEID_error);
				EstEID_errorCode = ESTEID_CRYPTO_API_ERROR;
				browserFunctions->setexception(&obj->header, EstEID_error);
				return false;
			}
		}
		sprintf(EstEID_error, "User didn't select sertificate");
		EstEID_log(EstEID_error);
		EstEID_errorCode = ESTEID_USER_CANCEL;
		browserFunctions->setexception(&obj->header, EstEID_error);
		return false;
	}
	if(cert_store){
		if (!CertCloseStore(cert_store,0)){
			sprintf(EstEID_error, "CertCloseStore failed");
			EstEID_log(EstEID_error);
			EstEID_errorCode = ESTEID_CRYPTO_API_ERROR;
			browserFunctions->setexception(&obj->header, EstEID_error);
			return false;
		}
	}

	*certContext = cert_context;
	return true;
}

bool pluginGetProperty(PluginInstance *obj, NPIdentifier name, NPVariant *variant) {
	LOG_LOCATION;
	EstEID_log("pluginGetProperty: %s", (char *)browserFunctions->utf8fromidentifier(name));
	if (isSameIdentifier(name, "version"))
		return copyStringToNPVariant(ESTEID_PLUGIN_VERSION, variant);
	else if (isSameIdentifier(name, "errorMessage")){
		EstEID_log("Reading error message: %s", EstEID_error);
		return copyStringToNPVariant(EstEID_error, variant);
	}
	else if (isSameIdentifier(name, "errorCode")) {
		INT32_TO_NPVARIANT(EstEID_errorCode, *variant);
		EstEID_log("returning errorCode=%i", EstEID_errorCode);
		return true;
	}
	else if (isSameIdentifier(name, "authCert") || isSameIdentifier(name, "signCert")){
		return doGetCertificate(obj, variant);
	}
	else if (isSameIdentifier(name, "pluginLanguage")){
		return copyStringToNPVariant(pluginLanguage, variant);
	}
	EstEID_log("returning false");
	return false;
}

bool pluginInvoke(PluginInstance *obj, NPIdentifier name, NPVariant *args, unsigned argCount, NPVariant *result) {	
	LOG_LOCATION;
	EstEID_clear_error();
	EstEID_setLocale(pluginLanguage);

	if (isSameIdentifier(name, "sign")) {
		return doSign(obj, args, argCount, result);
	}
	if (isSameIdentifier(name, "getCertificate")) {
		return doGetCertificate(obj, result);
	}
	if (isSameIdentifier(name, "getVersion")) {
		return pluginGetProperty(obj, browserFunctions->getstringidentifier("version"), result);
	}
	EstEID_log("obj=%p, name=%p, argCount=%u", obj, name, argCount);
	return false;
}

NPObject *pluginAllocate(NPP npp, NPClass *theClass) {
	return(NPObject *)malloc(sizeof(PluginInstance));
}

void pluginDeallocate(PluginInstance *obj) {
	if(obj->certContext) {
		CertFreeCertificateContext(obj->certContext);		
	}
	free(obj);
}

static NPClass _class = {
	NP_CLASS_STRUCT_VERSION,
	(NPAllocateFunctionPtr)pluginAllocate,
	(NPDeallocateFunctionPtr)pluginDeallocate,
	(NPInvalidateFunctionPtr)pluginInvalidate,
	(NPHasMethodFunctionPtr)pluginHasMethod,
	(NPInvokeFunctionPtr)pluginInvoke,
	(NPInvokeDefaultFunctionPtr)pluginInvokeDefault,
	(NPHasPropertyFunctionPtr)pluginHasProperty,
	(NPGetPropertyFunctionPtr)pluginGetProperty,
	(NPSetPropertyFunctionPtr)pluginSetProperty,
};

NPClass *pluginClass() {
	return &_class;
}

BOOL Unicode16ToAnsi(WCHAR *in_Src, CHAR *out_Dst, INT in_MaxLen) {
    INT  lv_Len;
    
	if (in_MaxLen <= 0)
    return FALSE;

	lv_Len = WideCharToMultiByte(CP_UTF8, 0, in_Src, -1, out_Dst, in_MaxLen, NULL, NULL);
	EstEID_log("Unicode16ToAnsi = %s / %s", in_Src, out_Dst);
		
	if (lv_Len < 0)
	    lv_Len = 0;

	if (lv_Len < in_MaxLen)
	    out_Dst[lv_Len] = 0;
	else if (out_Dst[in_MaxLen-1])
		out_Dst[0] = 0;
	
	return true;
}

char* byteToChar(BYTE* signature, int length ) {
	char* buf_str = (char*) malloc (2 * length + 1);
	char* buf_ptr = buf_str;
	for (int i = 0; i < length; i++){
		buf_ptr += sprintf(buf_ptr, "%02X", (short)signature[i]);
	}
	sprintf(buf_ptr, "\0");
	
	return buf_str;
}

void reverseBytes(BYTE* signature, int signatureLength) {

	int middle = signatureLength / 2;
	for (int i = 0; i < middle; i++) {
		BYTE tmp = signature[i];
		signature[i] = signature[signatureLength - 1 - i];
		signature[signatureLength - 1 - i] = tmp;
	}
}

void handleError(char* methodName, PluginInstance *obj) {
	unsigned int errorCode = GetLastError();
	handleErrorWithCode( GetLastError(), methodName, obj);
}

void handleErrorWithCode(int errorCode, char* methodName, PluginInstance *obj) {
	sprintf(EstEID_error, "ERROR: %s failed, error code = 0x%02X", methodName, errorCode);
	EstEID_log(EstEID_error);
	if(errorCode == SCARD_W_CANCELLED_BY_USER) {
		sprintf(EstEID_error, "User cancel");
		EstEID_errorCode = ESTEID_USER_CANCEL;
	}
	else {
		EstEID_errorCode = ESTEID_CRYPTO_API_ERROR;
	}
	browserFunctions->setexception(&obj->header, EstEID_error);
}