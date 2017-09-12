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

#include "plugin.h"
#include "BinaryUtils.h"
#include "Logger.h"

NPObject *certAllocate(NPP npp, NPClass *theClass) {
    _log("");
    return new CertInstance;
}

void certDeallocate(CertInstance *obj) {
    _log("");
    delete obj;
}

void certInvalidate(CertInstance *obj) {
    _log("");
}

bool certHasMethod(CertInstance *obj, NPIdentifier name) {
    NPUTF8 *nameString = obj->parent->browserFunctions->utf8fromidentifier(name);
    _log("name=%s", nameString);
    obj->parent->browserFunctions->memfree(nameString);
    return false;
}

bool certInvoke(CertInstance *obj, NPIdentifier name, NPVariant *args, uint32_t argCount, NPVariant *result) {
    NPUTF8 *nameString = obj->parent->browserFunctions->utf8fromidentifier(name);
    _log("name=%s", nameString);
    obj->parent->browserFunctions->memfree(nameString);
    return false;
}

bool certInvokeDefault(CertInstance *obj, NPVariant *args, uint32_t argCount, NPVariant *result) {
    _log("");
    return false;
}

bool certHasProperty(CertInstance *obj, NPIdentifier name) {
    NPUTF8 *nameString = obj->parent->browserFunctions->utf8fromidentifier(name);
    _log("name=%s", nameString);
    obj->parent->browserFunctions->memfree(nameString);
    return
        isSameIdentifier(name, "id") ||
        isSameIdentifier(name, "cert") ||
        isSameIdentifier(name, "certificateAsHex");
}

bool certGetProperty(CertInstance *obj, NPIdentifier name, NPVariant *variant) {
    NPUTF8 *nameString = obj->parent->browserFunctions->utf8fromidentifier(name);
    _log("name=%s", nameString);
    obj->parent->browserFunctions->memfree(nameString);
    std::string result;
    if (isSameIdentifier(name, "id")) {
        result = BinaryUtils::bin2hex(md5(obj->parent->certInfo));
    }
    else if (isSameIdentifier(name, "certificateAsHex") ||
             isSameIdentifier(name, "cert")) {
        result = BinaryUtils::bin2hex(obj->parent->certInfo);
    }
    if (result.empty())
        return false;
    return copyStringToNPVariant(result.c_str(), variant);
}

bool certSetProperty(CertInstance *obj, NPIdentifier name, const NPVariant *variant) {
    _log("");
    return false;
}

NPClass *certClass() {
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
        (NPRemovePropertyFunctionPtr)NULL,
        (NPEnumerationFunctionPtr)NULL,
        (NPConstructFunctionPtr)NULL
    };
    return &_class;
}

