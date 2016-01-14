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

#include <stdlib.h>
#include <string.h>

#include <openssl/x509.h>
#include <openssl/pem.h>

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#define CRYPTOKI_COMPAT
#include "pkcs11.h"

#include "esteid_sign.h"
#include "esteid_log.h"
#include "pkcs11_errors.h"
#include "l10n.h"
#include "esteid_error.h"
#include "dialogs.h"
#include "esteid_certinfo.h"

#define FAILURE 0
#define SUCCESS 1

#define BINARY_SHA1_LENGTH 20
#define BINARY_SHA224_LENGTH 28
#define BINARY_SHA256_LENGTH 32
#define BINARY_SHA512_LENGTH 64

CK_BYTE RSA_SHA1_DESIGNATOR_PREFIX[] = {48, 33, 48, 9, 6, 5, 43, 14, 3, 2, 26, 5, 0, 4, 20};
CK_BYTE RSA_SHA224_DESIGNATOR_PREFIX[] = {0x30, 0x2d, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04, 0x05, 0x00, 0x04, 0x1c};
CK_BYTE RSA_SHA256_DESIGNATOR_PREFIX[] = {48, 49, 48, 13, 6, 9, 96, 134, 72, 1, 101, 3, 4, 2, 1, 5, 0, 4, 32};
CK_BYTE RSA_SHA512_DESIGNATOR_PREFIX[] = {0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40};

extern CK_FUNCTION_LIST_PTR fl;

extern char EstEID_error[1024];
extern int EstEID_errorCode;

#ifdef _WIN32
#define CLOSE_SESSION_AND_RETURN(_r) { if (session) { fl->C_CloseSession(session); } return (_r); }
#else
#define CLOSE_SESSION_AND_RETURN(_r) { if (name) free(name); if (session) { fl->C_CloseSession(session); } return FAILURE; }
#endif

#ifdef _WIN32
#define pthread_mutex_t HANDLE
#define pthread_cond_t HANDLE
#endif

pthread_mutex_t pinpad_thread_mutex;
pthread_cond_t pinpad_thread_condition;
int pinpad_thread_result;
int pinpad_thread_completed = FALSE;

int EstEID_getRemainingTries(CK_SLOT_ID slotID) {
	CK_TOKEN_INFO tokenInfo;
	if (EstEID_CK_failure("C_GetTokenInfo", fl->C_GetTokenInfo(slotID, &tokenInfo))) return -1;
	EstEID_log("flags: %li (%lx)", tokenInfo.flags, tokenInfo.flags);
	if (tokenInfo.flags & CKF_USER_PIN_LOCKED) return 0;
	else if (tokenInfo.flags & CKF_USER_PIN_FINAL_TRY) return 1;
	else if (tokenInfo.flags & CKF_USER_PIN_COUNT_LOW) return 2;
	else return 3;
}

int EstEID_isPinPad(CK_SLOT_ID slotID) {
	CK_TOKEN_INFO tokenInfo;
	if (EstEID_CK_failure("C_GetTokenInfo", fl->C_GetTokenInfo(slotID, &tokenInfo))) return 0;
	EstEID_log("flags: %li (%lx)", tokenInfo.flags, tokenInfo.flags);
	if (tokenInfo.flags & CKF_PROTECTED_AUTHENTICATION_PATH) return 1;
	else return 0;
}


