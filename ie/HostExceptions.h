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
	TechnicalException(const string &message) :BaseException("technical_error", message, 5) {}
};

class InvalidArgumentException : public BaseException {
public:
	InvalidArgumentException(const string &message) :BaseException("invalid_argument", message, 21) {}
};

class InvalidHashException : public BaseException {
public:
	InvalidHashException() :BaseException("invalid_argument", "Invalid Hash", 17) {}
};

class NotAllowedException : public BaseException {
public:
	NotAllowedException(const string &message) :BaseException("not_allowed", message, 19) {}
};

class UserCancelledException : public BaseException {
public:
	UserCancelledException() :BaseException("user_cancel", "User cancelled", 1) {}
	UserCancelledException(const string &message) :BaseException("user_cancel", message, 1) {}
};

class NoCertificatesException : public BaseException {
public:
	NoCertificatesException() :BaseException("no_certificates", "Cert not found", 2) {}
};

class NotSelectedCertificateException : public BaseException {
public:
	NotSelectedCertificateException() :BaseException("invalid_argument", "Unable to sign with certificate that has not been selected by the user", 22) {}
};

class InconsistentOriginException : public BaseException {
public:
	InconsistentOriginException() :BaseException("invalid_argument", "Request origin can't change between requests", 23) {}
};

class PinBlockedException : public BaseException {
public:
	PinBlockedException() :BaseException("pin_blocked", "Maximum number of PIN entry attempts has been reached", 24) {}
};

