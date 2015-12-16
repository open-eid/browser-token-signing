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

#include "Pkcs11Signer.h"
#include "PKCS11CardManager.h"
#include "BinaryUtils.h"
#include "HostExceptions.h"
#include "PinDialog.h"
#include <future>
#include <string>
extern "C" {
#include "esteid_log.h"
}

using namespace std;

void Pkcs11Signer::initialize() {
	LOG_LOCATION
	cardManager = getCardManager();
	pinTriesLeft = cardManager->getPIN2RetryCount();
	dialog = new CPinDialog;
}

unique_ptr<PKCS11CardManager> Pkcs11Signer::getCardManager() {
	try {
		unique_ptr<PKCS11CardManager> manager;
		manager.reset(createCardManager());
		if (!manager) {
			EstEID_log("No card manager found for this certificate");
			throw TechnicalException("No card manager found for this certificate");
		}
		return manager;
	}
	catch (const std::runtime_error &a) {
		EstEID_log("Technical error: %s", a.what());
		throw TechnicalException("Error getting card manager: " + string(a.what()));
	}
}

PKCS11CardManager* Pkcs11Signer::createCardManager() {
	if (pkcs11ModulePath.empty()) {
		return PKCS11CardManager::instance(BinaryUtils::hex2bin(certInHex));
	}
	return PKCS11CardManager::instance(BinaryUtils::hex2bin(certInHex), pkcs11ModulePath);
}

string Pkcs11Signer::sign() {
	EstEID_log("Signing using PKCS#11 module");
	EstEID_log("Hash is %s and cert id (MD5 hash) is %s", getHash()->c_str(), getCertId());
	validateHashLength();
	return askPinAndSignHash();
}

string Pkcs11Signer::askPinAndSignHash() {
	try {
		dialog->setAttemptsRemaining(pinTriesLeft);
		char* signingPin = askPin();
		vector<unsigned char> binaryHash = BinaryUtils::hex2bin(*getHash());
		vector<unsigned char> result = cardManager->sign(binaryHash, signingPin);
		string signature = BinaryUtils::bin2hex(result);
		EstEID_log("Sign result: %s", signature.c_str());
		return signature;
	}
	catch (AuthenticationError &e) {
		EstEID_log("Wrong pin");
		handleWrongPinEntry();
		return askPinAndSignHash();
	}
	catch (AuthenticationBadInput &e) {
		EstEID_log("Bad pin input");
		handleWrongPinEntry();
		return askPinAndSignHash();
	}
}

char* Pkcs11Signer::askPin() {
	char* signingPin = dialog->getPin();
	if (strlen(signingPin) < 4) {
		EstEID_log("Pin is too short");
		dialog->setInvalidPin(true);
		return askPin();
	}
	return signingPin;
}

void Pkcs11Signer::validateHashLength() {
	int length = getHash()->length();
	if (length != BINARY_SHA1_LENGTH * 2 && length != BINARY_SHA224_LENGTH * 2 && length != BINARY_SHA256_LENGTH * 2 && length != BINARY_SHA384_LENGTH * 2 && length != BINARY_SHA512_LENGTH * 2) {
		EstEID_log("Hash length %i is invalid", getHash()->length());
		throw InvalidHashException();
	}
}

void Pkcs11Signer::handleWrongPinEntry() {
	pinTriesLeft--;
	dialog->setInvalidPin(true);
}

void Pkcs11Signer::setPkcs11ModulePath(string &path) {
	EstEID_log("pkcs11 module path is %s", path.c_str());
	pkcs11ModulePath = path;
}