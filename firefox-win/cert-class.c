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
#include "cert-class.h"
#include "plugin-class.h"
#include "esteid_error.h"
#include "esteid_certinfo.h"

extern NPNetscapeFuncs* browserFunctions;
extern bool allowedSite;
extern char EstEID_error[1024];
extern int EstEID_errorCode;


NPObject *certAllocate(NPP npp, NPClass *theClass) {
	return(NPObject *)malloc(sizeof(CertInstance));
}

void certDeallocate(CertInstance *obj) {
	LOG_LOCATION
	free(obj->CN);
	free(obj->certificate);
	free(obj->certificateAsHex);
	free(obj->issuerCN);
	free(obj->validFrom);
	free(obj->validTo);
	free(obj);
}

void certInvalidate() {
}

bool certInvokeDefault(CertInstance *obj, NPVariant *args, uint32_t argCount, NPVariant *result) {
	return false;
}

bool certSetProperty(CertInstance *obj, NPIdentifier name, const NPVariant *variant) {
	return false;
}


bool certHasProperty(NPClass *theClass, NPIdentifier name) {
	FAIL_IF_NOT_ALLOWED_SITE
	static char *certProperties[] = {"id", "cert", "CN", "issuerCN", "keyUsage", "validFrom", "validTo", "certificateAsHex"};
	for (int i = 0; i < sizeof(certProperties) / sizeof(char *); i++) {
		if (isSameIdentifier(name, certProperties[i])) return true;
	}
	return false;
}

bool certGetProperty(CertInstance *obj, NPIdentifier name, NPVariant *variant) {
	NPUTF8* nameString = browserFunctions->utf8fromidentifier(name);
	EstEID_log("name=%s", (char *)nameString);
	if(isSameIdentifier(name, "id")){
		return copyStringToNPVariant(EstEID_getCertHash(obj->certificate), variant);
	}
	if(isSameIdentifier(name, "certificateAsHex") || isSameIdentifier(name, "cert")){
		return copyStringToNPVariant(obj->certificateAsHex, variant);
	}
	if(isSameIdentifier(name, "issuerCN")){
		return copyStringToNPVariant(obj->issuerCN, variant);
	}
	if(isSameIdentifier(name, "CN")){
		EstEID_log("CN=%s", obj->CN);
		return copyStringToNPVariant(obj->CN, variant);
	}
	if(isSameIdentifier(name, "validTo")){
		return copyStringToNPVariant(obj->validTo, variant);
	}
	if(isSameIdentifier(name, "validFrom")){
		return copyStringToNPVariant(obj->validFrom, variant);
	}
	if(isSameIdentifier(name, "keyUsage")){
		return copyStringToNPVariant("Non-Repudiation", variant);
	}
	return false;
}

bool certHasMethod(NPObject *npobj, NPIdentifier name) {
	return isSameIdentifier(name, "toString");
}

bool certInvoke(CertInstance *obj, NPIdentifier name, NPVariant *args, uint32_t argCount, NPVariant *result) {
	if (isSameIdentifier(name, "toString")) {
		return certGetProperty(obj, browserFunctions->getstringidentifier("CN"), result);
	}
	return false;
}

static NPClass _class = {
    NP_CLASS_STRUCT_VERSION,
    (NPAllocateFunctionPtr) certAllocate,
    (NPDeallocateFunctionPtr) certDeallocate,
    (NPInvalidateFunctionPtr) certInvalidate,
    (NPHasMethodFunctionPtr) certHasMethod,
    (NPInvokeFunctionPtr) certInvoke,
    (NPInvokeDefaultFunctionPtr) certInvokeDefault,
    (NPHasPropertyFunctionPtr) certHasProperty,
    (NPGetPropertyFunctionPtr) certGetProperty,
    (NPSetPropertyFunctionPtr) certSetProperty,
};

NPClass *certClass() {
	return &_class;
}