char *EstEID_addPadding(const char *hash, unsigned int hashLength, unsigned int *paddedHashLength) {	
	CK_BYTE *padding;
	char *hashWithPadding;
	int paddingLength;
	EstEID_log("incoming digest length = %u", hashLength);
	switch (hashLength) {
		case BINARY_SHA1_LENGTH:
			EstEID_log("SHA1");
			padding = RSA_SHA1_DESIGNATOR_PREFIX;
			paddingLength = sizeof(RSA_SHA1_DESIGNATOR_PREFIX);			
			break;
		case BINARY_SHA224_LENGTH:
			EstEID_log("SHA224");
			padding = RSA_SHA224_DESIGNATOR_PREFIX;
			paddingLength = sizeof(RSA_SHA224_DESIGNATOR_PREFIX);
			break;
		case BINARY_SHA256_LENGTH:
			EstEID_log("SHA256");
			padding = RSA_SHA256_DESIGNATOR_PREFIX;
			paddingLength = sizeof(RSA_SHA256_DESIGNATOR_PREFIX);
			break;
		case BINARY_SHA512_LENGTH:
			EstEID_log("SHA512");
			padding = RSA_SHA512_DESIGNATOR_PREFIX;
			paddingLength = sizeof(RSA_SHA512_DESIGNATOR_PREFIX);
			break;
		default:
			EstEID_log("incorrect digest length, dropping padding");
			*paddedHashLength = 0;
			return NULL;
	}

	hashWithPadding = (char *)malloc(paddingLength + hashLength);
	memcpy(hashWithPadding, padding, paddingLength);
	memcpy(hashWithPadding + paddingLength, hash, hashLength);
	*paddedHashLength = paddingLength + hashLength;
	return hashWithPadding;
}


THREAD_RETURN_TYPE EstEID_pinPadLogin(void* threadData) {
#ifndef _WIN32
	LOG_LOCATION;
	pthread_mutex_lock(&pinpad_thread_mutex);
	CK_SESSION_HANDLE session = ((EstEID_PINPadThreadData*)threadData)->session;
	CK_RV loginResult = fl->C_Login(session, CKU_USER, NULL, 0);		
	((EstEID_PINPadThreadData*)threadData)->result = loginResult;

	closePinPadModalSheet();
	EstEID_log("modal sheet/dialog destroyed");

	pinpad_thread_completed = TRUE;
	pthread_cond_broadcast(&pinpad_thread_condition);

	pthread_mutex_unlock(&pinpad_thread_mutex);
	pthread_exit(NULL);
#else
	EstEID_PINPromptDataEx* pinPromptDataEx;
	LOG_LOCATION;
	WaitForSingleObject(pinpad_thread_mutex, INFINITE);
	pinPromptDataEx = (EstEID_PINPromptDataEx*)threadData;
	pinPromptDataEx->pinPromptData.promptFunction(NULL, pinPromptDataEx->name, pinPromptDataEx->message, 0, TRUE);	
	ReleaseMutex(pinpad_thread_mutex);
	return TRUE;
#endif
}

void setUserCancelErrorCodeAndMessage() {
	EstEID_log("Got user cancel");
	sprintf(EstEID_error, "User cancelled");
	EstEID_errorCode = ESTEID_USER_CANCEL;
}

char* EstEID_getFullNameWithPersonalCode(EstEID_Map cert) {
	const char *surname;
	const char *personalID;
	const char *givenName; 
	char *name;	
	size_t nameSize = 0;
	
	givenName = EstEID_mapGet(cert, "givenName");
	if (!givenName) givenName = "";
	
	surname = EstEID_mapGet(cert, "surname");
	if (!surname) surname = "";
	
	personalID = EstEID_mapGet(cert, "serialNumber");
	if (!personalID) personalID = "";

	nameSize = strlen(givenName) + strlen(surname) + strlen(personalID) + 4;
	name = (char *)malloc(nameSize);
	sprintf(name,"%s %s", givenName, surname);
	if(strlen(personalID)) {
		strcat(name, ", ");
		strcat(name, personalID);
	}

	return name;
}

