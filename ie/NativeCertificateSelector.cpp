#include "NativeCertificateSelector.h"
#include "BinaryUtils.h"
#include "HostExceptions.h"
extern "C" {
#include "esteid_log.h"
}

using namespace std;

std::vector<unsigned char> NativeCertificateSelector::getCert() {
	LOG_LOCATION;
	PCCERT_CONTEXT pCertContext = NULL;
	HCERTSTORE hCertStore = NULL;
	CRYPTUI_SELECTCERTIFICATE_STRUCT sel = { sizeof(sel) };
	int counter = 0;

	hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
	if (!hCertStore){
		throw TechnicalException("Failed to open Cert Store");
	}

	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);

	sel.pvCallbackData = &counter;
	sel.pFilterCallback = filter_proc;
	sel.rghDisplayStores = &hCertStore;
	sel.cDisplayStores = 1;

#ifdef _SEB_BUILD	
	EstEID_log("SEB build");
	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);
	PCCERT_CONTEXT pCertContextForEnumeration = NULL;
	int certificatesCount = 0;
	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);
	while (pCertContextForEnumeration = CertEnumCertificatesInStore(hCertStore, pCertContextForEnumeration))
	{
		if (isValidForSigning(pCertContextForEnumeration))
		{
			certificatesCount++;
			pCertContext = pCertContextForEnumeration;
			loadCertContexts(pCertContext);
		}
	}

	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);

	EstEID_log("Certificates count %i", certificatesCount);

	if (certificatesCount != 1)
	{
		pCertContext = CryptUIDlgSelectCertificate(&sel);
	}

	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);

	if (pCertContextForEnumeration){
		CertFreeCertificateContext(pCertContextForEnumeration);
	}
	if (!pCertContext) {
		EstEID_log("User didn't select sertificate");
		throw UserCancelledException();
	}
#else
	pCertContext = CryptUIDlgSelectCertificate(&sel);
	if (!pCertContext) {
		EstEID_log("User didn't select sertificate");
		CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
		throw UserCancelledException();
	}
#endif
	vector<unsigned char> certData(pCertContext->pbCertEncoded, pCertContext->pbCertEncoded + pCertContext->cbCertEncoded);
	CertFreeCertificateContext(pCertContext);
	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);
	if (hCertStore) {
		CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
	}
	return certData;
	
}