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
#include "stdafx.h"
#include <AtlConv.h>
#include "EstEidPinPadDlg.h"

extern "C" {
#include "l10n.h"
#include "esteid_log.h"
#include "esteid_sign.h"
#include "esteid_dialog_common.h"
#include "dialogs.h"
#include "pkcs11_errors.h"
#include "esteid_error.h"
}

typedef LANGID (WINAPI *FP_SetThreadUILanguage)(LANGID LangId);

// CCertificateSelectionDlg dialog

IMPLEMENT_DYNAMIC(CEstEidPinPadDlg, CDialog)

CEstEidPinPadDlg::CEstEidPinPadDlg(CWnd* pParent): CDialog(CEstEidPinPadDlg::IDD, pParent) {
	this->countdownValue = 30;
	this->signature = NULL;
}

CEstEidPinPadDlg::~CEstEidPinPadDlg()
{
	KillTimer(PROGRESS_TIMER);
}

BEGIN_MESSAGE_MAP(CEstEidPinPadDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CCertificateSelectionDlg message handlers

BOOL CEstEidPinPadDlg::OnInitDialog() {
	LOG_LOCATION;
	CDialog::OnInitDialog();

	progressBar = GetDlgItem(IDC_PROGRESSBAR);
	SetTimer(PROGRESS_TIMER, 1000, NULL);
	SetUpPin2Dialog();
	
	return TRUE;
}

void CEstEidPinPadDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}

void CEstEidPinPadDlg::SetCertId(WCHAR *id) {
	this->certId = id;
}

void CEstEidPinPadDlg::SetHash(WCHAR *hash) {
	this->hash = hash;
}

void CEstEidPinPadDlg::SetUpPin2Dialog() {
	LOG_LOCATION;

	WCHAR label[512];
	MultiByteToWideChar(CP_UTF8, 0, l10n("For signing enter PIN2 from PIN pad"), -1, label, sizeof(label) / sizeof(WCHAR));
	SetDlgItemText(IDC_PINPAD_PIN2_LABEL, label);

	 // in seconds
	EstEID_log("initializing pinpad countdown timer with value %i", countdownValue);
	progressBar->SendMessage(PBM_SETSTATE, (WPARAM)PBST_PAUSED, 0L);
	progressBar->SendMessage(PBM_SETRANGE, 0, MAKELPARAM(0, countdownValue));
	progressBar->SendMessage(PBM_SETPOS, (WPARAM)countdownValue, 0L);
	progressBar->SendMessage(PBM_SETSTATE, (WPARAM)PBST_NORMAL, 0L);
}

char* CEstEidPinPadDlg::GetSignature() {
	return this->signature;
}

void CEstEidPinPadDlg::OnTimer(UINT_PTR nIDEvent) {
	if(countdownValue > 0){
		countdownValue--;
		progressBar->SendMessage(PBM_SETPOS, (WPARAM)countdownValue, 0L);
	}
	else {
		KillTimer(PROGRESS_TIMER);
		EndDialog(ESTEID_USER_CANCEL);
	}
}

void CEstEidPinPadDlg::OnCancel() {
}