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

#ifndef ESTEID_VERSION_H
#define ESTEID_VERSION_H

#if defined(MAJOR_VER) && defined(MINOR_VER) && defined(RELEASE_VER)
# define VER_STR_HELPER(x)       #x
# define VER_STR(x)              VER_STR_HELPER(x)
# ifdef WIN64
#  define ESTEID_PLUGIN_VERSION VER_STR(MAJOR_VER.MINOR_VER.RELEASE_VER) " 64bit"
# else
#  define ESTEID_PLUGIN_VERSION VER_STR(MAJOR_VER.MINOR_VER.RELEASE_VER) " 32bit"
# endif
#else
#  define ESTEID_PLUGIN_VERSION VERSION
#endif

#define PLUGIN_NAME        "Firefox Token Signing"
#define PLUGIN_DESCRIPTION "Allows digital signing with Estonian, Finnish, Latvian and Lithuanian ID cards"
#define MIME_TYPE         "application/x-digidoc"

#endif
    