int EstEID_RealSign(CK_SESSION_HANDLE session, char **signature, unsigned int *signatureLength, const char *hash, unsigned int hashLength, char* name, unsigned int privateKeyIndex) {
	CK_ULONG objectCount;
	unsigned int hashWithPaddingLength = 0;
	char *hashWithPadding;
	CK_MECHANISM mechanism = {CKM_RSA_PKCS, 0, 0};
	CK_OBJECT_CLASS objectClass = CKO_PRIVATE_KEY;
	CK_ATTRIBUTE searchAttribute = {CKA_CLASS, &objectClass, sizeof(objectClass)};
    
    unsigned int max = privateKeyIndex + 1;
    CK_OBJECT_HANDLE privateKeyHandle[max];

	if (EstEID_CK_failure("C_FindObjectsInit", fl->C_FindObjectsInit(session, &searchAttribute, 1))) CLOSE_SESSION_AND_RETURN(FAILURE);

	if (EstEID_CK_failure("C_FindObjects", fl->C_FindObjects(session, privateKeyHandle, max, &objectCount))) CLOSE_SESSION_AND_RETURN(FAILURE);
	if (EstEID_CK_failure("C_FindObjectsFinal", fl->C_FindObjectsFinal(session))) CLOSE_SESSION_AND_RETURN(FAILURE);

	if (objectCount == 0) CLOSE_SESSION_AND_RETURN(FAILURE); // todo ?? set error message
    EstEID_log("found %i private keys in slot, using key in position %i", objectCount, privateKeyIndex);

    if (EstEID_CK_failure("C_SignInit", fl->C_SignInit(session, &mechanism, privateKeyHandle[privateKeyIndex]))) CLOSE_SESSION_AND_RETURN(FAILURE);

	hashWithPadding = EstEID_addPadding(hash, hashLength, &hashWithPaddingLength);
	if (hashWithPadding) { // This is additional safeguard, as digest length is checked already before calling EstEID_addPadding()
		CK_ULONG len;
		if (EstEID_CK_failure("C_Sign", fl->C_Sign(session, (CK_BYTE_PTR)hashWithPadding, hashWithPaddingLength, NULL, &len))) {
			free(hashWithPadding);
			CLOSE_SESSION_AND_RETURN(FAILURE);
		}
		*signature = (char *)malloc(len);
		if (EstEID_CK_failure("C_Sign", fl->C_Sign(session, (CK_BYTE_PTR)hashWithPadding, hashWithPaddingLength, (CK_BYTE_PTR) * signature, &len))) {
			free(hashWithPadding);
			CLOSE_SESSION_AND_RETURN(FAILURE);
		}
		*signatureLength = len;
		free(hashWithPadding);		
	}

	if (session) {
		if (EstEID_CK_failure("C_CloseSession", fl->C_CloseSession(session))) {
			return FAILURE;
		}
	}

	if(name) {
		free(name);
	}
  
	if (!hashWithPaddingLength) { // This is additional safeguard, as digest length is checked already before calling EstEID_addPadding()
		EstEID_log("will not sign due to incorrect incoming message digest length");
		return FAILURE;
	}
	EstEID_log("successfully signed");
	return SUCCESS;
}


int EstEID_getSlotId(char* certId, CK_SLOT_ID* slotId) {
	int certIndex;
	EstEID_Certs *certs = EstEID_loadCerts();
	EstEID_log("certs loaded");
	if (!certs) {
		EstEID_log("%s", EstEID_error);
		return FALSE;
	}
	else if ((certIndex = EstEID_findNonRepuditionCert(certs, certId)) == NOT_FOUND) {
		EstEID_log("card is changed");
		return FALSE;
	}
	*slotId = certs->slotIDs[certIndex];
	return TRUE;
}


