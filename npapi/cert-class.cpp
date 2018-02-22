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

static NPObject *certAllocate(NPP npp, NPClass *theClass) {
    _log("");
    return new CertInstance;
}

static void certDeallocate(NPObject *obj) {
    _log("");
    delete (CertInstance*)obj;
}

static void certInvalidate(NPObject *obj) {
    _log("");
}

static bool certHasMethod(NPObject *obj, NPIdentifier name) {
    _log("");
    return false;
}

static bool certInvoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
    _log("");
    return false;
}

static bool certInvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
    _log("");
    return false;
}

static bool certHasProperty(NPObject *npobj, NPIdentifier name) {
    _log("name=%s", toString(name).c_str());
    return
        name == toIdentifier("id") ||
        name == toIdentifier("cert") ||
        name == toIdentifier("certificateAsHex");
}

static bool certGetProperty(NPObject *npobj, NPIdentifier name, NPVariant *variant) {
    CertInstance *obj = (CertInstance*)npobj;
    _log("name=%s", toString(name).c_str());
    std::string result;
    if (name == toIdentifier("id")) {
        result = BinaryUtils::bin2hex(md5(obj->parent->certInfo));
    }
    else if (name == toIdentifier("certificateAsHex") ||
             name == toIdentifier("cert")) {
        result = BinaryUtils::bin2hex(obj->parent->certInfo);
    }
    if (result.empty())
        return false;
    return setValue(variant, result.c_str());
}

static bool certSetProperty(NPObject *obj, NPIdentifier name, const NPVariant *variant) {
    _log("");
    return false;
}

NPClass *certClass() {
    static NPClass _class = {
        NP_CLASS_STRUCT_VERSION,
        certAllocate,
        certDeallocate,
        certInvalidate,
        certHasMethod,
        certInvoke,
        certInvokeDefault,
        certHasProperty,
        certGetProperty,
        certSetProperty,
        nullptr,
        nullptr,
        nullptr
    };
    return &_class;
}

