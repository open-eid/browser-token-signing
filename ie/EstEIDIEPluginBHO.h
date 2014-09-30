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

// EstEIDIEPluginBHO.h : Declaration of the CEstEIDIEPluginBHO

#pragma once
#include "resource.h"       // main symbols
#include <WinCrypt.h>
#include "EstEIDHelper.h"
#include "esteidpluginie_i.h"
#include <comutil.h>
#include "EstEIDPinPadDlg.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

#define BINARY_SHA1_LENGTH 20
#define BINARY_SHA224_LENGTH 28
#define BINARY_SHA256_LENGTH 32
#define BINARY_SHA512_LENGTH 64

// CEstEIDIEPluginBHO
DEFINE_GUID(CATID_AppContainerCompatible,
    0x59fb2056, 0xd625, 0x48d0, 0xa9, 0x44, 0x1a, 0x85, 0xb5, 0xab, 0x26, 0x40);
using namespace ATL;

class ATL_NO_VTABLE CEstEIDIEPluginBHO :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CEstEIDIEPluginBHO, &CLSID_EstEIDIEPluginBHO>,
	public IObjectWithSiteImpl<CEstEIDIEPluginBHO>,
	public IObjectSafetyImpl<CEstEIDIEPluginBHO, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA >,
	public IDispatchImpl<IEstEIDIEPluginBHO, &IID_IEstEIDIEPluginBHO, &LIBID_esteidpluginieLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CEstEIDIEPluginBHO()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ESTEIDIEPLUGINBHO)

DECLARE_NOT_AGGREGATABLE(CEstEIDIEPluginBHO)

BEGIN_COM_MAP(CEstEIDIEPluginBHO)
	COM_INTERFACE_ENTRY(IEstEIDIEPluginBHO)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		this->errorCode = 0;
		this->language = NULL;
		return S_OK;
	}

	void FinalRelease()
	{
		webBrowser.Release();
	}

private:
	CComPtr<IWebBrowser2> webBrowser;
	CComPtr<IEstEIDCertificate> certificate;
	BSTR language;
	int errorCode;
	std::string errorMessage;

	void CryptoErrorHandler(BOOL result);
	void setError(unsigned int code);
	void mapInternalErrorCodes(unsigned int code);
	BOOL isSiteAllowed();
	BOOL isSameCardInReader(CComPtr<IEstEIDCertificate> _cert);
	BOOL CEstEIDIEPluginBHO::certificateMatchesId(PCCERT_CONTEXT certContext, BSTR id);
	void CEstEIDIEPluginBHO::signWithCSP(BSTR id, BSTR hash, BSTR *signature);
	void CEstEIDIEPluginBHO::signWithCNG(BSTR id, BSTR hash, BSTR *signature);
	BOOL CEstEIDIEPluginBHO::isCNGInstalled();
	BOOL CEstEIDIEPluginBHO::isWinVistaOrLater() ;
	void CEstEIDIEPluginBHO::showAlert(void *nativeWindowHandle, const char *message);
	void signWithPKCS11(HINSTANCE hInst, BSTR id, BSTR hash, BSTR *signature);

public:
	STDMETHOD(SetSite)(IUnknown *pUnkSite);
	STDMETHOD(get_version)(BSTR *result);
	STDMETHOD(get_pluginLanguage)(BSTR *result);
	STDMETHOD(put_pluginLanguage)(BSTR language);
	STDMETHOD(get_errorMessage)(BSTR *result);
	STDMETHOD(get_errorCode)(BSTR *result);
	STDMETHOD(getCertificate)(IDispatch **_certificate);
	STDMETHOD(sign)(BSTR id, BSTR hash, BSTR language, BSTR *signature);	
};

OBJECT_ENTRY_AUTO(__uuidof(EstEIDIEPluginBHO), CEstEIDIEPluginBHO)