#ifdef _WIN32
int EstEID_sighHashWindows(char **signature, unsigned int *signatureLength, CK_SLOT_ID slotID, EstEID_Map cert, const char *hash, unsigned int hashLength, EstEID_PINPromptData pinPromptData) {
	CK_SESSION_HANDLE session = 0L;
	char message[1024];
	int remainingTries = 0;	
	CK_RV loginResult = CKR_FUNCTION_CANCELED;
	
	LOG_LOCATION;

	if (EstEID_CK_failure("C_OpenSession", fl->C_OpenSession(slotID, CKF_SERIAL_SESSION, NULL_PTR, NULL_PTR, &session))) return FAILURE;

	remainingTries = EstEID_getRemainingTries(slotID);
	EstEID_log("EstEID_getRemainingTries(slotID) = %i", remainingTries);
	if (remainingTries == -1)
		CLOSE_SESSION_AND_RETURN(FAILURE);
	if (!remainingTries) {
		sprintf_s(EstEID_error, ESTEID_ERROR_SIZE, "C_Login error: %s (%li)", pkcs11_error_message(CKR_PIN_LOCKED), CKR_PIN_LOCKED);
		CLOSE_SESSION_AND_RETURN(FAILURE);
	}
	if (remainingTries < 3) {
		sprintf_s(message, 1024, "%s %i", l10n("Tries left:"), remainingTries);
	}
	else {
		message[0] = 0;
	}

	loginResult = fl->C_Login(session, CKU_USER, (unsigned char *)pinPromptData.pin2, strlen(pinPromptData.pin2));
	if(loginResult != CKR_OK) {
		EstEID_log("loginResult = %s", pkcs11_error_message(loginResult));
		sprintf_s(EstEID_error, 1024, "C_Login error: %s (%li)", pkcs11_error_message(loginResult), loginResult);
		CLOSE_SESSION_AND_RETURN(loginResult);
	}
	
	return EstEID_RealSign(session, signature, signatureLength, hash, hashLength, NULL, 0);
}
#endif

int EstEID_signHash(char **signature, unsigned int *signatureLength, CK_SLOT_ID slotID, EstEID_Map cert, const char *hash, unsigned int hashLength, EstEID_PINPromptData pinPromptData) {
	CK_SESSION_HANDLE session = 0L;
	CK_RV loginResult = CKR_FUNCTION_CANCELED;
	char *name;
	char message[1024];
	int remainingTries = -1;
	int attempt = 0, blocked = FALSE;
	int isPinPad;
    unsigned int privateKeyIndex;
#ifdef _WIN32
	EstEID_PINPromptDataEx pinPromptDataEx;
#endif

	
	LOG_LOCATION;

	if (EstEID_CK_failure("C_OpenSession", fl->C_OpenSession(slotID, CKF_SERIAL_SESSION, NULL_PTR, NULL_PTR, &session))) return FAILURE;

	name = EstEID_getFullNameWithPersonalCode(cert);
    privateKeyIndex = (unsigned)atoi(EstEID_mapGet(cert, "privateKeyIndex"));
    remainingTries = EstEID_getRemainingTries(slotID);
	for (attempt = 0;; attempt++) {
		if (remainingTries == -1)
			CLOSE_SESSION_AND_RETURN(FAILURE);
		if (!remainingTries || blocked) {
			sprintf(EstEID_error, "C_Login error: %s (%li)", pkcs11_error_message(CKR_PIN_LOCKED), CKR_PIN_LOCKED);
			pinPromptData.alertFunction(pinPromptData.nativeWindowHandle, l10n("PIN2 blocked, cannot sign!"));
			CLOSE_SESSION_AND_RETURN(FAILURE);
		}
		if (remainingTries < 3 || attempt) {
			sprintf(message, "%s%s %i", (attempt ? l10n("Incorrect PIN2! ") : ""), l10n("Tries left:"), remainingTries);
		}
		else {
			message[0] = 0;
		}
		isPinPad = EstEID_isPinPad(slotID);
		if(!isPinPad) {
			// Simple card reader
			char *pin = pinPromptData.promptFunction(pinPromptData.nativeWindowHandle, name, message, (unsigned)atoi(EstEID_mapGet(cert, "minPinLen")), isPinPad);
			if (!pin || strlen(pin) == 0) {
				free(pin);
				setUserCancelErrorCodeAndMessage();
				CLOSE_SESSION_AND_RETURN(FAILURE);
			}
			loginResult = fl->C_Login(session, CKU_USER, (unsigned char *)pin, strlen(pin));
			free(pin);
		}
		else {
			// PIN pad			
#ifdef _WIN32
			EstEID_log("creating pinpad dialog UI thread");
			pinpad_thread_result = -1;
			FAIL_IF_THREAD_ERROR("CreateMutex", (pinpad_thread_mutex = CreateMutex(NULL, FALSE, NULL)));
#else
			EstEID_log("creating pinpad worker thread");
			pinpad_thread_result = -1;
			FAIL_IF_PTHREAD_ERROR("pthread_mutex_init", pthread_mutex_init(&pinpad_thread_mutex, NULL));
			FAIL_IF_PTHREAD_ERROR("pthread_cond_init", pthread_cond_init(&pinpad_thread_condition, NULL));
			pthread_t pinpad_thread;
			EstEID_PINPadThreadData threadData;
			threadData.session = session;
			threadData.result = CKR_OK;
#endif
			EstEID_log("thread launched");
#ifdef _WIN32
			/*
			NB! Due to Firefox for Windows specific behaviour C_Login() is launched from main thread
			and UI code is running in separate thread if running on Windows.
			*/
			pinPromptDataEx.pinPromptData = pinPromptData;
			pinPromptDataEx.message = message;
			pinPromptDataEx.name = name;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&EstEID_pinPadLogin, (LPVOID)&pinPromptDataEx, 0, NULL);
			loginResult = fl->C_Login(session, CKU_USER, NULL, 0);
			//closePinPadModalSheet();
#else
			FAIL_IF_PTHREAD_ERROR("pthread_create", pthread_create(&pinpad_thread, NULL, EstEID_pinPadLogin, (void*)&threadData));
			pinPromptData.promptFunction(pinPromptData.nativeWindowHandle, name, message, 0, isPinPad);
			loginResult = threadData.result;
#endif
			EstEID_log("pinpad sheet/dialog closed");			
			if (loginResult == CKR_FUNCTION_CANCELED) {
				setUserCancelErrorCodeAndMessage();				
				CLOSE_SESSION_AND_RETURN(FAILURE);
			}
		}
		EstEID_log("loginResult = %s", pkcs11_error_message(loginResult));
		switch (loginResult) {
			case CKR_PIN_LOCKED:
				blocked = TRUE;
			case CKR_PIN_INCORRECT:
                remainingTries--;
			case CKR_PIN_INVALID:
			case CKR_PIN_LEN_RANGE:
				EstEID_log("this was attempt %i, loginResult causes to run next round", attempt);
				continue;
			default:
				if (EstEID_CK_failure("C_Login", loginResult)) CLOSE_SESSION_AND_RETURN(FAILURE);
		}
		break; // Login successful - correct PIN supplied
	}

	return EstEID_RealSign(session, signature, signatureLength, hash, hashLength, name, privateKeyIndex);
}

