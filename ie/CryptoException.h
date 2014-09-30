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

#ifndef __CRYPTOEXCEPTION_H__
#define __CRYPTOEXCEPTION_H__

#pragma once
#include <iostream>
#include <sstream>
#include "stdafx.h"
#include "esteid_error.h"
extern "C" {
#include "esteid_log.h"
}

class CryptoException {
public:
	std::string _reason;
	DWORD windowsErrorCode;
	
	CryptoException() {
		std::stringstream ss;
		windowsErrorCode = GetLastError();
		ss << std::hex << windowsErrorCode;
		_reason = ss.str();
		EstEID_log("CryptoException: %s", _reason.c_str());
	};
	CryptoException(std::string reason) : _reason (reason) {EstEID_log("reason: %s", reason);};
	
	CryptoException(DWORD errorCode) {
		this->windowsErrorCode = errorCode;
		_reason.assign(getErrorMessage(errorCode));
		EstEID_log("CryptoException: %s", getErrorMessage(errorCode));
	};
	~CryptoException() {};
};

#endif