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

#pragma once

#define PROGRESS_TIMER 0x1001

#include "afxcmn.h"

// CCertificateSelectionDlg dialog

class CEstEidPinPadDlg : public CDialog
{
	DECLARE_DYNAMIC(CEstEidPinPadDlg)

public:
	CEstEidPinPadDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEstEidPinPadDlg();
	void SetCertId(WCHAR *id);
	void SetHash(WCHAR *hahs);
	char* GetSignature();
	enum { IDD = IDD_PINPAD_DIALOG };

protected:
	BOOL OnInitDialog();
	void SetUpPin2Dialog();
	void CEstEidPinPadDlg::OnTimer(UINT_PTR nIDEvent);
	void CEstEidPinPadDlg::OnCancel();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

private:
	WCHAR* certId;
	WCHAR* hash;
	int countdownValue;
	char* signature;
	CWnd *progressBar;
};
