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

#include "HostExceptions.h"
#include "PinDialog.h"
extern "C" {
#include "esteid_log.h"
}

wstring CPinDialog::getWrongPinErrorMessage() {
	if (attemptsRemainig <= 0) {
		return L"Invalid PIN was entered too many times. PIN is blocked.";
	}
	wstring msg = L"Invalid PIN was entered. You have ";
	if (attemptsRemainig == 1) {
		return msg + L"1 retry left!";
	}
	return msg + to_wstring(attemptsRemainig) + L" retries left!";
}

char * CPinDialog::getPin() {

	INT_PTR nResponse = DoModal();
	if (attemptsRemainig <= 0) {
		EstEID_log("Pin is blocked");
		throw PinBlockedException();
	}

	if (nResponse == IDOK) {
		return pin;
	}
	else {
		EstEID_log("User cancelled");
		throw UserCancelledException();
	}
}

void CPinDialog::setAttemptsRemaining(int _attemptsRemaining) {
	attemptsRemainig = _attemptsRemaining;
}

void CPinDialog::setInvalidPin(bool wasPinInvalid) {
	invalidPin = wasPinInvalid;
}

