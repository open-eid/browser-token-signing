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

#ifndef ESTEID_CERT_CLASS_H
#define	ESTEID_CERT_CLASS_H

#include "plugin.h"
#include "esteid_map.h"

typedef struct {
    NPObject header;
    NPP npp;
    char* certificate;
	char* issuerCN;
	char* validFrom;
	char* validTo;
	char* certificateAsHex;
	char* CN; 
} CertInstance;

NPObject *certAllocate(NPP npp, NPClass *theClass);

void certDeallocate(CertInstance *obj);

void certInvalidate();

bool certInvokeDefault(CertInstance *obj, NPVariant *args, uint32_t argCount, NPVariant *result);

bool certSetProperty(CertInstance *obj, NPIdentifier name, const NPVariant *variant);

bool certHasProperty(NPClass *theClass, NPIdentifier name);

bool certGetProperty(CertInstance *obj, NPIdentifier name, NPVariant *variant);

bool certHasMethod(NPObject *npobj, NPIdentifier name);

bool certInvoke(CertInstance *obj, NPIdentifier name, NPVariant *args, uint32_t argCount, NPVariant *result);

NPClass *certClass();

#endif

