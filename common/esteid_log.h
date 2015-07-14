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

#ifndef ESTEID_LOG_H
#define ESTEID_LOG_H

#include "esteid_map.h"

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define EstEID_log(...) EstEID_log_real(__func__, __FILE__, __LINE__, __VA_ARGS__)
#define EstEID_logMap(map) EstEID_logMap_real(__func__, __FILE__, __LINE__, map)
#elif (defined(__GNUC__) && __GNUC__ >= 3) || defined(_WIN32)
#define EstEID_log(...) EstEID_log_real(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
//#ifndef WIN_IE
#define EstEID_logMap(map) EstEID_logMap_real(__FUNCTION__, __FILE__, __LINE__, map)
//#endif
#else
#define EstEID_log(...) EstEID_log_real(NULL, NULL, -1, ...)
#define EstEID_logMap(map) EstEID_logMap_real(NULL, NULL, -1, map)
#endif

void EstEID_log_real(const char *func, const char *file, int line, const char *message, ...);

//#ifndef WIN_IE
void EstEID_logMap_real(const char *func, const char *file, int line, EstEID_Map map);
//#endif

#define LOG_LOCATION	EstEID_log("");

#endif
