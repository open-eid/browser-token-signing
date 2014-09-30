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

#ifndef __ESTEID_DIALOG_COMMON_H__
#define __ESTEID_DIALOG_COMMON_H__

char* createDialogTitle(const char* nameAndID);

#ifdef _WIN32
#include "windows.h"
#define PIN2_MAX_LEN 15
#define SIGNATURE_BUFFER_SIZE 1024
typedef struct {
	TCHAR pin2[PIN2_MAX_LEN + 2];
	const char *name;
	const char *message;
	char *certId;
	char *hash;
	char signature[SIGNATURE_BUFFER_SIZE]; //NB!!! is it correct size!!!
	unsigned int minPin2Length;
} DialogData;
#endif

#endif
