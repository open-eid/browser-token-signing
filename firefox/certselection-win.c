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

#include "certselection-win.h"
#include "esteid_certinfo.h"
#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include "cert-class.h"
#include "esteid_time.h"
#include <AtlConv.h>
#include <atlstr.h>
#include <Cryptuiapi.h>
#include "l10n.h"

#pragma comment(lib, "Cryptui.lib")
#pragma comment(lib, "comctl32.lib")

extern HINSTANCE pluginInstance;

void buildCertificatesList(HWND hwnd) {
	RECT rect;
	HWND certificatesList;
	LV_COLUMN lvC;
	
	certificatesList = GetDlgItem(hwnd, IDC_CERTIFICATESLIST);
	ListView_SetUnicodeFormat(certificatesList, true);
	GetClientRect(certificatesList, &rect);
	int rectWidth = rect.right - rect.left; 
	int colSize = rectWidth/6;
	int colWidths[] = {3 * colSize, 2 * colSize, colSize + (rectWidth - (6*colSize)), 0};
	char* colTitles[] = {"Certificate", "Type", "Valid to", ""};
	
	ListView_SetExtendedListViewStyle(certificatesList, LVS_EX_FULLROWSELECT);
	lvC.mask = LVCF_WIDTH | LVCF_TEXT;
    lvC.fmt = LVCFMT_LEFT;
	

	WCHAR columnName[64];
	for(int index = 0; index < 4; index++) {
		lvC.iSubItem = index;
		lvC.cx = colWidths[index];
		
		MultiByteToWideChar(CP_UTF8, 0, l10n(colTitles[index]), -1, columnName, sizeof(columnName) / sizeof(WCHAR));
		
		lvC.pszText = columnName;
		ListView_InsertColumn(certificatesList, index, &lvC);
    }

	LV_ITEM lv;
	lv.mask = LVIF_TEXT;
	
	EstEID_Certs *certs = EstEID_loadCerts();
	EstEID_log("reading certs @ %p, certs->count = %u ", certs, certs->count);

	TCHAR unicodeBuf[512];
	int currentCertIndex = 0;	
	for (int i = 0; i < certs->count; i++) {		
		EstEID_Map cert = certs->certs[i];
		EstEID_logMap(cert);
		if (!EstEID_mapGet(cert, "usageNonRepudiation")) {
			EstEID_log("cert %s is removed from list", EstEID_mapGet(cert, "label"));
			continue;
		}
		
		lv.iSubItem = 0;
		lv.iItem = currentCertIndex;	
		
		MultiByteToWideChar(CP_UTF8, 0, EstEID_mapGet(cert, "commonName"), -1, unicodeBuf, sizeof(unicodeBuf) / sizeof(TCHAR)); 
		lv.pszText = (LPWSTR)unicodeBuf;
		ListView_InsertItem(certificatesList, &lv);

		const char *org = EstEID_mapGet(cert, "organizationName");
		if (org) {
			MultiByteToWideChar(CP_UTF8, 0, EstEID_mapGet(cert, "organizationName"), -1, unicodeBuf, sizeof(unicodeBuf) / sizeof(TCHAR)); 
			ListView_SetItemText(certificatesList, currentCertIndex, 1, unicodeBuf);
		} else {
			ListView_SetItemText(certificatesList, currentCertIndex, 1, L"");
		}
		
		char* validTo = getDateFromDateTime(EstEID_mapGet(cert, "validTo"));
		MultiByteToWideChar(CP_UTF8, 0, validTo, -1, unicodeBuf, sizeof(unicodeBuf) / sizeof(TCHAR)); 
		ListView_SetItemText(certificatesList, currentCertIndex, 2, unicodeBuf);		
		free(validTo);
				
		MultiByteToWideChar(CP_UTF8, 0, EstEID_mapGet(cert, "certHash"), -1, unicodeBuf, sizeof(unicodeBuf) / sizeof(TCHAR)); 
		ListView_SetItemText(certificatesList, currentCertIndex, 3, unicodeBuf);

		currentCertIndex++;
	}	
	if (certs->count > 0) {		
		ListView_SetItemState(certificatesList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}	
}

INT_PTR CALLBACK CertSelectionDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {	
	static char* selectedCertID;

	switch(message) {
		case WM_INITDIALOG:			
			HWND previousWindowsHandle;

			EstEID_log("current focus is @ %08X", GetFocus());

			WCHAR label[128];
			MultiByteToWideChar(CP_UTF8, 0, l10n("Select certificate"), -1, label, sizeof(label) / sizeof(WCHAR));
			SetWindowText(hwnd, label);

			MultiByteToWideChar(CP_UTF8, 0, l10n("Select"), -1, label, sizeof(label) / sizeof(WCHAR));
			SetDlgItemText(hwnd, IDOK, label);

			MultiByteToWideChar(CP_UTF8, 0, l10n("Cancel"), -1, label, sizeof(label) / sizeof(WCHAR));
			SetDlgItemText(hwnd, IDCANCEL, label);

			MultiByteToWideChar(CP_UTF8, 0, l10n("By selecting a certificate I accept that my name and personal ID code will be sent to service provider."), -1, label, sizeof(label) / sizeof(WCHAR));
			SetDlgItemText(hwnd, IDC_WARNING, label);

			buildCertificatesList(hwnd);
			selectedCertID = (char*)lParam;
			selectedCertID[0] = '\0';

			EstEID_log("setting focus to certificate selection dialog, hwnd = %08X", hwnd);
			previousWindowsHandle = SetFocus(hwnd);
			if(!previousWindowsHandle) {
				EstEID_log("focus setting returned without previous windows handle, error code = %li", GetLastError());
			}
			else {
				EstEID_log("focus taken from %08X and set to %08X", previousWindowsHandle, hwnd);
			}	

			return true;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK: {
					int index = ListView_GetNextItem(GetDlgItem(hwnd, IDC_CERTIFICATESLIST), -1, LVNI_SELECTED);
					TCHAR unicodeSelectedCertID[256];
					ListView_GetItemText(GetDlgItem(hwnd, IDC_CERTIFICATESLIST), index, 3, (LPTSTR)unicodeSelectedCertID, 33);
					strcpy(selectedCertID, CW2A(unicodeSelectedCertID));
					EstEID_log("selected certificate id in dialog proc: %s", selectedCertID);
				}
				default:
					EndDialog(hwnd, LOWORD(wParam));
			}	
			return true;
		case WM_NOTIFY:			
			if(wParam == IDC_CERTIFICATESLIST) {
				switch(((LPNMHDR)lParam)->code) {
					case LVN_ITEMCHANGED:
					case NM_CLICK:
						Button_Enable(GetDlgItem(hwnd, IDOK), ((LPNMITEMACTIVATE)lParam)->iItem != -1);
						break;
					case LVN_ITEMACTIVATE: {
						int selectedCertIndex = ((LPNMITEMACTIVATE)lParam)->iItem;
						TCHAR unicodeSelectedCertID[256];
						ListView_GetItemText(GetDlgItem(hwnd, IDC_CERTIFICATESLIST), selectedCertIndex, 3, (LPTSTR)unicodeSelectedCertID, 33);
						strcpy(selectedCertID, CW2A(unicodeSelectedCertID));
						EstEID_log("dblcklicked certificate id in dialog proc: %s", selectedCertID);
						EndDialog(hwnd, LOWORD(wParam));
					}
				}
			}
			return true;
		default:
			return false;
	}	
}
