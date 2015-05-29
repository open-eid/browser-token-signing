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

#include "esteid_error.h"
#include <string>
#include <stdexcept>

using namespace std;


class BaseException : public runtime_error {
private:
	string description;
	string message;
	int errorCode;
public:
	BaseException(const string &msg, const string &desc, const int &code) :runtime_error(msg), message(msg), description(desc), errorCode(code){}

	int getErrorCode() const{
		return errorCode;
	}
	string getErrorMessage() const{
		return message;
	}
	string getErrorDescription() const{
		return description;
	}
};

class TechnicalException : public BaseException {
public:
	TechnicalException(const string &message) :BaseException("technical_error", message, ESTEID_UNKNOWN_ERROR) {}
};

class InvalidHashException : public BaseException {
public:
	InvalidHashException() :BaseException("invalid_argument", "Invalid Hash", ESTEID_INVALID_HASH_ERROR) {}
};

class NotAllowedException : public BaseException {
public:
	NotAllowedException(const string &message) :BaseException("not_allowed", message, ESTEID_SITE_NOT_ALLOWED) {}
};

class UserCancelledException : public BaseException {
public:
	UserCancelledException() :BaseException("user_cancel", "User cancelled", ESTEID_USER_CANCEL) {}
	UserCancelledException(const string &message) :BaseException("user_cancel", message, ESTEID_USER_CANCEL) {}
};

class NoCertificatesException : public BaseException {
public:
	NoCertificatesException() :BaseException("no_certificates", "Cert not found", ESTEID_CERT_NOT_FOUND_ERROR) {}
};

class PinBlockedException : public BaseException {
public:
	PinBlockedException() :BaseException("pin_blocked", "Maximum number of PIN entry attempts has been reached", ESTEID_PIN_BLOCKED) {}
};

