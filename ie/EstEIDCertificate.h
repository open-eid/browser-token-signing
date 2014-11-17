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

// EstEIDCertificate.h : Declaration of the CEstEIDCertificate
#pragma once

#include <comutil.h>
#include <iomanip>
#include <iostream>
#include <vector>
#include <WinCrypt.h>
#include <Cryptuiapi.h>
#include "resource.h"       // main symbols
#include "esteidpluginie_i.h"
#include "CryptoException.h"
#include "EstEIDHelper.h"
#include "cert_dialog_win.h"
extern "C" {
#include "esteid_log.h"
#include "esteid_error.h"
}


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

char* calculateHash(char* certificate);

using namespace ATL;

// CEstEIDCertificate

class ATL_NO_VTABLE CEstEIDCertificate :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CEstEIDCertificate, &CLSID_EstEIDCertificate>,
	public IDispatchImpl<IEstEIDCertificate, &IID_IEstEIDCertificate, &LIBID_esteidpluginieLib, /*wMajor =*/ 1, /*wMinor =*/ 0>{
public:
	CEstEIDCertificate(){
		this->window = NULL;
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_ESTEIDCERTIFICATE)

	HRESULT FinalConstruct() {
		certificate = NULL;
		CN[0] = '\0';
		issuerCN[0] = '\0';
		readFromCertContext();

		return S_OK;
	}

	void FinalRelease() {
		if(certificate) {
			free(certificate);
			certificate = NULL;
		}
	}

	BEGIN_COM_MAP(CEstEIDCertificate)
		COM_INTERFACE_ENTRY(IEstEIDCertificate)
		COM_INTERFACE_ENTRY(IDispatch)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

private:
	CComPtr<IWebBrowser2> webBrowser;
	BYTE *certificate;
	HCRYPTPROV cryptoProvider;
	DWORD length;
	std::string id;
	TCHAR CN[2048];
	TCHAR issuerCN[2048];
	std::string validFrom;
	std::string validTo;
	std::string certificateAsHex;
	HWND window;

	void readFromCertContext();
	void CryptoErrorHandler(BOOL result);
	std::string dateToString(FILETIME *filetime);
	void binCert2Hex(const unsigned int binLength);
	void calculateMD5Hash(unsigned int certLength);
	char *getBytesAsHexString(void *bytes, unsigned int length);
	void loadCertContexts(PCCERT_CONTEXT certContext);

public:
	STDMETHOD(get_id)(BSTR *id);	
	STDMETHOD(get_CN)(BSTR *cn);
	STDMETHOD(get_issuerCN)(BSTR *issuerCN);
	STDMETHOD(get_validFrom)(BSTR *validFrom);
	STDMETHOD(get_validTo)(BSTR *validTo);
	STDMETHOD(get_certificateAsPem)(BSTR *certificate);
	STDMETHOD(get_certificateAsHex)(BSTR *certificate);
	STDMETHOD(get_cert)(BSTR *certificate);
	//std::vector<PCCERT_CONTEXT> certificates;
};

OBJECT_ENTRY_AUTO(__uuidof(EstEIDCertificate), CEstEIDCertificate)
