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

#include "Signer.h"
#include "BinaryUtils.h"
#include "HostExceptions.h"
#include "EstEIDHelper.h"
#include <Windows.h>
#include <ncrypt.h>
#include <WinCrypt.h>
#include <cryptuiapi.h>

class SignerFactory {
public:
	static Signer * createSigner(const string &_hash, char *_certId);
private:
	static bool isLithuanianCertificate(PCCERT_CONTEXT certContext);
	static bool certificateMatchesId(PCCERT_CONTEXT certContext, char *certId);
	static PCCERT_CONTEXT findCertificateById(char *certId);
};