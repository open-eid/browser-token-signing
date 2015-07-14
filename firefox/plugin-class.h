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

#include "plugin.h"

typedef struct {
    NPObject header;
    NPP npp;
    void *nativeWindowHandle;
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

#endif	

