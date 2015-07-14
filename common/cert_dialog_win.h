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

#ifndef ESTEID_CERT_DIAOLG_WIN_H
#define	ESTEID_CERT_DIAOLG_WIN_H

#include <windows.h>
#include <cryptuiapi.h>
#include <ncrypt.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef BOOL (WINAPI * PFNCCERTDISPLAYPROC)(
	__in  PCCERT_CONTEXT pCertContext,
	__in  HWND hWndSelCertDlg,
	__in  void *pvCallbackData
);

typedef struct _CRYPTUI_SELECTCERTIFICATE_STRUCT {
	DWORD               dwSize;
	HWND                hwndParent;
	DWORD               dwFlags;
	LPCWSTR             szTitle;
	DWORD               dwDontUseColumn;
	LPCWSTR             szDisplayString;
	PFNCFILTERPROC      pFilterCallback;
	PFNCCERTDISPLAYPROC pDisplayCallback;
	void *              pvCallbackData;
	DWORD               cDisplayStores;
	HCERTSTORE *        rghDisplayStores;
	DWORD               cStores;
	HCERTSTORE *        rghStores;
	DWORD               cPropSheetPages;
	LPCPROPSHEETPAGEW   rgPropSheetPages;
	HCERTSTORE          hSelectedCertStore;
} CRYPTUI_SELECTCERTIFICATE_STRUCT, *PCRYPTUI_SELECTCERTIFICATE_STRUCT;

typedef const CRYPTUI_SELECTCERTIFICATE_STRUCT *PCCRYPTUI_SELECTCERTIFICATE_STRUCT;
PCCERT_CONTEXT WINAPI CryptUIDlgSelectCertificateW(
  _In_  PCCRYPTUI_SELECTCERTIFICATE_STRUCT pcsc
);

#define CryptUIDlgSelectCertificate CryptUIDlgSelectCertificateW

BOOL isCardInReader(PCCERT_CONTEXT certContext);
BOOL WINAPI isValidForSigning(PCCERT_CONTEXT);
BOOL WINAPI filter_proc(PCCERT_CONTEXT, BOOL *, void *);
#ifdef __cplusplus
}
#endif
#endif
