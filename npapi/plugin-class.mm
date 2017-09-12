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
#include "Labels.h"
#include "Logger.h"
#include "esteid_error.h"
#include "CertificateSelection.h"
#include "PINDialog.h"

static NPObject *pluginAllocate(NPP npp, NPClass *theClass) {
    _log("");
    return new PluginInstance;
}

static void pluginDeallocate(PluginInstance *obj) {
    _log("");
    delete obj;
}

static void pluginInvalidate(PluginInstance *obj) {
    _log("");
}

static bool pluginHasMethod(PluginInstance *obj, NPIdentifier name) {
    NPUTF8 *nameString = obj->browserFunctions->utf8fromidentifier(name);
    _log("name=%s", nameString);
    obj->browserFunctions->memfree(nameString);
    return
        isSameIdentifier(name, "sign") ||
        isSameIdentifier(name, "getCertificate") ||
        isSameIdentifier(name, "getVersion");
}

static bool pluginInvoke(PluginInstance *obj, NPIdentifier name, NPVariant *args, unsigned argCount, NPVariant *result) {
    NPUTF8 *nameString = obj->browserFunctions->utf8fromidentifier(name);
    _log("name=%s argCount=%u", nameString, argCount);
    obj->browserFunctions->memfree(nameString);
    obj->error.clear();
    obj->errorCode = ESTEID_NO_ERROR;

#define CHECK(eval, msg, code) \
    if (eval) { \
        obj->error = msg; \
        obj->errorCode = code; \
        obj->browserFunctions->setexception(obj, obj->error.c_str()); \
        _log(obj->error.c_str()); \
        return false; \
    }

    if (isSameIdentifier(name, "getVersion")) {
        return copyStringToNPVariant(VERSION, result);
    }

    CHECK(!obj->allowedSite, "Site is not allowed", ESTEID_SITE_NOT_ALLOWED)

    if (isSameIdentifier(name, "sign")) {
        CHECK(argCount < 2, "Missing arguments", ESTEID_CERT_NOT_FOUND_ERROR)

        std::string certId = createStringFromNPVariant(args[0]);
        CHECK(BinaryUtils::hex2bin(certId) != md5(obj->certInfo), "CertID does not match", ESTEID_CERT_NOT_FOUND_ERROR)

        std::string hash = createStringFromNPVariant(args[1]);
        _log("certId %s hash %s", certId.c_str(), hash.c_str());
        NSDictionary *params = @{
            @"hash": [[NSString alloc] initWithUTF8String:hash.c_str()],
            @"cert": [[NSString alloc] initWithUTF8String:BinaryUtils::bin2hex(obj->certInfo).c_str()]
        };

        if (argCount > 2 && NPVARIANT_IS_STRING(args[2])) {
            Labels::l10n.setLanguage(createStringFromNPVariant(args[2]));
        }

        if (argCount > 3 && NPVARIANT_IS_STRING(args[3])) {
            NSMutableDictionary *tmp = [[NSMutableDictionary alloc] initWithDictionary:params];
            tmp[@"info"] = [[NSString alloc] initWithUTF8String:createStringFromNPVariant(args[3]).c_str()];
            params = tmp;
        }

        NSDictionary *dict = [PINPanel show:params cert:params[@"cert"]];
        CHECK([@"user_cancel" isEqualToString:dict[@"result"]], "User canceled", ESTEID_USER_CANCEL)
        CHECK([@"pin_blocked" isEqualToString:dict[@"result"]], "PIN blocked", ESTEID_PIN_BLOCKED)
        CHECK([@"technical_error" isEqualToString:dict[@"result"]], "Technical error", ESTEID_UNKNOWN_ERROR)
        CHECK([@"invalid_argument" isEqualToString:dict[@"result"]], "Invalid argument", ESTEID_UNKNOWN_ERROR)

        NSString *signature = dict[@"signature"];
        return copyStringToNPVariant(signature.UTF8String, result);
    }
    if (isSameIdentifier(name, "getCertificate")) {
        std::string filter;
        if (argCount > 0 && NPVARIANT_IS_STRING(args[0])) {
            filter = createStringFromNPVariant(args[0]);
        }

        NSDictionary *dict = [CertificateSelection show:filter != "AUTH"];
        CHECK([@"technical_error" isEqualToString:dict[@"result"]], "Technical error", ESTEID_UNKNOWN_ERROR)
        CHECK([@"no_certificates" isEqualToString:dict[@"result"]], "No certificates", ESTEID_CERT_NOT_FOUND_ERROR)
        CHECK([@"user_cancel" isEqualToString:dict[@"result"]], "User canceled", ESTEID_USER_CANCEL)

        NSString *cert = dict[@"cert"];
        obj->certInfo = BinaryUtils::hex2bin(std::string(cert.UTF8String, cert.length));
        CertInstance *certInstance = (CertInstance *)obj->browserFunctions->createobject(obj->npp, certClass());
        certInstance->parent = obj;

        OBJECT_TO_NPVARIANT(certInstance, *result);
        return true;
    }
    return false;
}

static bool pluginInvokeDefault(PluginInstance *obj, NPVariant *args, unsigned argCount, NPVariant *result) {
    _log("");
    return false;
}

static bool pluginHasProperty(PluginInstance *obj, NPIdentifier name) {
    NPUTF8 *nameString = obj->browserFunctions->utf8fromidentifier(name);
    _log("name=%s", nameString);
    obj->browserFunctions->memfree(nameString);
    return
        isSameIdentifier(name, "version") ||
        isSameIdentifier(name, "errorMessage") ||
        isSameIdentifier(name, "errorCode") ||
        isSameIdentifier(name, "pluginLanguage");
}

static bool pluginGetProperty(PluginInstance *obj, NPIdentifier name, NPVariant *variant) {
    NPUTF8 *nameString = obj->browserFunctions->utf8fromidentifier(name);
    _log("name=%s", nameString);
    obj->browserFunctions->memfree(nameString);
    if (isSameIdentifier(name, "version")) {
        return copyStringToNPVariant(VERSION, variant);
    }
    if (isSameIdentifier(name, "errorMessage")) {
        return copyStringToNPVariant(obj->error.c_str(), variant);
    }
    if (isSameIdentifier(name, "errorCode")) {
        INT32_TO_NPVARIANT(obj->errorCode, *variant);
        return true;
    }
    if (isSameIdentifier(name, "pluginLanguage")) {
        return copyStringToNPVariant(Labels::l10n.get("language").c_str(), variant);
    }
    return false;
}

static bool pluginSetProperty(PluginInstance *obj, NPIdentifier name, const NPVariant *variant) {
    NPUTF8 *nameString = obj->browserFunctions->utf8fromidentifier(name);
    _log("name=%s", nameString);
    obj->browserFunctions->memfree(nameString);
    if(isSameIdentifier(name, "pluginLanguage") && NPVARIANT_IS_STRING(*variant)) {
        Labels::l10n.setLanguage(createStringFromNPVariant(*variant));
        return true;
    }
    return false;
}

NPClass *pluginClass() {
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
    return &_class;
}


