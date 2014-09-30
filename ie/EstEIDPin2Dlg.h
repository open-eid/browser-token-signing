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
#include "afxcmn.h"

// CCertificateSelectionDlg dialog

class CEstEidPin2Dlg : public CDialog
{
	DECLARE_DYNAMIC(CEstEidPin2Dlg)

public:
	CEstEidPin2Dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEstEidPin2Dlg();
	enum { IDD = IDD_PIN_DIALOG };

protected:
	BOOL CEstEidPin2Dlg::OnInitDialog();
	void CEstEidPin2Dlg::EndDialogIfPIN2Blocked(int cardStatus);
	unsigned long int CEstEidPin2Dlg::SetUpPin2Dialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	HBRUSH OnCtlColor(CDC* pDC, CWnd *pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

public:
	char* GetSignature();
	void CEstEidPin2Dlg::SetCertId(WCHAR *id);
	void CEstEidPin2Dlg::SetHash(WCHAR *hahs);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();

private:
	char* signature;
	WCHAR* certId;
	WCHAR* hash;
	CString pin2;
	CButton btnOk;
	CButton btnCancel;
	CStatic errorText;
	CEdit pin2Edit;
	int retryCount;
	unsigned int pin2MinLen;

public:
	afx_msg void OnEnChangePin2();
};
