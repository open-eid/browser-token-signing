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
#pragma once

#include "resource.h"
#include <atlbase.h>
#include <atlhost.h>
#include <atlstr.h>
#include <atlctl.h>
extern "C" {
#include "esteid_log.h"
}


using namespace ATL;

class CPinDialog : 
	public CAxDialogImpl<CPinDialog>
{
public:
	char* getPin();
	void showPinBlocked();
	void setAttemptsRemaining(int attemptsRemaining);
	void setInvalidPin(bool isPininvalid);
	CPinDialog(){}
	~CPinDialog(){}

	enum { IDD = IDD_PINDIALOG };

BEGIN_MSG_MAP(CPinDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	CHAIN_MSG_MAP(CAxDialogImpl<CPinDialog>)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {		
		CAxDialogImpl<CPinDialog>::OnInitDialog(uMsg, wParam, lParam, bHandled);
		ATLVERIFY(CenterWindow());
		GotoDlgCtrl(GetDlgItem(IDC_PIN_ENTRY));
		if (invalidPin || attemptsRemainig <= 0) {			
			wstring msg = getWrongPinErrorMessage();
			EstEID_log("you have %i tries left", attemptsRemainig);
			SetDlgItemText(IDC_PIN_MESSAGE, &msg[0]);
		}
		bHandled = TRUE;
		return FALSE; //Focus is set manually
	}

	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {	
		if (GetDlgItem(IDC_PIN_MESSAGE).m_hWnd == (HWND)lParam && (invalidPin || attemptsRemainig <= 0)) {
			SetTextColor((HDC)wParam, RGB(255, 0, 0));
		}
		HBRUSH  hBr = (HBRUSH)GetStockObject(WHITE_BRUSH);
		return (LRESULT)hBr;
	}

	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {	
		CString rawPin;
		GetDlgItem(IDC_PIN_ENTRY).GetWindowText(rawPin);
		pin = _strdup(ATL::CT2CA(rawPin));
		EndDialog(wID);
		return 0;
	}

	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
		EndDialog(wID);
		return 0;
	}

private:
	char* pin;
	int attemptsRemainig;
	bool invalidPin = false;
	wstring getWrongPinErrorMessage();
};


