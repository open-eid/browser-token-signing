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
#include "Exceptions.h"
#include "CertificateSelection.h"
#include "PINDialog.h"

static int is_from_allowed_url(NPNetscapeFuncs *browserFunctions, NPP instanceData) {
    NPObject *windowObject = nullptr;
    browserFunctions->getvalue(instanceData, NPNVWindowNPObject, &windowObject);
    NPIdentifier location = browserFunctions->getstringidentifier("location");
    NPVariant variantValue;
    browserFunctions->getproperty(instanceData, windowObject, location, &variantValue);

    NPObject *locationObj = NPVARIANT_TO_OBJECT(variantValue);
    NPIdentifier href = browserFunctions->getstringidentifier("href");
    browserFunctions->getproperty(instanceData, locationObj, href, &variantValue);

    std::string stringValue = toString(variantValue);
    _log("href=%s", stringValue.c_str());

    size_t pos = stringValue.find(":");
    if (pos == std::string::npos) {
        _log("hdetected protocol: null");
        return false;
    }

    std::string protocol = stringValue.substr(0, pos);
    int allowed = !strcasecmp("https", protocol.c_str());
    _log("protocol %s is %sallowed", protocol.c_str(), allowed ? "" : "not ");
    return allowed;
}

static NPObject *pluginAllocate(NPP npp, NPClass *theClass) {
    _log("");
    return new PluginInstance;
}

static void pluginDeallocate(NPObject *obj) {
    _log("");
    delete (PluginInstance*)obj;
}

static void pluginInvalidate(NPObject *obj) {
    _log("");
}

static bool pluginHasMethod(NPObject *obj, NPIdentifier name) {
    _log("name=%s", toString(name).c_str());
    return
        name == toIdentifier("sign") ||
        name == toIdentifier("getCertificate") ||
        name == toIdentifier("getVersion");
}

static bool pluginInvoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
    PluginInstance *obj = (PluginInstance*)npobj;
    _log("name=%s argCount=%u", toString(name).c_str(), argCount);
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

    if (name == toIdentifier("getVersion")) {
        return setValue(result, VERSION);
    }

    CHECK(!is_from_allowed_url(obj->browserFunctions, obj->npp), "Site is not allowed", ESTEID_SITE_NOT_ALLOWED)

    if (name == toIdentifier("sign")) {
        CHECK(argCount < 2, "Missing arguments", ESTEID_CERT_NOT_FOUND_ERROR)

        std::string certId = toString(args[0]);
        CHECK(BinaryUtils::hex2bin(certId) != md5(obj->certInfo), "CertID does not match", ESTEID_CERT_NOT_FOUND_ERROR)

        std::string hash = toString(args[1]);
        _log("certId %s hash %s", certId.c_str(), hash.c_str());
        NSDictionary *params = @{
            @"hash": [[NSString alloc] initWithUTF8String:hash.c_str()],
            @"cert": [[NSString alloc] initWithUTF8String:BinaryUtils::bin2hex(obj->certInfo).c_str()]
        };

        if (argCount > 2) {
            Labels::l10n.setLanguage(toString(args[2]));
        }

        if (argCount > 3) {
            NSMutableDictionary *tmp = [[NSMutableDictionary alloc] initWithDictionary:params];
            tmp[@"info"] = [[NSString alloc] initWithUTF8String:toString(args[3]).c_str()];
            params = tmp;
        }

        NSDictionary *dict = [PINPanel show:params cert:params[@"cert"]];
        CHECK([@"user_cancel" isEqualToString:dict[@"result"]], "User canceled", ESTEID_USER_CANCEL)
        CHECK([@"pin_blocked" isEqualToString:dict[@"result"]], "PIN blocked", ESTEID_PIN_BLOCKED)
        CHECK([@"driver_error" isEqualToString:dict[@"result"]], "Failed to load driver", ESTEID_LIBRARY_LOAD_ERROR)
        CHECK([@"technical_error" isEqualToString:dict[@"result"]], "Technical error", ESTEID_UNKNOWN_ERROR)
        CHECK([@"invalid_argument" isEqualToString:dict[@"result"]], "Invalid argument", ESTEID_UNKNOWN_ERROR)

        NSString *signature = dict[@"signature"];
        return setValue(result, signature.UTF8String);
    }
    if (name == toIdentifier("getCertificate")) {
        std::string filter;
        if (argCount > 0) {
            filter = toString(args[0]);
        }

        NSDictionary *dict = [CertificateSelection show:filter != "AUTH"];
        CHECK([@"driver_error" isEqualToString:dict[@"result"]], "Failed to load driver", ESTEID_LIBRARY_LOAD_ERROR)
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

static bool pluginInvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
    _log("");
    return false;
}

static bool pluginHasProperty(NPObject *obj, NPIdentifier name) {
    _log("name=%s", toString(name).c_str());
    return
        name == toIdentifier("version") ||
        name == toIdentifier("errorMessage") ||
        name == toIdentifier("errorCode") ||
        name == toIdentifier("pluginLanguage");
}

static bool pluginGetProperty(NPObject *npobj, NPIdentifier name, NPVariant *variant) {
    PluginInstance *obj = (PluginInstance*)npobj;
    _log("name=%s", toString(name).c_str());
    if (name == toIdentifier("version")) {
        return setValue(variant, VERSION);
    }
    if (name == toIdentifier("errorMessage")) {
        return setValue(variant, obj->error.c_str());
    }
    if (name == toIdentifier("errorCode")) {
        INT32_TO_NPVARIANT(obj->errorCode, *variant);
        return true;
    }
    if (name == toIdentifier("pluginLanguage")) {
        return setValue(variant, Labels::l10n.get("language").c_str());
    }
    return false;
}

static bool pluginSetProperty(NPObject *obj, NPIdentifier name, const NPVariant *variant) {
    _log("name=%s", toString(name).c_str());
    if(name == toIdentifier("pluginLanguage")) {
        Labels::l10n.setLanguage(toString(*variant));
        return true;
    }
    return false;
}

NPClass *pluginClass() {
    static NPClass _class = {
        NP_CLASS_STRUCT_VERSION,
        pluginAllocate,
        pluginDeallocate,
        pluginInvalidate,
        pluginHasMethod,
        pluginInvoke,
        pluginInvokeDefault,
        pluginHasProperty,
        pluginGetProperty,
        pluginSetProperty,
        nullptr,
        nullptr,
        nullptr
    };
    return &_class;
}


