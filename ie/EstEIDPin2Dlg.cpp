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
// EstEIDPin2Dlg.cpp : Implementation of CEstEIDPin2Dlg
#define NOT_FOUND -1

#include "stdafx.h"
#include <AtlConv.h>
#include "EstEIDPin2Dlg.h"
#include "../firefox/dialogs-win.h"

extern "C" {
#include "l10n.h"
#include "esteid_log.h"
#include "esteid_sign.h"
#include "esteid_dialog_common.h"
#include "dialogs.h"
#include "pkcs11_errors.h"
}

typedef LANGID (WINAPI *FP_SetThreadUILanguage)(LANGID LangId);

// CCertificateSelectionDlg dialog

IMPLEMENT_DYNAMIC(CEstEidPin2Dlg, CDialog)

CEstEidPin2Dlg::CEstEidPin2Dlg(CWnd* pParent): CDialog(CEstEidPin2Dlg::IDD, pParent) {
	LOG_LOCATION;
	this->signature = NULL;
	this->certId = NULL;
	retryCount = 0;
}

CEstEidPin2Dlg::~CEstEidPin2Dlg()
{
}

BEGIN_MESSAGE_MAP(CEstEidPin2Dlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CEstEidPin2Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CEstEidPin2Dlg::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_PIN2, &CEstEidPin2Dlg::OnEnChangePin2)
END_MESSAGE_MAP()


// CCertificateSelectionDlg message handlers

BOOL CEstEidPin2Dlg::OnInitDialog() {
	LOG_LOCATION;
	CDialog::OnInitDialog();
	WCHAR label[512];

	int certIndex;
	EstEID_Certs *certs = EstEID_loadCerts();
	if (!certs) {
		EstEID_log("no certs found");
	}
	else if ((certIndex = EstEID_findNonRepuditionCert(certs, ATL::CW2A(this->certId))) == NOT_FOUND) {
		EstEID_log("card is changed");
	}
	EstEID_Map cert = certs->certs[certIndex];

	char* name = EstEID_getFullNameWithPersonalCode(cert);
	MultiByteToWideChar(CP_UTF8, 0, name, -1, label, sizeof(label) / sizeof(WCHAR));

	this->SetWindowText(label);

	SetDlgItemText(IDC_NAME, label);

	this->pin2MinLen = (unsigned)atoi(EstEID_mapGet(cert, "minPinLen"));

	free(name);

	MultiByteToWideChar(CP_UTF8, 0, l10n("For signing enter PIN2:"), -1, label, sizeof(label) / sizeof(WCHAR));
	SetDlgItemText(IDC_PIN2_LABEL, label);

	MultiByteToWideChar(CP_UTF8, 0, l10n("Sign"), -1, label, sizeof(label) / sizeof(WCHAR));
	SetDlgItemText(IDOK, label);

	MultiByteToWideChar(CP_UTF8, 0, l10n("Cancel"), -1, label, sizeof(label) / sizeof(WCHAR));
	SetDlgItemText(IDCANCEL, label);
	SetDlgItemText(IDC_ERROR, L"");

	EndDialogIfPIN2Blocked(SetUpPin2Dialog());
	return TRUE;
}

void CEstEidPin2Dlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, btnOk);
	DDX_Control(pDX, IDCANCEL, btnCancel);
	DDX_Control(pDX, IDC_ERROR, errorText);
	DDX_Control(pDX, IDC_PIN2, pin2Edit);
}

void CEstEidPin2Dlg::OnBnClickedOk() {
	LOG_LOCATION;
	GetDlgItem(IDC_PIN2)->GetWindowText(this->pin2);

	this->retryCount++;
	
	EstEID_PINPromptData pinPromptData = {promptForPIN, showAlert, this->m_hWnd};
	pinPromptData.pin2 = _strdup(ATL::CT2CA(this->pin2));	
	
	this->signature = EstEID_sign(strdup(ATL::CW2A(this->certId)), strdup(ATL::CW2A(this->hash)), pinPromptData);
	if(this->signature == NULL) {
		int cardStatus = SetUpPin2Dialog();
		if( cardStatus!= CKR_OK) {
			EndDialogIfPIN2Blocked(cardStatus);
		}
	}
	else {
		EndDialog(true);
	}
}

void CEstEidPin2Dlg::OnBnClickedCancel() {
	EndDialog(false);
}

void CEstEidPin2Dlg::SetCertId(WCHAR *id) {
	this->certId = id;
}

void CEstEidPin2Dlg::SetHash(WCHAR *hash) {
	this->hash = hash;
}

char* CEstEidPin2Dlg::GetSignature() {
	return this->signature;
}

CK_ULONG CEstEidPin2Dlg::SetUpPin2Dialog() {
	LOG_LOCATION;
	char message[512];
	CK_SLOT_ID slotId;

	EstEID_log("certId: %s ", certId);
	if(!EstEID_getSlotId(ATL::CW2A(this->certId), &slotId)) {
		return CKR_GENERAL_ERROR;
	}
	int remainingTries = EstEID_getRemainingTries(slotId);
	EstEID_log("EstEID_getRemainingTries(slotID) = %i", remainingTries);
	if (remainingTries == -1) {
		return CKR_GENERAL_ERROR;
	}		
	if (!remainingTries) {		
		EstEID_log("C_Login error: %s (%li)", pkcs11_error_message(CKR_PIN_LOCKED), CKR_PIN_LOCKED);
		return CKR_PIN_LOCKED;
	}
	EstEID_log("retryCount = %i", this->retryCount);
	if (remainingTries < 3) {
		sprintf(message, "%s %i", l10n("Tries left:"), remainingTries);		
		sprintf(message, "%s%s %i", (retryCount ? l10n("Incorrect PIN2! ") : ""), l10n("Tries left:"), remainingTries);
	}
	else {
		strcpy(message, "");
	}
	
	WCHAR label[512];
	MultiByteToWideChar(CP_UTF8, 0, message, -1, label, sizeof(label) / sizeof(WCHAR));
	errorText.SetWindowText(label);
	pin2Edit.SetWindowText(L"");	
	
	return CKR_OK;
}

void CEstEidPin2Dlg::EndDialogIfPIN2Blocked(int cardStatus) {
	LOG_LOCATION;
	if(cardStatus == CKR_PIN_LOCKED) {
		WCHAR label[512];
		MultiByteToWideChar(CP_UTF8, 0, l10n("PIN2 blocked, cannot sign!"), -1, label, sizeof(label) / sizeof(WCHAR));
		ShowWindow(false);
		MessageBox(label, ATL::CA2W(l10n("Error")), MB_OK | MB_ICONERROR);
		EndDialog(true);
	}
}

HBRUSH CEstEidPin2Dlg::OnCtlColor(CDC* pDC, CWnd *pWnd, UINT nCtlColor) {
	if(pWnd->GetDlgCtrlID() == IDC_ERROR) {
		pDC->SetTextColor(RGB(255, 0, 0));
		pDC->SetBkColor(GetSysColor(COLOR_BTNFACE));
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CEstEidPin2Dlg::OnEnChangePin2() {
	btnOk.EnableWindow(((unsigned)pin2Edit.LineLength() >= this->pin2MinLen));
}