char *EstEID_base64Encode(const char *input, int length) {
	BIO *memBio;
	BIO *b64Bio;
	char *b;
	int len;
	char *result;

	LOG_LOCATION;

	memBio = BIO_new(BIO_s_mem());
	b64Bio = BIO_new(BIO_f_base64());
	b64Bio = BIO_push(b64Bio, memBio);

	BIO_write(b64Bio, input, length);
	(void)BIO_flush(b64Bio);


	len = BIO_get_mem_data(memBio, &b);
	result = (char *)malloc(len + 1);
	strncpy(result, b, len);
	result[len] = 0;
	BIO_free_all(b64Bio);
	while (result[--len] == '\n') result[len] = 0;
	return result;
}

void EstEID_logBase64(char *message, char* data, int dataLength) {
	char *base64;
	LOG_LOCATION;
	base64 = EstEID_base64Encode(data, dataLength);
	EstEID_log(message, base64);
	free(base64);
}

char *EstEID_hex2bin(const char *hex) {
	size_t binLength;
	char *bin;
	char *c;
	char *h;
	int i = 0;

	LOG_LOCATION;
	
	binLength = strlen(hex) / 2;
	bin = (char *)malloc(binLength);
	c = bin;
	h = (char *)hex;
	while (*h) {
		int x;
		sscanf(h, "%2X", &x);
		*c = x;
		c++;
		h += 2;
		i++;
	}
	return bin;
}

