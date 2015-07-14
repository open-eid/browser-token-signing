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

#ifndef ESTEID_ERROR_H
#define	ESTEID_ERROR_H

#define ESTEID_ERROR_SIZE 1024

#define ESTEID_NO_ERROR 0
#define ESTEID_USER_CANCEL 1
#define ESTEID_CERT_NOT_FOUND_ERROR 2
#define ESTEID_MD5_ERROR 3
#define ESTEID_CRYPTO_API_ERROR 4
#define ESTEID_UNKNOWN_ERROR 5
#define ESTEID_PKCS11_ERROR 14
#define ESTEID_LIBRARY_LOAD_ERROR 15
#define ESTEID_INVALID_HASH_ERROR 17
#define ESTEID_PTHREAD_ERROR 18
#define ESTEID_SITE_NOT_ALLOWED 19
#define ESTEID_PIN_BLOCKED 24

#define NOT_FOUND -1

char *getErrorMessage(unsigned int code);

#endif

