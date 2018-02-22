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
#include "Logger.h"

#include <CommonCrypto/CommonDigest.h>

#define PLUGIN_NAME        "Firefox Token Signing"
#define PLUGIN_DESCRIPTION "Allows digital signing with Estonian, Finnish, Latvian and Lithuanian ID cards"
#define MIME_TYPE          "application/x-digidoc"

// Helpers

NPNetscapeFuncs* browserFunctions;

NPIdentifier toIdentifier(const char* name) {
    return browserFunctions->getstringidentifier(name);
}

bool setValue(NPVariant *variant, const char *string) {
    char *out = (char *)browserFunctions->memalloc(uint32_t(strlen(string) + 1));
    strcpy(out, string);
    STRINGZ_TO_NPVARIANT(out, *variant);
    return true;
}

std::string toString(const NPVariant &variant) {
    if (!NPVARIANT_IS_STRING(variant)) {
        return std::string();
    }
    return std::string(NPVARIANT_TO_STRING(variant).UTF8Characters,
                       NPVARIANT_TO_STRING(variant).UTF8Length);
}

std::string toString(NPIdentifier identifier) {
    NPUTF8 *str = browserFunctions->utf8fromidentifier(identifier);
    std::string result = str;
    browserFunctions->memfree(str);
    return result;
}

std::vector<unsigned char> md5(const std::vector<unsigned char> &data) {
    std::vector<unsigned char> md5(CC_MD5_DIGEST_LENGTH, 0);
    CC_MD5(data.data(), (unsigned int)data.size(), md5.data());
    return md5;
}

// NP Fucntions

NP_EXPORT(NPError) NP_GetEntryPoints(NPPluginFuncs *pluginFuncs) {
    _log("");
    pluginFuncs->version = 11;
    pluginFuncs->size = sizeof(*pluginFuncs);
    pluginFuncs->newp = NPP_New;
    pluginFuncs->destroy = NPP_Destroy;
    pluginFuncs->setwindow = NPP_SetWindow;
    pluginFuncs->newstream = NPP_NewStream;
    pluginFuncs->destroystream = NPP_DestroyStream;
    pluginFuncs->asfile = NPP_StreamAsFile;
    pluginFuncs->writeready = NPP_WriteReady;
    pluginFuncs->write = NPP_Write;
    pluginFuncs->print = NPP_Print;
    pluginFuncs->event = NPP_HandleEvent;
    pluginFuncs->urlnotify = NPP_URLNotify;
    pluginFuncs->getvalue = NPP_GetValue;
    pluginFuncs->setvalue = NPP_SetValue;
    return NPERR_NO_ERROR;
}

NP_EXPORT(NPError) NP_Initialize(NPNetscapeFuncs *browserFuncs) {
    _log("");
    browserFunctions = browserFuncs;
    return NPERR_NO_ERROR;
}

NP_EXPORT(NPError) NP_Shutdown(void) {
    _log("");
    return NPERR_NO_ERROR;
}

NP_EXPORT(const char*) NP_GetMIMEDescription() {
    const char *mime = MIME_TYPE "::" PLUGIN_NAME;
    _log("NP_GetMIMEDescription: %s", mime);
    return mime;
}

NP_EXPORT(char*) NP_GetPluginVersion() {
    _log("NP_GetPluginVersion: %s", VERSION);
    return (char *)VERSION;
}

NP_EXPORT(NPError) NP_GetValue(void *instanceData, NPPVariable variable, void *value) {
    return NPP_GetValue(NPP(instanceData), variable, value);
}

NPError NPP_New(NPMIMEType mimeType, NPP instanceData, uint16_t mode, int16_t argc, char* argn[], char* argv[], NPSavedData* saved) {
    _log("LOADED PLUGIN PKCS11 VERSION %s", VERSION);
    _log("instanceData=%p, mimeType=%s", instanceData, mimeType);
#ifdef XP_MACOSX
    browserFunctions->setvalue(instanceData, NPPVpluginEventModel, (void*)NPEventModelCocoa);
    browserFunctions->setvalue(instanceData, NPPVpluginDrawingModel, (void*)NPDrawingModelCoreGraphics);
#endif
    browserFunctions->setvalue(instanceData, NPPVpluginWindowBool, (void*)false);
    PluginInstance *pluginInstance = (PluginInstance *)browserFunctions->createobject(instanceData, pluginClass());
    pluginInstance->browserFunctions = browserFunctions;
    pluginInstance->npp = instanceData;
    instanceData->pdata = pluginInstance;
    return NPERR_NO_ERROR;
}

NPError NPP_Destroy(NPP instanceData, NPSavedData** save) {
    _log("instanceData=%p", instanceData);
    browserFunctions->releaseobject((NPObject *)instanceData->pdata);
    return NPERR_NO_ERROR;
}

NPError NPP_SetWindow(NPP instanceData, NPWindow* window) {
    _log("");
    return NPERR_NO_ERROR;
}

NPError NPP_NewStream(NPP instanceData, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype) {
    _log("");
    return NPERR_NO_ERROR;
}

NPError NPP_DestroyStream(NPP instanceData, NPStream* stream, NPReason reason) {
    _log("");
    return NPERR_NO_ERROR;
}

void NPP_StreamAsFile(NPP instanceData, NPStream* stream, const char* fname) {
    _log("");
}

int32_t NPP_WriteReady(NPP instanceData, NPStream* stream) {
    _log("");
    return 0;
}

int32_t NPP_Write(NPP instanceData, NPStream* stream, int32_t offset, int32_t len, void* buffer) {
    _log("");
    return 0;
}

void NPP_Print(NPP instanceData, NPPrint* platformPrint) {
    _log("");
}

int16_t NPP_HandleEvent(NPP instanceData, void* event) {
    _log("");
    return 0;
}

void NPP_URLNotify(NPP instanceData, const char* url, NPReason reason, void* notifyData) {
    _log("");
}

NPError NPP_GetValue(NPP instanceData, NPPVariable variable, void *value) {
    _log("variable %u", variable);
    switch (variable) {
        case NPPVpluginNameString:
            *((char**)value) = (char *)PLUGIN_NAME;
            return NPERR_NO_ERROR;
        case NPPVpluginDescriptionString:
            *((char**)value) = (char *)PLUGIN_DESCRIPTION;
            return NPERR_NO_ERROR;
        case NPPVpluginScriptableNPObject: {
            NPObject *plugin = (NPObject *)instanceData->pdata;
            browserFunctions->retainobject(plugin);
            *((void **)value) = plugin;
            return NPERR_NO_ERROR;
        }
        default:
            return NPERR_INVALID_PARAM;
    }
}

NPError NPP_SetValue(NPP instanceData, NPNVariable variable, void *value) {
    _log("instanceData=%p, variable=%i, value=%p", instanceData, variable, value);
    return NPERR_GENERIC_ERROR;
}
