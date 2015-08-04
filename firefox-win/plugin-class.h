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

#ifndef ESTEID_PLUGIN_CLASS_H
#define	ESTEID_PLUGIN_CLASS_H

#define FAIL_IF_NOT_ALLOWED_SITE if(!isAllowedSite()) return false;

#define BINARY_SHA1_LENGTH 20
#define BINARY_SHA224_LENGTH 28
#define BINARY_SHA256_LENGTH 32
#define BINARY_SHA512_LENGTH 64

#include "plugin.h"

typedef struct {
    NPObject header;
    NPP npp;
    void *nativeWindowHandle;
	PCCERT_CONTEXT certContext;
} PluginInstance;

void pluginInvalidate();

bool pluginHasProperty(NPClass *theClass, NPIdentifier name);

bool pluginGetProperty(PluginInstance *obj, NPIdentifier name, NPVariant *variant);

bool pluginSetProperty(PluginInstance *obj, NPIdentifier name, const NPVariant *variant);

bool pluginHasMethod(PluginInstance *obj, NPIdentifier name);

bool pluginInvoke(PluginInstance *obj, NPIdentifier name, NPVariant *args, unsigned argCount, NPVariant *result);

bool pluginInvokeDefault(PluginInstance *obj, NPVariant *args, unsigned argCount, NPVariant *result);

NPObject *pluginAllocate(NPP npp, NPClass *theClass);

void pluginDeallocate(PluginInstance *obj);

NPClass *pluginClass();

bool isAllowedSite();

BOOL Unicode16ToAnsi(WCHAR *in_Src, CHAR *out_Dst, INT in_MaxLen);

char* byteToChar(BYTE* signature, int length );

void reverseBytes(BYTE* signature, int signatureLength);

bool selectCertificate(PluginInstance *obj, PCCERT_CONTEXT *certContext);

void handleError(char* methodName, PluginInstance *obj);

void handleErrorWithCode(int errorCode, char* methodName, PluginInstance *obj);

bool certificateMatchesId(PCCERT_CONTEXT certContext, char* id);

#endif	

