/*
* Estonian ID card plugin for web browsers
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include "../include/pkcs11.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>
extern "C" {
#include "esteid_log.h"
}

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <WinCrypt.h>

#define BINARY_SHA1_LENGTH 20
#define BINARY_SHA224_LENGTH 28
#define BINARY_SHA256_LENGTH 32
#define BINARY_SHA384_LENGTH 48
#define BINARY_SHA512_LENGTH 64

#ifndef PKCS11_MODULE
#define PKCS11_MODULE "opensc-pkcs11.dll"
#endif

#define C(API, ...) Call(__FILE__, __LINE__, "C_"#API, fl->C_##API, __VA_ARGS__)

class UserCanceledError : public std::runtime_error {
public:
	UserCanceledError() : std::runtime_error("User canceled"){}
};

class AuthenticationError : public std::runtime_error {
public:
	AuthenticationError() : std::runtime_error("Authentication error"){}
};

class AuthenticationBadInput : public std::runtime_error {
public:
	AuthenticationBadInput() : std::runtime_error("Authentication Bad Input"){}
};

class PKCS11TokenNotRecognized : public std::runtime_error {
public:
	PKCS11TokenNotRecognized() : std::runtime_error("Token not recognized.") {}
};

class PKCS11TokenNotPresent : public std::runtime_error {
public:
	PKCS11TokenNotPresent() : std::runtime_error("Token not present.") {}
};

class PKCS11CardManager {
private:
	HINSTANCE library = 0;
	PCCERT_CONTEXT cert = NULL;
	CK_FUNCTION_LIST_PTR fl = nullptr;
	CK_TOKEN_INFO tokenInfo;
	CK_SESSION_HANDLE session = 0;
	std::vector<unsigned char> signCert;
	size_t certIndex = 0;

	template <typename Func, typename... Args>
	void Call(const char *file, int line, const char *function, Func func, Args... args) const
	{
		CK_RV rv = func(args...);
		switch (rv) {
		case CKR_OK:
			break;
		case CKR_FUNCTION_CANCELED:
			throw UserCanceledError();
		case CKR_PIN_INCORRECT:
			throw AuthenticationError();
		case CKR_PIN_LEN_RANGE:
			throw AuthenticationBadInput();
		case CKR_TOKEN_NOT_RECOGNIZED:
			throw PKCS11TokenNotRecognized();
		case CKR_TOKEN_NOT_PRESENT:
			throw PKCS11TokenNotPresent();
		default:
			throw std::runtime_error("PKCS11 method failed.");
		}
	}

	std::vector<CK_OBJECT_HANDLE> findObject(CK_OBJECT_CLASS objectClass, CK_ULONG max = 2) const {
		if (!fl) {
			throw std::runtime_error("PKCS11 is not loaded");
		}
		CK_ATTRIBUTE searchAttribute = { CKA_CLASS, &objectClass, sizeof(objectClass) };
		C(FindObjectsInit, session, &searchAttribute, 1);
		CK_ULONG objectCount = max;
		std::vector<CK_OBJECT_HANDLE> objectHandle(objectCount);
		C(FindObjects, session, objectHandle.data(), objectHandle.size(), &objectCount);
		C(FindObjectsFinal, session);
		objectHandle.resize(objectCount);
		return objectHandle;
	}

	bool isSignCertificate(const std::vector<unsigned char> &certificateCandidate) {
		bool validForSigning = false;
		if (cert = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, certificateCandidate.data(), certificateCandidate.size())) {
			EstEID_log("new certificate handle created.");
			BYTE keyUsage;
			CertGetIntendedKeyUsage(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, cert->pCertInfo, &keyUsage, 1);
			if ((keyUsage & CERT_NON_REPUDIATION_KEY_USAGE) && (CertVerifyTimeValidity(NULL, cert->pCertInfo) == 0)) {
				validForSigning = true;
			}
		}
		return validForSigning;
	}

	void findSigningCertificate() {
		std::vector<CK_OBJECT_HANDLE> certificateObjectHandle = findObject(CKO_CERTIFICATE);
		size_t certificateCount = certificateObjectHandle.size();
		EstEID_log("certificate count: %i", certificateCount);
		if (certificateObjectHandle.empty()) {
			throw std::runtime_error("Could not read cert");
		}

		for (size_t i = 0; i < certificateCount; i++) {
			EstEID_log("check cert %i", i);
			std::vector<unsigned char> certCandidate;
			CK_ATTRIBUTE attribute = { CKA_VALUE, nullptr, 0 };
			C(GetAttributeValue, session, certificateObjectHandle[i], &attribute, 1);
			certCandidate.resize(attribute.ulValueLen, 0);
			attribute.pValue = certCandidate.data();
			C(GetAttributeValue, session, certificateObjectHandle[i], &attribute, 1);
			if (isSignCertificate(certCandidate)) {
				signCert = certCandidate;
				certIndex = i;
				break;
			}
		}
	}

	PKCS11CardManager(CK_SLOT_ID slotID, CK_FUNCTION_LIST_PTR fl) : fl(fl) {
		C(GetTokenInfo, slotID, &tokenInfo);
		C(OpenSession, slotID, CKF_SERIAL_SESSION, nullptr, nullptr, &session);
		findSigningCertificate();
	}

	PKCS11CardManager(const std::string &module) {
		CK_C_GetFunctionList C_GetFunctionList = nullptr;
		library = LoadLibraryA(module.c_str());
		if (library)
			EstEID_log("library loaded");
		C_GetFunctionList = CK_C_GetFunctionList(GetProcAddress(library, "C_GetFunctionList"));
		if (!C_GetFunctionList) {
			EstEID_log("Function List not loaded");
			throw std::runtime_error("PKCS11 is not loaded");
		}
		Call(__FILE__, __LINE__, "C_GetFunctionList", C_GetFunctionList, &fl);
		EstEID_log("initializing module");
		C(Initialize, nullptr);
	}

public:

	static PKCS11CardManager* instance(const std::string &module = PKCS11_MODULE) {
		static PKCS11CardManager instance(module);
		return &instance;
	}

	~PKCS11CardManager() {
		if (session)
			C(CloseSession, session);
		if (cert)
		CertFreeCertificateContext(cert);
		if (!library)
			return;
		C(Finalize, nullptr);
		FreeLibrary(library);
	}

	std::vector<CK_SLOT_ID> getAvailableTokens() const {
		if (!fl) {
			throw std::runtime_error("PKCS11 is not loaded");
		}
		CK_ULONG slotCount = 0;
		C(GetSlotList, CK_TRUE, nullptr, &slotCount);
		EstEID_log("slotCount = %i", slotCount);
		std::vector<CK_SLOT_ID> slotIDs(slotCount, 0);
		C(GetSlotList, CK_TRUE, slotIDs.data(), &slotCount);
		std::reverse(slotIDs.begin(), slotIDs.end());
		return slotIDs;
	}

	PKCS11CardManager *getManagerForReader(CK_SLOT_ID slotId) {
		if (!fl) {
			EstEID_log("PKCS11 is not loaded");
			throw std::runtime_error("PKCS11 is not loaded");
		}
		return new PKCS11CardManager(slotId, fl);
	}

	std::vector<unsigned char> sign(const std::vector<unsigned char> &hash, const char *pin) const {
		if (!fl) {
			throw std::runtime_error("PKCS11 is not loaded");
		}
		C(Login, session, CKU_USER, (unsigned char*)pin, pin ? strlen(pin) : 0);
		std::vector<CK_OBJECT_HANDLE> privateKeyHandle = findObject(CKO_PRIVATE_KEY);
		if (privateKeyHandle.empty()) {
			throw std::runtime_error("Could not read private key");
		}
		CK_MECHANISM mechanism = { CKM_RSA_PKCS, 0, 0 };
		EstEID_log("found %i private keys in slot, using key in position %i", privateKeyHandle.size(), certIndex);
		C(SignInit, session, &mechanism, privateKeyHandle[certIndex]);
		std::vector<unsigned char> hashWithPadding;
		switch (hash.size()) {
		case BINARY_SHA1_LENGTH:
			hashWithPadding = { 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14 };
			break;
		case BINARY_SHA224_LENGTH:
			hashWithPadding = { 0x30, 0x2d, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04, 0x05, 0x00, 0x04, 0x1c };
			break;
		case BINARY_SHA256_LENGTH:
			hashWithPadding = { 0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20 };
			break;
		case BINARY_SHA384_LENGTH:
			hashWithPadding = { 0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30 };
			break;
		case BINARY_SHA512_LENGTH:
			hashWithPadding = { 0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40 };
			break;
		default:
			EstEID_log("incorrect digest length, dropping padding");
		}
		hashWithPadding.insert(hashWithPadding.end(), hash.begin(), hash.end());
		CK_ULONG signatureLength = 0;
		C(Sign, session, hashWithPadding.data(), hashWithPadding.size(), nullptr, &signatureLength);
		std::vector<unsigned char> signature(signatureLength, 0);
		C(Sign, session, hashWithPadding.data(), hashWithPadding.size(), signature.data(), &signatureLength);
		C(Logout, session);

		return signature;
	}

	bool isPinpad() const {
		return tokenInfo.flags & CKF_PROTECTED_AUTHENTICATION_PATH;
	}

	int getPIN2RetryCount() const {
		if (tokenInfo.flags & CKF_USER_PIN_LOCKED) return 0;
		if (tokenInfo.flags & CKF_USER_PIN_FINAL_TRY) return 1;
		if (tokenInfo.flags & CKF_USER_PIN_COUNT_LOW) return 2;
		return 3;
	}

	std::vector<unsigned char> getSignCert() const {
		return signCert;
	}

	bool hasSignCert() {
		return !signCert.empty();
	}
};
