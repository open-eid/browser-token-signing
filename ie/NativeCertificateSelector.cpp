#include "NativeCertificateSelector.h"
#include "BinaryUtils.h"
#include "HostExceptions.h"
extern "C" {
#include "esteid_log.h"
}

using namespace std;

PCCERT_CONTEXT NativeCertificateSelector::getCert() {
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
	EstEID_log("Pointer to CERT_STORE 0x%08X", hCertStore);
	if (hCertStore) {
		CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
	}
	//vector<unsigned char> data(pCertContext->pbCertEncoded, pCertContext->pbCertEncoded + pCertContext->cbCertEncoded);
	return pCertContext;
	
	//return pCertContext;
	/*
	HCERTSTORE store = CertOpenSystemStore(0, L"MY");
	if (!store)
	{
		throw TechnicalException("Failed to open Cert Store");
	}

	PCCERT_CONTEXT pCertContextForEnumeration = nullptr;
	int certificatesCount = 0;
	while (pCertContextForEnumeration = CertEnumCertificatesInStore(store, pCertContextForEnumeration)) {
		if (isValidForSigning(pCertContextForEnumeration)) {
			certificatesCount++;
		}
	}
	if (pCertContextForEnumeration){
		CertFreeCertificateContext(pCertContextForEnumeration);
	}
	if (certificatesCount < 1) {
		CertCloseStore(store, 0);
		throw NoCertificatesException();
	}

	CRYPTUI_SELECTCERTIFICATE_STRUCT pcsc = { sizeof(pcsc) };
	pcsc.pFilterCallback = filter_proc;
	pcsc.pvCallbackData = nullptr;
	pcsc.cDisplayStores = 1;
	pcsc.rghDisplayStores = &store;
	PCCERT_CONTEXT cert_context = CryptUIDlgSelectCertificate(&pcsc);

	if (!cert_context)
	{
		CertCloseStore(store, 0);
		throw UserCancelledException();
	}

	vector<unsigned char> data(cert_context->pbCertEncoded, cert_context->pbCertEncoded + cert_context->cbCertEncoded);
	CertFreeCertificateContext(cert_context);
	CertCloseStore(store, 0);
	return BinaryUtils::bin2hex(data);
	*/
}