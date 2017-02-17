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
#include "PKCS11Path.h"
extern "C" {
#include "esteid_log.h"
}

Signer * SignerFactory::createSigner(const string &hash, char *certId) {
	
	std::string pkcs11 = PKCS11Path::getPkcs11ModulePath();
	if (!pkcs11.empty()) {
		Pkcs11Signer *signer = new Pkcs11Signer(hash, certId);
		signer->setPkcs11ModulePath(pkcs11);
		signer->initialize();
		return signer;
	}
	
	PCCERT_CONTEXT cert = findCertificateById(certId);
	return new CngCapiSigner(hash, certId, cert);
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
