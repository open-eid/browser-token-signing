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
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "version.h"
#include "plugin.h"
#include "plugin-class.h"
#include "esteid_mime_types.h"
#include "esteid_misc.h"

//#define PLUGIN_NAME        "EstEID Firefox plug-in"
//#define PLUGIN_DESCRIPTION "Allows digital signing with Estonian ID cards"

NPNetscapeFuncs* browserFunctions;

extern char EstEID_error[1024];
extern int EstEID_errorCode;
extern bool allowedSite;

bool isSameIdentifier(NPIdentifier identifier, const char* name) {
	return browserFunctions->getstringidentifier(name) == identifier;
}

bool copyStringToNPVariant(const char *string, NPVariant *variant) {
	char *out = (char *)browserFunctions->memalloc(strlen(string) + 1);
	strcpy(out, string);
	//EstEID_log("copyStringToNPVariant copies: %s", string);
	STRINGZ_TO_NPVARIANT(out, *variant);
	//EstEID_log("copyStringToNPVariant variant: %s", createStringFromNPVariant(variant));
	return true;
}

NPUTF8* createStringFromNPVariant(const NPVariant *variant) {
	if (!NPVARIANT_IS_STRING(*variant)) {
		return strdup("");
	}
	size_t length = NPVARIANT_TO_STRING(*variant).UTF8Length;
	NPUTF8 *result = (NPUTF8 *)malloc(length + 1);
	memcpy(result, NPVARIANT_TO_STRING(*variant).UTF8Characters, length);
	result[length] = '\0';
	return result;
}

NP_EXPORT(NPError) NP_GetEntryPoints(NPPluginFuncs* pluginFuncs) {
	EstEID_log("");
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

#if defined(XP_MACOSX) || defined(_WIN32)

NP_EXPORT(NPError) NP_Initialize(NPNetscapeFuncs* browserFuncs) {
	browserFunctions = browserFuncs;
	return NPERR_NO_ERROR;
}
#else

NP_EXPORT(NPError) NP_Initialize(NPNetscapeFuncs* browserFuncs, NPPluginFuncs* pluginFuncs) {
	EstEID_log("");
	browserFunctions = browserFuncs;
	return NP_GetEntryPoints(pluginFuncs);
}
#endif

NP_EXPORT(NPError) NP_Shutdown(void) {
	EstEID_log("");
	return NPERR_NO_ERROR;
}

HINSTANCE pluginInstance;
BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
	EstEID_log("reason: %lu", reason);
	pluginInstance = instance;
	if (reason == DLL_PROCESS_ATTACH || reason == DLL_THREAD_ATTACH) {
		DisableThreadLibraryCalls(instance);
	}
    return TRUE;
}

NP_EXPORT(char*) NP_GetPluginVersion() {
	EstEID_log("NP_GetPluginVersion: %s", ESTEID_PLUGIN_VERSION);
	return(char *)ESTEID_PLUGIN_VERSION;
}

NP_EXPORT(NPError) NP_GetValue(void* future, NPPVariable aVariable, void* aValue) {
	switch (aVariable) {
		case NPPVpluginNameString:
			*((char**)aValue) = (char *)PLUGIN_NAME;
			break;
		case NPPVpluginDescriptionString:
			*((char**)aValue) = (char *)PLUGIN_DESCRIPTION;
			break;
		default:
			return NPERR_INVALID_PARAM;
			break;
	}
	return NPERR_NO_ERROR;
}

int is_allowed_protocol(char *protocol) {
	#ifdef DEVELOPMENT_MODE
	EstEID_log("protocol %s is allowed in development mode", protocol);
	return TRUE;
	#else
	int allowed = (!STRCASECMP("https", protocol));	
	EstEID_log("protocol %s is %sallowed", protocol, allowed ? "" : "not ");
	return allowed;
	#endif
}

#define MAX_PROTOCOL_NAME_LENGTH 5
int is_from_allowed_url(NPP instanceData) {

	NPObject *windowObject = NULL;

	browserFunctions->getvalue(instanceData, NPNVWindowNPObject, &windowObject);
	NPIdentifier identifier = browserFunctions->getstringidentifier("location");
	NPVariant variantValue;
	browserFunctions->getproperty(instanceData, windowObject, identifier, &variantValue);
	NPObject *locationObj = NPVARIANT_TO_OBJECT(variantValue);
	identifier =  browserFunctions->getstringidentifier("href");
	browserFunctions->getproperty(instanceData, locationObj, identifier, &variantValue);
	EstEID_log("href=%s", NPVARIANT_TO_STRING(variantValue).UTF8Characters);
	char protocol[MAX_PROTOCOL_NAME_LENGTH + 1];
	memset(protocol, '\0', MAX_PROTOCOL_NAME_LENGTH + 1);
	strncpy(protocol, (const char *)NPVARIANT_TO_STRING(variantValue).UTF8Characters, MAX_PROTOCOL_NAME_LENGTH);
	char *p = protocol;
	while(isalpha(*p)){
		p++;
	}
	*p = '\0';

	EstEID_log("detected protocol: %s", protocol);
	return is_allowed_protocol(protocol);
}


