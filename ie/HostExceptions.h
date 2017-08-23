/*
 * Chrome Token Signing Native Host
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

#include <stdexcept>

#include "esteid_error.h"

class BaseException : public std::runtime_error {
private:
	int errorCode;
public:
	BaseException(int code, const std::string &msg) :runtime_error(msg), errorCode(code) {}
	int getErrorCode() const { return errorCode; }
};

class TechnicalException : public BaseException {
public:
	TechnicalException(const std::string &message) :BaseException(ESTEID_UNKNOWN_ERROR, message) {}
};

class InvalidArgumentException : public BaseException {
public:
	InvalidArgumentException(const std::string &message) :BaseException(ESTEID_UNKNOWN_ERROR, message) {}
};

class InvalidHashException : public BaseException {
public:
	InvalidHashException() :BaseException(ESTEID_INVALID_HASH_ERROR, "Invalid Hash") {}
};

class NotAllowedException : public BaseException {
public:
	NotAllowedException(const std::string &message) :BaseException(ESTEID_SITE_NOT_ALLOWED, message) {}
};

class UserCancelledException : public BaseException {
public:
	UserCancelledException() :BaseException(ESTEID_USER_CANCEL, "User cancelled") {}
	UserCancelledException(const std::string &message) :BaseException(ESTEID_USER_CANCEL, message) {}
};

class NoCertificatesException : public BaseException {
public:
	NoCertificatesException() :BaseException(ESTEID_CERT_NOT_FOUND_ERROR, "Cert not found") {}
};

class NotSelectedCertificateException : public BaseException {
public:
	NotSelectedCertificateException() :BaseException(ESTEID_CERT_NOT_FOUND_ERROR, "Unable to sign with certificate that has not been selected by the user") {}
};

class InconsistentOriginException : public BaseException {
public:
	InconsistentOriginException() :BaseException(ESTEID_SITE_NOT_ALLOWED, "Request origin can't change between requests") {}
};

class PinBlockedException : public BaseException {
public:
	PinBlockedException() :BaseException(ESTEID_PIN_BLOCKED, "Maximum number of PIN entry attempts has been reached") {}
};

