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
#include "plugin-class.h"
#include "version.h"
#include "cert-class.h"
#include "dialogs.h"
#include "esteid_sign.h"
#include "esteid_map.h"
#include "esteid_json.h"
#include "l10n.h"
#include "esteid_error.h"
#include "npapi.h"
#include "esteid_certinfo.h"

extern NPNetscapeFuncs* browserFunctions;
extern char EstEID_error[1024];
extern int EstEID_errorCode;

bool allowedSite;
char pluginLanguage[3];

bool isAllowedSite() {
	if(!allowedSite) {
		sprintf(EstEID_error, "Site is not allowed");
		EstEID_errorCode = ESTEID_SITE_NOT_ALLOWED;
		EstEID_log("called from forbidden site");
		return false;
	}
	return true;
}

bool pluginInvokeDefault(PluginInstance *obj, NPVariant *args, unsigned argCount, NPVariant *result) {
	return false;
}

void pluginInvalidate() {
}

bool pluginSetProperty(PluginInstance *obj, NPIdentifier name, const NPVariant *variant) {
	LOG_LOCATION;
	if(isSameIdentifier(name, "pluginLanguage")) {
		memset(pluginLanguage, '\0', 3);
		if(NPVARIANT_IS_STRING(*variant)) {
			strncpy(pluginLanguage, NPVARIANT_TO_STRING(*variant).UTF8Characters, 2);
			return true;
		}
		return true;
	}
	return false;
}

bool pluginHasMethod(PluginInstance *obj, NPIdentifier name) {
	return 
		isSameIdentifier(name, "sign") ||
		isSameIdentifier(name, "getCertificate") ||		
		isSameIdentifier(name, "getVersion");
}

bool pluginHasProperty(NPClass *theClass, NPIdentifier name) {
	return 
		isSameIdentifier(name, "version") ||
		isSameIdentifier(name, "errorMessage") ||
		isSameIdentifier(name, "errorCode") ||
		isSameIdentifier(name, "authCert") ||
		isSameIdentifier(name, "pluginLanguage") ||
		isSameIdentifier(name, "signCert");
}

void *getNativeWindowHandle(PluginInstance *obj) {
	void *nativeWindowHandle = obj->nativeWindowHandle;
	if (!nativeWindowHandle) {
		browserFunctions->getvalue(obj->npp, NPNVnetscapeWindow, &nativeWindowHandle);
		EstEID_log("reading nativeWindowHandle=%p from browserFunctions", nativeWindowHandle);
	}
	else {
		EstEID_log("reading nativeWindowHandle=%p from PluginInstance", nativeWindowHandle);
	}
	return nativeWindowHandle;
}

char *getLanguageFromOptions(PluginInstance *obj, NPVariant options) {
	LOG_LOCATION;
	NPObject *object = NPVARIANT_TO_OBJECT(options);
	NPIdentifier identifier = browserFunctions->getstringidentifier("language");
	NPVariant languageResult;
	if (browserFunctions->getproperty(obj->npp, object, identifier, &languageResult) && NPVARIANT_IS_STRING(languageResult)) {
		char *language = createStringFromNPVariant(&languageResult);
		EstEID_log("returning language '%s'", language);
		return language;
	}
	EstEID_log("unable to read language from options");
	
	return NULL;
}

bool doSign(PluginInstance *obj, NPVariant *args, unsigned argCount, NPVariant *result) {
	EstEID_log("obj=%p, name=sign argCount=%u", obj, argCount);

	FAIL_IF_NOT_ALLOWED_SITE;

	if (argCount < 2) {
		browserFunctions->setexception(&obj->header, "Missing arguments");
		return false;
	}
	
	if(argCount > 2 && NPVARIANT_IS_OBJECT(args[2])){
		NPUTF8* optionsLanguage = getLanguageFromOptions(obj, args[2]);

		if (NULL != optionsLanguage) {
			strncpy(pluginLanguage, optionsLanguage, 2);
			free(optionsLanguage);
		}
		else {
			strcpy(pluginLanguage, "");
		}
	}
	EstEID_setLocale(pluginLanguage);

	void* wnd = getNativeWindowHandle(obj);

	EstEID_PINPromptData pinPromptData = {promptForPIN, showAlert, wnd, NULL};
	NPUTF8* certId = createStringFromNPVariant(&args[0]);
	NPUTF8* hash = createStringFromNPVariant(&args[1]);
	char *signature = NULL;

	signature = EstEID_sign(certId, hash, pinPromptData);

	LOG_LOCATION
	if (signature) {
		copyStringToNPVariant(signature, result);
		free(signature);
		return true;
	}
	else {
		EstEID_log("EstEID_error=%s", EstEID_error);
		browserFunctions->setexception(&obj->header, EstEID_error);
		return false;
	}
}

