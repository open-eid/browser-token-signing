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

#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <atlconv.h>
#include <atlstr.h>
#include <commctrl.h>
#include "esteid_log.h"
#include "esteid_error.h"
#include "esteid_sign.h"
#include "resource.h"
#include "l10n.h"
#include "esteid_dialog_common.h"
#include "pkcs11_errors.h"

#define PINPAD_TIMER_ID 1

extern HINSTANCE pluginInstance;
extern char EstEID_error[1024];

HWND dialogWindowHandle;
HANDLE timerHandle;
int countdownValue;

void SetDialogItemText(HWND hwnd, UINT resourceID, LPCSTR text){
	WCHAR unicodeBuf[512];
	MultiByteToWideChar(CP_UTF8, 0, text, -1, unicodeBuf, sizeof(unicodeBuf) / sizeof(WCHAR)); 
	SetDlgItemText(hwnd, resourceID, unicodeBuf);
}

void showAlert(void *nativeWindowHandle, const char *message) {
	MessageBox((HWND)nativeWindowHandle, CA2W(message), CA2W(l10n("Error")), MB_OK | MB_ICONERROR);
}

BOOL isAcceptableLengthPIN2(HWND hwnd, UINT pinFieldId, unsigned int minPin2Length){
	return SendDlgItemMessage(hwnd, pinFieldId, WM_GETTEXTLENGTH, NULL, NULL) >= minPin2Length;
}

void UpdateProgerssBar(HWND dialogWindowHandle, int countdownValue) {
	SendMessage(GetDlgItem(dialogWindowHandle, IDC_PROGRESSBAR), PBM_SETPOS, (WPARAM)countdownValue, 0L);
}

void KillCountdownTimer(){
	KillTimer(dialogWindowHandle, 1);
	EstEID_log("pinpad countdown timer killed");
}

void CountdownTimerProc() {
	if(countdownValue > 0){
		countdownValue--;
		UpdateProgerssBar(dialogWindowHandle, countdownValue);
	}
	else {
		KillCountdownTimer();
	}
}

void SetPinDialogLabels(HWND windowHandle, char *certId, unsigned int *pin2Len) {
	LOG_LOCATION; 

//duplicate code
	int certIndex;
	EstEID_Certs *certs = EstEID_loadCerts();
	EstEID_log("certs loaded");
	if (!certs) {
		EstEID_log("%s", EstEID_error);
	}
	else if ((certIndex = EstEID_findNonRepuditionCert(certs, certId)) == NOT_FOUND) {
		EstEID_log("crd is changed");
	}
	EstEID_Map cert = certs->certs[certIndex];

//duplicated code
	char* name = EstEID_getFullNameWithPersonalCode(cert);

	WCHAR label[512];
	MultiByteToWideChar(CP_UTF8, 0, createDialogTitle(name), -1, label, sizeof(label) / sizeof(WCHAR));
	SetWindowText(windowHandle, label);

	SetDialogItemText(windowHandle, IDC_NAME, name);

	*pin2Len = (unsigned)atoi(EstEID_mapGet(cert, "minPinLen"));

	free(name);
}

void SetPinPadDialogLabels(HWND windowHandle, LPARAM lParam) {
	WCHAR label[512];
	MultiByteToWideChar(CP_UTF8, 0, createDialogTitle(((DialogData *)lParam)->name), -1, label, sizeof(label) / sizeof(WCHAR));
	SetWindowText(windowHandle, label);

	SetDialogItemText(windowHandle, IDC_NAME, ((DialogData *)lParam)->name);
	if(((DialogData *)lParam)->message != NULL) {
		SetDialogItemText(windowHandle, IDC_ERROR, ((DialogData *)lParam)->message);
	}
}

INT_PTR SetMessageLabelColorToRed(WPARAM wParam) {
	SetTextColor((HDC)wParam, RGB(255, 0, 0));
	SetBkColor((HDC)wParam, GetSysColor(COLOR_BTNFACE));
	return (INT_PTR)GetStockObject(NULL_BRUSH);
}

CK_ULONG SetUpPin2Dialog(HWND hwnd, int retryCount, char* certId) {
	char message[512];
	CK_SLOT_ID slotId;

	if(!EstEID_getSlotId(certId, &slotId)) {
		return CKR_GENERAL_ERROR;
	}
	int remainingTries = EstEID_getRemainingTries(slotId);
	EstEID_log("EstEID_getRemainingTries(slotID) = %i", remainingTries);
	if (remainingTries == -1) {
		return CKR_GENERAL_ERROR;
	}		
	if (!remainingTries) {		
		sprintf(EstEID_error, "C_Login error: %s (%li)", pkcs11_error_message(CKR_PIN_LOCKED), CKR_PIN_LOCKED);
		return CKR_PIN_LOCKED;
	}
	EstEID_log("retryCount = %i", retryCount);
	if (remainingTries < 3) {
		sprintf(message, "%s %i", l10n("Tries left:"), remainingTries);		
		sprintf(message, "%s%s %i", (retryCount ? l10n("Incorrect PIN2! ") : ""), l10n("Tries left:"), remainingTries);
	}
	else {
		message[0] = 0;
	}
	SetDialogItemText(hwnd, IDC_ERROR, message);
	SetDlgItemText(hwnd, IDC_PIN2, L"");	
	return CKR_OK;
}

