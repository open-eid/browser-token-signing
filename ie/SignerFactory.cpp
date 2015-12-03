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

#include "Shlobj.h"
#include "Knownfolders.h"
#include "SignerFactory.h"
#include "PKCS11Signer.h"
#include "CngCapiSigner.h"
extern "C" {
#include "esteid_log.h"
}

Signer * SignerFactory::createSigner(const string &hash, char *certId) {
	PCCERT_CONTEXT cert = findCertificateById(certId);
	if (!isLithuanianCertificate(cert)) {
		return new CngCapiSigner(hash, certId, cert);
	}
	EstEID_log("certificate issuer C=LT, using PKCS11 Signer with custom dialog");
	vector<unsigned char> data(cert->pbCertEncoded, cert->pbCertEncoded + cert->cbCertEncoded);
	CertFreeCertificateContext(cert);
	Pkcs11Signer *signer = new Pkcs11Signer(hash, certId, BinaryUtils::bin2hex(data));
	signer->setPkcs11ModulePath(getLithuanianPKCS11ModulePath());
	signer->initialize();
	return signer;
}

bool SignerFactory::isLithuanianCertificate(PCCERT_CONTEXT certContext) {
	DWORD cbSize;
	LPTSTR issuerLpstr;
	cbSize = CertNameToStr(certContext->dwCertEncodingType, &(certContext->pCertInfo->Issuer), CERT_X500_NAME_STR, NULL, 0);
	if (cbSize == 1) {
		EstEID_log("Issuer is an empty String");
		return false;
	}
	if (!(issuerLpstr = (LPTSTR)malloc(cbSize * sizeof(TCHAR)))) {
		EstEID_log("Memory allocation failed");
		return false;
	}
	cbSize = CertNameToStr(certContext->dwCertEncodingType, &(certContext->pCertInfo->Issuer), CERT_X500_NAME_STR, issuerLpstr, cbSize);

	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, issuerLpstr, -1, NULL, 0, NULL, NULL);
	char* encodedStr = new char[sizeNeeded];
	WideCharToMultiByte(CP_UTF8, 0, issuerLpstr, -1, encodedStr, sizeNeeded, NULL, NULL);
	free(issuerLpstr);
	EstEID_log("Certificate issuer: %s", encodedStr);

	string issuerString = string(encodedStr);
	return issuerString.find("C=LT") != string::npos;
}

bool SignerFactory::certificateMatchesId(PCCERT_CONTEXT certContext, char *certId) {
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

PCCERT_CONTEXT SignerFactory::findCertificateById(char *certId) {
	HCERTSTORE cert_store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
	if (!cert_store) {
		throw TechnicalException("Failed to open Cert Store");
	}
	PCCERT_CONTEXT certContext = NULL;
	while (certContext = CertFindCertificateInStore(cert_store, X509_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, certContext)) {
		if (certificateMatchesId(certContext, certId)) {
			CertCloseStore(cert_store, 0);
			return certContext;
		}
	}
	CertCloseStore(cert_store, 0);
	throw NoCertificatesException();
}

string SignerFactory::getLithuanianPKCS11ModulePath() {
	wchar_t* ppszPath = 0;
	SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, 0, NULL, &ppszPath);
	wstring programFilesX86(ppszPath);
	wstring path = programFilesX86 + L"\\CryptoTech\\CryptoCard\\CCPkiP11.dll";
	CoTaskMemFree(static_cast<void*>(ppszPath));
	return string(CW2A(path.c_str()));
}