int loadErrorShown;

NPError NPP_New(NPMIMEType mimeType, NPP instanceData, uint16_t mode, int16_t argc, char* argn[], char* argv[], NPSavedData* saved) {
	EstEID_log("LOADED FF CNG PLUGIN VERSION %s", ESTEID_PLUGIN_VERSION);
	EstEID_log("instanceData=%p, mimeType=%s", instanceData, mimeType);

#ifdef DEVELOPMENT_MODE
	EstEID_log("*** NB! Plugin is built in development mode, all protocols are allowed! ***");
#endif
	
	browserFunctions->setvalue(instanceData, NPPVpluginWindowBool, (void*)false);

	PluginInstance *pluginInstance = (PluginInstance *)browserFunctions->createobject(instanceData, pluginClass());
	pluginInstance->npp = instanceData;
	pluginInstance->nativeWindowHandle = NULL;
	pluginInstance->certContext = NULL;


	if( is_from_allowed_url(instanceData) ) {
		allowedSite = TRUE;
	}
	else {
		allowedSite = FALSE;
		EstEID_log("URL did dot pass examination");
	}

	instanceData->pdata = pluginInstance;
	EstEID_log("no error on NPP_New");
	return NPERR_NO_ERROR;
}

NPError NPP_Destroy(NPP instanceData, NPSavedData** save) {
	EstEID_log("instanceData=%p", instanceData);
	browserFunctions->releaseobject((NPObject *)instanceData->pdata);
	return NPERR_NO_ERROR;
}

NPError NPP_GetValue(NPP instanceData, NPPVariable variable, void *value) {
	LOG_LOCATION;
	if (variable == NPPVpluginScriptableNPObject) {
		NPObject *plugin = (NPObject *)instanceData->pdata;
		browserFunctions->retainobject(plugin);
		*((void **)value) = plugin;
		return NPERR_NO_ERROR;
	}
	return NPERR_GENERIC_ERROR;
}

NPError NPP_SetValue(NPP instanceData, NPNVariable variable, void *value) {
	EstEID_log("instanceData=%p, variable=%i, value=%p", instanceData, variable, value);
	return NPERR_GENERIC_ERROR;
}

NPError NPP_SetWindow(NPP instanceData, NPWindow* window) {
	if (window == NULL || instanceData == NULL) return NPERR_NO_ERROR;
	PluginInstance* currentInstance = (PluginInstance*)(instanceData->pdata);

	if (currentInstance) {
		EstEID_log("window=%p, window->window=%p, currentInstance=%p, nativeWindowHandle=%p", window, window->window, currentInstance, currentInstance->nativeWindowHandle);

		if (window->window && (currentInstance->nativeWindowHandle != window->window)) {
			currentInstance->nativeWindowHandle = window->window;
			EstEID_log("nativeWindowHandle=%p", currentInstance->nativeWindowHandle);
		}
	}
	return NPERR_NO_ERROR;
}

NPError NPP_NewStream(NPP instanceData, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype) {
	return NPERR_NO_ERROR;
}

NPError NPP_DestroyStream(NPP instanceData, NPStream* stream, NPReason reason) {
	return NPERR_NO_ERROR;
}

int32_t NPP_WriteReady(NPP instanceData, NPStream* stream) {
	return 0;
}

int32_t NPP_Write(NPP instanceData, NPStream* stream, int32_t offset, int32_t len, void* buffer) {
	return 0;
}

void NPP_StreamAsFile(NPP instanceData, NPStream* stream, const char* fname) {
}

void NPP_Print(NPP instanceData, NPPrint* platformPrint) {
}

int16_t NPP_HandleEvent(NPP instanceData, void* event) {
	return 0;
}

void NPP_URLNotify(NPP instanceData, const char* url, NPReason reason, void* notifyData) {
}

void drawPlugin(NPP instanceData) {
}