void EndDialogIfPIN2Blocked(HWND hwnd, int cardStatus) {
	if(cardStatus == CKR_PIN_LOCKED) {
		ShowWindow(hwnd, false);
		showAlert(hwnd, l10n("PIN2 blocked, cannot sign!"));
		EndDialog(hwnd, IDCANCEL);
	}
}

INT_PTR CALLBACK Pin2DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static TCHAR *pin2;
	static unsigned int minPin2Length;
	static char* certId;
	static char* hash;
	static char* signature;
	static int retryCount;
	int cardStatus;
	char* tmpSignature = NULL;
	EstEID_PINPromptData pinPromptData = {NULL, NULL, NULL, NULL};

	switch(message) {
		case WM_INITDIALOG:
			dialogWindowHandle = hwnd;
			EstEID_log("PIN2 dialog window handle = %08X", dialogWindowHandle);
			SetPinDialogLabels(dialogWindowHandle, ((DialogData *)lParam)->certId, &minPin2Length);				
			SetDialogItemText(dialogWindowHandle, IDC_PIN2_LABEL, l10n("For signing enter PIN2:"));
			SetDialogItemText(dialogWindowHandle, IDOK, l10n("Sign"));
			SetDialogItemText(dialogWindowHandle, IDCANCEL, l10n("Cancel"));
			pin2 = ((DialogData *)lParam)->pin2;
			certId = ((DialogData *)lParam)->certId;
			hash = ((DialogData *)lParam)->hash;
			signature = ((DialogData *)lParam)->signature;
			retryCount = 0;
			EndDialogIfPIN2Blocked(hwnd, SetUpPin2Dialog(hwnd, retryCount, certId));
			return true;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					GetDlgItemText(hwnd, IDC_PIN2, pin2, PIN2_MAX_LEN);
					pinPromptData.pin2 = strdup(CW2A(pin2)); //NB!!! vaata yle maluhaldus					
					tmpSignature = EstEID_sign(strdup(certId), strdup(hash), pinPromptData); // duplicates used as EstEID_sign() will free them					
					retryCount++;
					if(tmpSignature) {
						strcpy(signature, tmpSignature);
						free(tmpSignature);
					}
					else {
						cardStatus = SetUpPin2Dialog(hwnd, retryCount, certId);
						if( cardStatus!= CKR_OK) {
							EndDialogIfPIN2Blocked(hwnd, cardStatus);
						}
						return true;
					}
				case IDCANCEL:
					dialogWindowHandle = 0;
					EndDialog(hwnd, LOWORD(wParam));
					EstEID_log("closing PIN2 dialog, focus is back @ %08X", GetFocus());
					return true;
			}
			if(HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_PIN2) {
				Button_Enable(GetDlgItem(hwnd, IDOK), isAcceptableLengthPIN2(hwnd, IDC_PIN2, minPin2Length));
			}
			return true;
		case WM_CTLCOLORSTATIC:			
			if(GetDlgItem(hwnd, IDC_ERROR) == ((HWND)lParam)){
				return SetMessageLabelColorToRed(wParam);
			}			
		default:
			return false;
	}	
}

INT_PTR CALLBACK PinPadDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
		case WM_INITDIALOG:
			dialogWindowHandle = hwnd;

			SetPinPadDialogLabels(dialogWindowHandle, lParam);
			SetDialogItemText(hwnd, IDC_PINPAD_PIN2_LABEL, l10n("For signing enter PIN2 from PIN pad"));

			countdownValue = 30; // in seconds
			EstEID_log("initializing pinpad countdown timer with value %i", countdownValue);
			SendMessage(GetDlgItem(dialogWindowHandle, IDC_PROGRESSBAR), PBM_SETSTATE, (WPARAM)PBST_PAUSED, 0L);
			SendMessage(GetDlgItem(dialogWindowHandle, IDC_PROGRESSBAR), PBM_SETRANGE, 0, MAKELPARAM(0, countdownValue));
			UpdateProgerssBar(dialogWindowHandle, countdownValue);
			SendMessage(GetDlgItem(dialogWindowHandle, IDC_PROGRESSBAR), PBM_SETSTATE, (WPARAM)PBST_NORMAL, 0L);
			SetTimer(dialogWindowHandle, 1, 1000, NULL);
			break;
		case WM_DESTROY:
			KillCountdownTimer();
			break;
		case WM_TIMER:
			CountdownTimerProc();
			break;
		case WM_CTLCOLORSTATIC:
			if(GetDlgItem(hwnd, IDC_ERROR) == ((HWND)lParam)){
				return SetMessageLabelColorToRed(wParam);
			}
		default:			
			return false;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);	
}

void closePinPadModalSheet() {
	LOG_LOCATION;
	EndDialog(dialogWindowHandle, IDCANCEL);
}

char *promptForPIN(void *nativeWindowHandle, const char *name, const char *message, unsigned int minPin2Length, int usePinPad) {	
	LOG_LOCATION;
	DialogData dialogData;

	if(usePinPad){
		dialogData.pin2[0] = '\0';
		dialogData.name = name;
		dialogData.message = message;
		dialogData.minPin2Length = minPin2Length;
		DialogBoxParamW(pluginInstance, MAKEINTRESOURCEW(IDD_PINPAD_DIALOG), (HWND)nativeWindowHandle, PinPadDialogProc, (LPARAM)&dialogData);
	}
	return strdup("");
}
