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

#ifndef ESTEID_PLUGIN_H
#define	ESTEID_PLUGIN_H

#ifdef __APPLE__
#define XP_MACOSX
#define XP_UNIX
#endif

#include "npapi.h"
#include "npfunctions.h"

#include <string>
#include <vector>

typedef struct : public NPObject {
    NPP npp;
    NPNetscapeFuncs *browserFunctions = nullptr;
    std::string error;
    int errorCode = 0;
    bool allowedSite = false;
    std::vector<unsigned char> certInfo;
} PluginInstance;

NPClass *pluginClass();

typedef struct : public NPObject {
    PluginInstance *parent = nullptr;
} CertInstance;
    
NPClass *certClass();

std::vector<unsigned char> md5(const std::vector<unsigned char> &data);
bool isSameIdentifier(NPIdentifier identifier, const char* name);
bool copyStringToNPVariant(const char *string, NPVariant *variant);
std::string createStringFromNPVariant(const NPVariant &variant);

#endif