int EstEID_signHashHex(char **signatureHex, CK_SLOT_ID slotID, EstEID_Map cert, const char *hashHex, EstEID_PINPromptData pinPromptData) {
	int expectedLengthSHA1 = BINARY_SHA1_LENGTH * 2;
	int expectedLengthSHA256 = BINARY_SHA256_LENGTH * 2;
	int expectedLengthSHA224 = BINARY_SHA224_LENGTH * 2;
	int expectedLengthSHA512 = BINARY_SHA512_LENGTH * 2;

	int hashHexLength, result = SUCCESS;
	char *hash, *signature;
	unsigned int signatureLength, hashLength;

	LOG_LOCATION;

	hashHexLength = strlen(hashHex);

	if (hashHexLength != expectedLengthSHA1 &&
		hashHexLength != expectedLengthSHA224 &&
		hashHexLength != expectedLengthSHA256 &&
		hashHexLength != expectedLengthSHA512) {
		sprintf(EstEID_error, "invalid incoming hash length: %i", hashHexLength);
		EstEID_errorCode = ESTEID_INVALID_HASH_ERROR;
		return FAILURE;
	}
	hashLength = hashHexLength / 2;
	hash = EstEID_hex2bin(hashHex);
	EstEID_logBase64("hash[base64] = %s", hash, hashLength);

#ifdef _WIN32
	//NB!!! error codes may mess this up, as earlier retun values were only true/false
	if(EstEID_isPinPad(slotID)){
		EstEID_log("pinpad detected");
		result = EstEID_signHash(&signature, &signatureLength, slotID, cert, hash, hashLength, pinPromptData);
	}
	else {
		EstEID_log("simple card reader detected");
		result = EstEID_sighHashWindows(&signature, &signatureLength, slotID, cert, hash, hashLength, pinPromptData);
	}
#else
	result = EstEID_signHash(&signature, &signatureLength, slotID, cert, hash, hashLength, pinPromptData);
#endif
	EstEID_log("result = %i (where failure = %i, success = %i)", result, FAILURE, SUCCESS);

	if (result==SUCCESS) {
		EstEID_logBase64("signature[base64] = %s", signature, signatureLength);
		*signatureHex = EstEID_bin2hex(signature, signatureLength);
		free(hash);
		free(signature);
		return SUCCESS;
	}
	else {
		free(hash);
		return FAILURE;
	}
}

int EstEID_findNonRepuditionCert(EstEID_Certs *certs, const char *certId) {
	unsigned int i = 0;
	for (i = 0; i < certs->count; i++) {
		EstEID_Map cert = certs->certs[i];
		if (EstEID_mapGet(cert, "usageNonRepudiation") && !strcmp(certId, EstEID_mapGet(cert, "certHash"))) {
			return i;
		}
	}
	return NOT_FOUND;
}

char *EstEID_sign(char *certId, char *hash, EstEID_PINPromptData pinPromptData) {	
	char *signature = NULL;
	int i;
	EstEID_Certs *certs;
	
	EstEID_log("called, hash=%s", hash);

	certs = EstEID_loadCerts();
	EstEID_log("certs loaded");
	if (!certs) {
		EstEID_log("%s", EstEID_error);
	}
	else if ((i = EstEID_findNonRepuditionCert(certs, certId)) == NOT_FOUND) {
		snprintf(EstEID_error, sizeof(EstEID_error) - 1, "no cert has ID: %s", certId);
		EstEID_errorCode = ESTEID_CERT_NOT_FOUND_ERROR;
		EstEID_error[sizeof(EstEID_error) - 1] = 0;
		EstEID_log("%s", EstEID_error);
	}
	else if (EstEID_signHashHex(&signature, certs->slotIDs[i], certs->certs[i], hash, pinPromptData)) {
		EstEID_log("signature=%s", signature);
	}
	else {
		EstEID_log("%s", EstEID_error);
	}
	free(certId);
	free(hash);
	return signature;
}