bool doGetCertificate(PluginInstance *obj, NPVariant *result) {
	LOG_LOCATION;
	FAIL_IF_NOT_ALLOWED_SITE;    
	char selectedCertID[33];
	int dlg_result = 0;
	
	dlg_result = promptForCertificate(getNativeWindowHandle(obj), selectedCertID);

	EstEID_log("Certificate selection dialog result = %i", dlg_result);
	EstEID_log("Selected certificate ID = %s", selectedCertID);

	if(dlg_result == IDCANCEL) {
		sprintf(EstEID_error, "User canceled");
		EstEID_errorCode = ESTEID_USER_CANCEL;
		EstEID_log("EstEID_error = %s", EstEID_error);
		browserFunctions->setexception(&obj->header, EstEID_error);	
		return false;
	}
	
	CertInstance *certInstance = (CertInstance *)browserFunctions->createobject(obj->npp, certClass());
	certInstance->npp = obj->npp;
	certInstance->certInfo = EstEID_mapClone(EstEID_getNonRepudiationCertById(selectedCertID));
//	certInstance->certInfo = EstEID_mapClone(EstEID_getNonRepudiationCert());

	EstEID_log("certObject=%p", certInstance);
	OBJECT_TO_NPVARIANT((NPObject *)certInstance, *result);
	EstEID_log("result=%p", result);	
	return true;
}

bool doGetCertificates(PluginInstance *obj, NPVariant *result) {
	LOG_LOCATION;
	EstEID_log("obj=%p, name=doGetCertificates", obj);
	NPObject *windowObject = NULL;
	browserFunctions->getvalue(obj->npp, NPNVWindowNPObject, &windowObject);
	NPVariant array;
	browserFunctions->invoke(obj->npp, windowObject, browserFunctions->getstringidentifier("Array"), NULL, 0, &array);
	EstEID_Certs *certs = EstEID_loadCerts();
	for (unsigned i = 0u; i < certs->count; i++) {
		EstEID_Map cert = certs->certs[i];
		if (!EstEID_mapGet(cert, "usageNonRepudiation")) continue;
		CertInstance *certInstance = (CertInstance *)browserFunctions->createobject(obj->npp, certClass());
		certInstance->npp = obj->npp;
		certInstance->certInfo = EstEID_mapClone(cert);
		browserFunctions->retainobject((NPObject *)certInstance);
		EstEID_log("certObject=%p", certInstance);
		NPVariant *arg = (NPVariant *)browserFunctions->memalloc(sizeof(NPVariant));
		OBJECT_TO_NPVARIANT((NPObject *)certInstance, *arg);
		NPVariant ret;
		browserFunctions->invoke(obj->npp, array.value.objectValue, browserFunctions->getstringidentifier("push"), arg, 1, &ret);
	}
	browserFunctions->retainobject(array.value.objectValue);
	OBJECT_TO_NPVARIANT(array.value.objectValue, *result);
	return true;
}

bool pluginGetProperty(PluginInstance *obj, NPIdentifier name, NPVariant *variant) {
	LOG_LOCATION;
	if (isSameIdentifier(name, "version"))
		return copyStringToNPVariant(ESTEID_PLUGIN_VERSION, variant);
	else if (isSameIdentifier(name, "errorMessage")){
		EstEID_log("Reading error message: %s", EstEID_error);
		return copyStringToNPVariant(EstEID_error, variant);
	}
	else if (isSameIdentifier(name, "errorCode")) {
		INT32_TO_NPVARIANT(EstEID_errorCode, *variant);
		EstEID_log("returning errorCode=%i", EstEID_errorCode);
		return true;
	}
	else if (isSameIdentifier(name, "authCert") || isSameIdentifier(name, "signCert")){
		return doGetCertificate(obj, variant);
	}
	else if (isSameIdentifier(name, "pluginLanguage")){
		return copyStringToNPVariant(pluginLanguage, variant);
	}
	EstEID_log("returning false");
	return false;
}

bool pluginInvoke(PluginInstance *obj, NPIdentifier name, NPVariant *args, unsigned argCount, NPVariant *result) {	
	LOG_LOCATION;
	EstEID_clear_error();
	EstEID_setLocale(pluginLanguage);

	if (isSameIdentifier(name, "sign")) {
		return doSign(obj, args, argCount, result);
	}
	if (isSameIdentifier(name, "getCertificate")) {
		return doGetCertificate(obj, result);
	}
	if (isSameIdentifier(name, "getVersion")) {
		return pluginGetProperty(obj, browserFunctions->getstringidentifier("version"), result);
	}
	EstEID_log("obj=%p, name=%p, argCount=%u", obj, name, argCount);
	return false;
}

NPObject *pluginAllocate(NPP npp, NPClass *theClass) {
	return(NPObject *)malloc(sizeof(PluginInstance));
}

void pluginDeallocate(PluginInstance *obj) {
	free(obj);
}

static NPClass _class = {
	NP_CLASS_STRUCT_VERSION,
	(NPAllocateFunctionPtr)pluginAllocate,
	(NPDeallocateFunctionPtr)pluginDeallocate,
	(NPInvalidateFunctionPtr)pluginInvalidate,
	(NPHasMethodFunctionPtr)pluginHasMethod,
	(NPInvokeFunctionPtr)pluginInvoke,
	(NPInvokeDefaultFunctionPtr)pluginInvokeDefault,
	(NPHasPropertyFunctionPtr)pluginHasProperty,
	(NPGetPropertyFunctionPtr)pluginGetProperty,
	(NPSetPropertyFunctionPtr)pluginSetProperty,
	(NPRemovePropertyFunctionPtr)NULL,
	(NPEnumerationFunctionPtr)NULL,
	(NPConstructFunctionPtr)NULL
};

NPClass *pluginClass() {
	return &_class;
}


