/*
* Chrome Token Signing Native Host
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

#include "CngCapiSigner.h"
#include "BinaryUtils.h"
#include "HostExceptions.h"
#include "EstEIDHelper.h"
#include <Windows.h>
#include <ncrypt.h>
#include <WinCrypt.h>
#include <cryptuiapi.h>
extern "C" {
#include "esteid_log.h"
}

using namespace std;

bool certificateMatchesId(PCCERT_CONTEXT certContext, char *certId) {
	BYTE *cert;
	cert = (BYTE*)malloc(certContext->cbCertEncoded + 1);
	memcpy(cert, certContext->pbCertEncoded, certContext->cbCertEncoded);
	cert[certContext->cbCertEncoded] = '\0';

	std::string hashAsString;
	hashAsString = CEstEIDHelper::calculateMD5Hash((char*)cert);
	free(cert);

	bool result = (strcmp(hashAsString.c_str(), certId) == 0);

	EstEID_log("Cert match check result: %s", result ? "matches" : "does not match");

	return result;
}

PCCERT_CONTEXT findCertificateById(char *certId, HCERTSTORE cert_store){
	PCCERT_CONTEXT certContext = NULL;
	while (certContext = CertFindCertificateInStore(cert_store, X509_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, certContext)) {
		if (certificateMatchesId(certContext, certId)) {
			return certContext;
		}
	}
	throw NoCertificatesException();
}

string CngCapiSigner::sign() {
	EstEID_log("Signing with hash: %s, with certId: %s", getHash()->c_str(), getCertId());
	BCRYPT_PKCS1_PADDING_INFO padInfo;
	DWORD obtainKeyStrategy = CRYPT_ACQUIRE_PREFER_NCRYPT_KEY_FLAG;
	vector<unsigned char> digest = BinaryUtils::hex2bin(getHash()->c_str());

	ALG_ID alg = 0;

	switch (digest.size())
	{
	case BINARY_SHA1_LENGTH:
		padInfo.pszAlgId = NCRYPT_SHA1_ALGORITHM;
		alg = CALG_SHA1;
		break;
	case BINARY_SHA224_LENGTH:
		padInfo.pszAlgId = L"SHA224";
		obtainKeyStrategy = CRYPT_ACQUIRE_ONLY_NCRYPT_KEY_FLAG;
		break;
	case BINARY_SHA256_LENGTH:
		padInfo.pszAlgId = NCRYPT_SHA256_ALGORITHM;
		alg = CALG_SHA_256;
		break;
	case BINARY_SHA384_LENGTH:
		padInfo.pszAlgId = NCRYPT_SHA384_ALGORITHM;
		alg = CALG_SHA_384;
		break;
	case BINARY_SHA512_LENGTH:
		padInfo.pszAlgId = NCRYPT_SHA512_ALGORITHM;
		alg = CALG_SHA_512;
		break;
	default:
		throw InvalidHashException();
	}

	SECURITY_STATUS err = 0;
	DWORD size = 256;
	vector<unsigned char> signature(size, 0);

	HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
	if (!store) {
		throw TechnicalException("Failed to open Cert Store");
	}

	PCCERT_CONTEXT certInStore = findCertificateById(certId, store);

	if (!certInStore)
	{
		CertCloseStore(store, 0);
		throw NoCertificatesException();
	}

	DWORD flags = obtainKeyStrategy | CRYPT_ACQUIRE_COMPARE_KEY_FLAG;
	DWORD spec = 0;
	BOOL freeKeyHandle = false;
	HCRYPTPROV_OR_NCRYPT_KEY_HANDLE key = NULL;
	BOOL gotKey = true;
	gotKey = CryptAcquireCertificatePrivateKey(certInStore, flags, 0, &key, &spec, &freeKeyHandle);
	CertFreeCertificateContext(certInStore);
	CertCloseStore(store, 0);

	switch (spec)
	{
	case CERT_NCRYPT_KEY_SPEC:
	{
		EstEID_log("Using CNG");
		err = NCryptSignHash(key, &padInfo, PBYTE(&digest[0]), DWORD(digest.size()),
			&signature[0], DWORD(signature.size()), (DWORD*)&size, BCRYPT_PAD_PKCS1);
		if (freeKeyHandle) {
			NCryptFreeObject(key);
		}
		break;
	}
	case AT_SIGNATURE:
	{
		EstEID_log("keySpec is AT_SIGNATURE... Using CAPI");
		HCRYPTHASH hash = 0;
		EstEID_log("calling CryptCreateHash()...");
		if (!CryptCreateHash(key, alg, 0, 0, &hash)) {
			EstEID_log("CryptCreateHash() return code: (%s) %x", "FAILURE", GetLastError());
			if (freeKeyHandle) {
				CryptReleaseContext(key, 0);
			}
			EstEID_log("throwing technical exception - createHash failed..");
			throw TechnicalException("CreateHash failed");
		}
		EstEID_log("calling CryptSetHashParam()..");
		if (!CryptSetHashParam(hash, HP_HASHVAL, digest.data(), 0))	{
			EstEID_log("CryptSetHashParam() error code: (%s) %x", "FAILURE", GetLastError());
			if (freeKeyHandle) {
				CryptReleaseContext(key, 0);
			}
			CryptDestroyHash(hash);
			EstEID_log("throwing technical exception - SetHashParam failed..");
			throw TechnicalException("SetHashParam failed");
		}
		EstEID_log("calling CryptSignHashW()..");
		INT retCode = CryptSignHashW(hash, AT_SIGNATURE, 0, 0, LPBYTE(signature.data()), &size);
		err = retCode ? ERROR_SUCCESS : GetLastError();
		EstEID_log("CryptSignHash() return code: %u (%s) %x", retCode, retCode ? "SUCCESS" : "FAILURE", err);
		if (freeKeyHandle) {
			CryptReleaseContext(key, 0);
		}
		CryptDestroyHash(hash);
		reverse(signature.begin(), signature.end());
		break;
	}
	default:
		throw TechnicalException("Incompatible key");
	}

	switch (err)
	{
	case ERROR_SUCCESS:
		break;
	case SCARD_W_CANCELLED_BY_USER: case ERROR_CANCELLED:
		throw UserCancelledException("Signing was cancelled");
	case SCARD_W_CHV_BLOCKED:
		throw PinBlockedException();
	case NTE_INVALID_HANDLE:
		throw TechnicalException("The supplied handle is invalid");
	default:
		throw TechnicalException("Signing failed");
	}
	signature.resize(size);
	return BinaryUtils::bin2hex(signature);
}

